//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <assert.h>
#include <float.h>
#include "cmodel_engine.h"
#include "dispcoll_common.h"
#include "cmodel_private.h"
#include "builddisp.h"
#include "collisionutils.h"
#include "vstdlib/random.h"
#include "tier0/fasttimer.h"
#include "vphysics_interface.h"
#include "vphysics/virtualmesh.h"
#include "host.h"
#include "gl_model_private.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//class CVirtualTerrain;

//csurface_t dispSurf = { "terrain", 0, 0 };

void CM_PreStab( TraceInfo_t *pTraceInfo, cleaf_t *pLeaf, Vector &vStabDir, int collisionMask, int &contents );
void CM_Stab( TraceInfo_t *pTraceInfo, Vector const &start, Vector const &vStabDir, int contents );
void CM_PostStab( TraceInfo_t *pTraceInfo );

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void SetDispTraceSurfaceProps( trace_t *pTrace, CDispCollTree *pDisp )
{
	// use the default surface properties
	pTrace->surface.name = "**displacement**";
	pTrace->surface.flags = 0;
	if ( pTrace->IsDispSurfaceProp2() )
	{
		pTrace->surface.surfaceProps = pDisp->GetSurfaceProps( 1 );
	}
	else
	{
		pTrace->surface.surfaceProps = pDisp->GetSurfaceProps( 0 );
	}
}

// Fill out the meshlist for this terrain patch
void CVirtualTerrain::GetVirtualMesh(void* userData, virtualmeshlist_t* pList)
{
	intp index = (intp)userData;
	Assert(index >= 0 && index < m_DispCollTreeCount);
	mod->GetDispCollTrees(index)->GetVirtualMeshList(pList);
	pList->pHull = NULL;
	if (m_pDispHullData)
	{
		if (m_dispHullOffset[index] >= 0)
		{
			pList->pHull = m_pDispHullData + m_dispHullOffset[index];
		}
	}
}
// returns the bounds for the terrain patch
void CVirtualTerrain::GetWorldspaceBounds(void* userData, Vector* pMins, Vector* pMaxs)
{
	intp index = (intp)userData;
	*pMins = mod->GetDispBounds(index)->mins;
	*pMaxs = mod->GetDispBounds(index)->maxs;
}
// Query against the AABB tree to find the list of triangles for this patch in a sphere
void CVirtualTerrain::GetTrianglesInSphere(void* userData, const Vector& center, float radius, virtualmeshtrianglelist_t* pList)
{
	intp index = (intp)userData;
	pList->triangleCount = mod->GetDispCollTrees(index)->AABBTree_GetTrisInSphere(center, radius, pList->triangleIndices, ARRAYSIZE(pList->triangleIndices));
}
void CVirtualTerrain::LevelInit(model_t* mod, dphysdisp_t* pLump, int lumpSize)
{
	if (!pLump)
	{
		m_pDispHullData = NULL;
		return;
	}
	this->mod = mod;
	int totalHullData = 0;
	m_dispHullOffset.SetCount(mod->GetDispCollTreesCount());
	Assert(pLump->numDisplacements == m_DispCollTreeCount);
	// count the size of the lump
	unsigned short* pDataSize = (unsigned short*)(pLump + 1);
	for (int i = 0; i < pLump->numDisplacements; i++)
	{
		if (pDataSize[i] == (unsigned short)-1)
		{
			m_dispHullOffset[i] = -1;
			continue;
		}
		m_dispHullOffset[i] = totalHullData;
		totalHullData += pDataSize[i];
	}
	m_pDispHullData = new byte[totalHullData];
	byte* pData = (byte*)(pDataSize + pLump->numDisplacements);
	memcpy(m_pDispHullData, pData, totalHullData);
#if _DEBUG
	int offset = pData - ((byte*)pLump);
	Assert(offset + totalHullData == lumpSize);
#endif
}
void CVirtualTerrain::LevelShutdown()
{
	m_dispHullOffset.Purge();
	delete[] m_pDispHullData;
	m_pDispHullData = NULL;
}

// Find or create virtual terrain collision model.  Note that these will be shared by client & server
CPhysCollide *CM_PhysCollideForDisp(model_t* mod, int index )
{
	if ( index < 0 || index >= mod->GetDispCollTreesCount())
		return NULL;

	return mod->GetTerrainList()[index];
}

int CM_SurfacepropsForDisp(model_t* mod, int index )
{
	return mod->GetDispCollTrees(index)->GetSurfaceProps(0);
}

void CM_CreateDispPhysCollide(model_t* mod, dphysdisp_t *pDispLump, int dispLumpSize )
{
	mod->GetVirtualTerrain().LevelInit(mod, pDispLump, dispLumpSize);
	mod->GetTerrainList().SetCount(mod->GetDispCollTreesCount());
	for ( intp i = 0; i < mod->GetDispCollTreesCount(); i++ )
	{
		// Don't create a physics collision model for displacements that have been tagged as such.
		CDispCollTree *pDispTree = mod->GetDispCollTrees(i);
		if ( pDispTree && pDispTree->CheckFlags( CCoreDispInfo::SURF_NOPHYSICS_COLL ) )
		{
			mod->GetTerrainList()[i] = NULL;
			continue;
		}
		virtualmeshparams_t params;
		params.pMeshEventHandler = &mod->GetVirtualTerrain();
		params.userData = (void *)i;
		params.buildOuterHull = dispLumpSize > 0 ? false : true;

		mod->GetTerrainList()[i] = physcollision->CreateVirtualMesh( params );
	}
}

// End of level, free the collision models
void CM_DestroyDispPhysCollide(model_t* mod)
{
	mod->GetVirtualTerrain().LevelShutdown();
	for ( int i = mod->GetTerrainList().Count() - 1; i >= 0; --i)
	{
		physcollision->DestroyCollide( mod->GetTerrainList()[i] );
	}
	mod->GetTerrainList().Purge();
}

//-----------------------------------------------------------------------------
// New Collision!
//-----------------------------------------------------------------------------
void CM_TestInDispTree( TraceInfo_t *pTraceInfo, cleaf_t *pLeaf, const Vector &traceStart,
					   const Vector &boxMin, const Vector &boxMax, int collisionMask, trace_t *pTrace )
{
	bool bIsBox = ( ( boxMin.x != 0.0f ) || ( boxMin.y != 0.0f ) || ( boxMin.z != 0.0f ) ||
		( boxMax.x != 0.0f ) || ( boxMax.y != 0.0f ) || ( boxMax.z != 0.0f ) );

	// box test
	if( bIsBox )
	{
		// Box/Tree intersection test.
		Vector absMins = traceStart + boxMin;
		Vector absMaxs = traceStart + boxMax;

		// Test box against all displacements in the leaf.
		TraceCounter_t *pCounters = pTraceInfo->GetDispCounters();
		int count = pTraceInfo->GetCount();
		for( int i = 0; i < pLeaf->dispCount; i++ )
		{
			int dispIndex = pTraceInfo->m_mod->GetDispList(pLeaf->dispListStart + i);
			alignedbbox_t * RESTRICT pDispBounds = pTraceInfo->m_mod->GetDispBounds(dispIndex);

			// Respect trace contents
			if( !(pDispBounds->GetContents() & collisionMask) )
				continue;

			if ( !pTraceInfo->Visit( pDispBounds->GetCounter(), count, pCounters ) )
				continue;

			if ( IsBoxIntersectingBox( absMins, absMaxs, pDispBounds->mins, pDispBounds->maxs ) )
			{
				CDispCollTree *pDispTree = pTraceInfo->m_mod->GetDispCollTrees(dispIndex);
				if( pDispTree->AABBTree_IntersectAABB( absMins, absMaxs ) )
				{
					pTrace->startsolid = true;
					pTrace->allsolid = true;
					pTrace->fraction = 0.0f;
					pTrace->fractionleftsolid = 0.0f;
					pTrace->contents = pDispTree->GetContents();
					return;
				}
			}
		}
	}

	//
	// need to stab if is was a point test or the box test yeilded no intersection
	//
	Vector stabDir;
	int    contents;
	CM_PreStab( pTraceInfo, pLeaf, stabDir, collisionMask, contents );
	CM_Stab( pTraceInfo, traceStart, stabDir, contents );
	CM_PostStab( pTraceInfo );
}


//-----------------------------------------------------------------------------
// New Collision!
//-----------------------------------------------------------------------------
void CM_PreStab( TraceInfo_t *pTraceInfo, cleaf_t *pLeaf, Vector &vStabDir, int collisionMask, int &contents )
{
	if( !pLeaf->dispCount )
		return;

	// if the point wasn't in the bounded area of any of the displacements -- stab in any
	// direction and set contents to "solid"
	int dispIndex = pTraceInfo->m_mod->GetDispList(pLeaf->dispListStart);
	CDispCollTree *pDispTree = pTraceInfo->m_mod->GetDispCollTrees(dispIndex);
	pDispTree->GetStabDirection( vStabDir );
	contents = CONTENTS_SOLID;

	//
	// if the point is inside a displacement's (in the leaf) bounded area
	// then get the direction to stab from it
	//
	for( int i = 0; i < pLeaf->dispCount; i++ )
	{
		dispIndex = pTraceInfo->m_mod->GetDispList(pLeaf->dispListStart + i);
		pDispTree = pTraceInfo->m_mod->GetDispCollTrees(dispIndex);

		// Respect trace contents
		if( !(pDispTree->GetContents() & collisionMask) )
			continue;

		if( pDispTree->PointInBounds( pTraceInfo->m_start, pTraceInfo->m_mins, pTraceInfo->m_maxs, pTraceInfo->m_ispoint ) )
		{
			pDispTree->GetStabDirection( vStabDir );
			contents = pDispTree->GetContents();
			return;
		}
	}
}

static Vector InvDelta(const Vector &delta)
{
	Vector vecInvDelta;
	for ( int iAxis = 0; iAxis < 3; ++iAxis )
	{
		if ( delta[iAxis] != 0.0f )
		{
			vecInvDelta[iAxis] = 1.0f / delta[iAxis];
		}
		else
		{
			vecInvDelta[iAxis] = FLT_MAX;
		}
	}
	return vecInvDelta;
}

//-----------------------------------------------------------------------------
// New Collision!
//-----------------------------------------------------------------------------
void CM_Stab( TraceInfo_t *pTraceInfo, const Vector &start, const Vector &vStabDir, int contents )
{
	//
	// initialize the displacement trace parameters
	//
	pTraceInfo->m_trace.fraction = 1.0f;
	pTraceInfo->m_trace.fractionleftsolid = 0.0f;
	pTraceInfo->m_trace.surface = pTraceInfo->m_mod->GetNullSurface();

	pTraceInfo->m_trace.startsolid = false;
	pTraceInfo->m_trace.allsolid = false;

	pTraceInfo->m_bDispHit = false;
	pTraceInfo->m_DispStabDir = vStabDir;

	Vector end = pTraceInfo->m_end;

	pTraceInfo->m_start = start;
	pTraceInfo->m_end = start + ( vStabDir * /* world extents * 2*/99999.9f );
	pTraceInfo->m_delta = pTraceInfo->m_end - pTraceInfo->m_start;
	pTraceInfo->m_invDelta = InvDelta(pTraceInfo->m_delta);

	// increment the checkcount -- so we can retest objects that may have been tested
	// previous to the stab
	PushTraceVisits( pTraceInfo );

	// increment the stab count -- statistics
#ifdef COUNT_COLLISIONS
	g_CollisionCounts.m_Stabs++;
#endif

	// stab
	CM_RecursiveHullCheck( pTraceInfo, 0 /*root*/, 0.0f, 1.0f );

	PopTraceVisits( pTraceInfo );

	pTraceInfo->m_end = end;
}

//-----------------------------------------------------------------------------
// New Collision!
//-----------------------------------------------------------------------------
void CM_PostStab( TraceInfo_t *pTraceInfo )
{
	//
	// only need to resolve things that impacted against a displacement surface,
	// this is partially resolved in the post trace phase -- so just use that
	// data to determine
	//
	if( pTraceInfo->m_bDispHit && pTraceInfo->m_trace.startsolid )
	{
		pTraceInfo->m_trace.allsolid = true;
		pTraceInfo->m_trace.fraction = 0.0f;
		pTraceInfo->m_trace.fractionleftsolid = 0.0f;
	}
	else
	{
		pTraceInfo->m_trace.startsolid = false;
		pTraceInfo->m_trace.allsolid = false;
		pTraceInfo->m_trace.contents = 0;
		pTraceInfo->m_trace.fraction = 1.0f;
		pTraceInfo->m_trace.fractionleftsolid = 0.0f;
	}
}

//-----------------------------------------------------------------------------
// New Collision!
//-----------------------------------------------------------------------------
void CM_PostTraceToDispTree( TraceInfo_t *pTraceInfo )
{
	// only resolve things that impacted against a displacement surface
	if( !pTraceInfo->m_bDispHit )
		return;

	//
	// determine whether or not we are in solid
	//	
	if( DotProduct( pTraceInfo->m_trace.plane.normal, pTraceInfo->m_delta ) > 0.0f )
	{
		pTraceInfo->m_trace.startsolid = true;
		pTraceInfo->m_trace.allsolid = true;
	}
}

//-----------------------------------------------------------------------------
// New Collision!
//-----------------------------------------------------------------------------
template <bool IS_POINT>
void FASTCALL CM_TraceToDispTree( TraceInfo_t *pTraceInfo, CDispCollTree *pDispTree, float startFrac, float endFrac )
{
	Ray_t ray;
	ray.m_Start = pTraceInfo->m_start;
	ray.m_Delta = pTraceInfo->m_delta;
	ray.m_IsSwept = true;

	trace_t *pTrace = &pTraceInfo->m_trace;

	// ray cast
	if( IS_POINT )
	{
		ray.m_Extents.Init();
		ray.m_IsRay = true;

		if( pDispTree->AABBTree_Ray( ray, pTraceInfo->m_invDelta, pTrace ) )
		{
			pTraceInfo->m_bDispHit = true;
			pTrace->contents = pDispTree->GetContents();
			SetDispTraceSurfaceProps( pTrace, pDispTree );
		}
	}
	// box sweep
	else
	{
		ray.m_Extents = pTraceInfo->m_extents;
		ray.m_IsRay = false;
		if( pDispTree->AABBTree_SweepAABB( ray, pTraceInfo->m_invDelta, pTrace ) )
		{
			pTraceInfo->m_bDispHit = true;
			pTrace->contents = pDispTree->GetContents();
			SetDispTraceSurfaceProps( pTrace, pDispTree );
		}
	}

	//	CM_TraceToDispTreeTest( pDispTree, traceStart, traceEnd, boxMin, boxMax, startFrac, endFrac, pTrace, bRayCast );
}

template void FASTCALL CM_TraceToDispTree<true>( TraceInfo_t *pTraceInfo, CDispCollTree *pDispTree, float startFrac, float endFrac );

template void FASTCALL CM_TraceToDispTree<false>( TraceInfo_t *pTraceInfo, CDispCollTree *pDispTree, float startFrac, float endFrac );
