//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#ifndef CMODEL_H
#define CMODEL_H
#ifdef _WIN32
#pragma once
#endif

#include "trace.h"
#include "tier0/dbg.h"
#include "basehandle.h"
#include "studio.h"
#include "iclientrenderable.h"
#include "datacache/imdlcache.h"
#include "model_types.h"
#include "tier1/utlstring.h"
#include "bspfile.h"
#include "bitmap/cubemap.h"
//#include "gametrace.h"
#include "bsptreedata.h"

struct edict_t;
class IVModel;
class CGameTrace;
typedef CGameTrace trace_t;

/*
==============================================================

COLLISION DETECTION

==============================================================
*/

#include "bspflags.h"
//#include "mathlib/vector.h"

// gi.BoxEdicts() can return a list of either solid or trigger entities
// FIXME: eliminate AREA_ distinction?
#define	AREA_SOLID		1
#define	AREA_TRIGGERS	2

#include "vcollide.h"

struct cmodel_t
{
	Vector		mins, maxs;
	Vector		origin;		// for sounds or lights
	int			headnode;

	vcollide_t	vcollisionData;
};

struct csurface_t
{
	const char	*name;
	short		surfaceProps;
	unsigned short	flags;		// BUGBUG: These are declared per surface, not per material, but this database is per-material now
};

//-----------------------------------------------------------------------------
// A ray...
//-----------------------------------------------------------------------------

struct Ray_t
{
	VectorAligned  m_Start;	// starting point, centered within the extents
	VectorAligned  m_Delta;	// direction + length of the ray
	VectorAligned  m_StartOffset;	// Add this to m_Start to get the actual ray start
	VectorAligned  m_Extents;	// Describes an axis aligned box extruded along a ray
	bool	m_IsRay;	// are the extents zero?
	bool	m_IsSwept;	// is delta != 0?

	void Init( Vector const& start, Vector const& end )
	{
		VectorSubtract( end, start, m_Delta );

		m_IsSwept = (m_Delta.LengthSqr() != 0);

		VectorClear( m_Extents );
		m_IsRay = true;

		// Offset m_Start to be in the center of the box...
		VectorClear( m_StartOffset );
		VectorCopy( start, m_Start );
	}

	void Init( Vector const& start, Vector const& end, Vector const& mins, Vector const& maxs )
	{
		VectorSubtract( end, start, m_Delta );

		m_IsSwept = (m_Delta.LengthSqr() != 0);

		VectorSubtract( maxs, mins, m_Extents );
		m_Extents *= 0.5f;
		m_IsRay = (m_Extents.LengthSqr() < 1e-6);

		// Offset m_Start to be in the center of the box...
		VectorAdd( mins, maxs, m_StartOffset );
		m_StartOffset *= 0.5f;
		VectorAdd( start, m_StartOffset, m_Start );
		m_StartOffset *= -1.0f;
	}

	// compute inverse delta
	Vector InvDelta() const
	{
		Vector vecInvDelta;
		for ( int iAxis = 0; iAxis < 3; ++iAxis )
		{
			if ( m_Delta[iAxis] != 0.0f )
			{
				vecInvDelta[iAxis] = 1.0f / m_Delta[iAxis];
			}
			else
			{
				vecInvDelta[iAxis] = FLT_MAX;
			}
		}
		return vecInvDelta;
	}

private:
};


class IVModel
{
public:
	
	virtual int ModelFrameCount() const = 0;
	virtual bool IsTranslucent() const = 0;
	virtual int GetModelType() const = 0;
	virtual void GetModelRenderBounds(Vector& mins, Vector& maxs) const = 0;
	virtual studiohdr_t* GetStudiomodel() const = 0;
	virtual bool IsModelVertexLit() const = 0;
	virtual bool ModelHasMaterialProxy() const = 0;
	virtual void Mod_RecomputeTranslucency(int nSkin, int nBody, void /*IClientRenderable*/* pClientRenderable, float fInstanceAlphaModulate = 1.0f) = 0;
	virtual void* GetModelExtraData() const = 0;
	virtual const char* GetModelName() const = 0;
	virtual void GetIlluminationPoint(IClientRenderable* pRenderable, Vector const& origin,
		QAngle const& angles, Vector* pLightingCenter) const = 0;
	virtual bool IsTranslucentTwoPass() const = 0;
	virtual MDLHandle_t	GetCacheHandle() const = 0;
	virtual void GetModelMaterialColorAndLighting(IVModel* pWorld, const Vector& origin,
		const QAngle& angles, trace_t* pTrace, Vector& lighting, Vector& matColor) const = 0;
	virtual void GetModelBounds(Vector& mins, Vector& maxs) const = 0;
	virtual int GetModelSpriteWidth() const = 0;
	virtual int GetModelSpriteHeight() const = 0;
	virtual const char* GetModelKeyValueText() const = 0;
	virtual bool IsUsingFBTexture(int nSkin, int nBody, void /*IClientRenderable*/* pClientRenderable) const = 0;
	virtual int GetModelContents() const = 0;
	virtual vcollide_t* GetVCollide() const = 0;
	virtual int R_GetBrushModelPlaneCount() const = 0;
	virtual const cplane_t& R_GetBrushModelPlane(int nIndex, Vector* pOrigin) const = 0;
	virtual bool GetModelKeyValue(CUtlBuffer& buf) const = 0;
	virtual CPhysCollide* GetCollideForVirtualTerrain(int index) const = 0;

	// Returns the number of leaves
	virtual int LeafCount() const = 0;
	// Enumerates the leaves along a ray, box, etc.
	virtual bool EnumerateLeavesAtPoint(const Vector& pt, ISpatialLeafEnumerator* pEnum, intp context) = 0;
	virtual bool EnumerateLeavesInBox(const Vector& mins, const Vector& maxs, ISpatialLeafEnumerator* pEnum, intp context) = 0;
	virtual bool EnumerateLeavesInSphere(const Vector& center, float radius, ISpatialLeafEnumerator* pEnum, intp context) = 0;
	virtual bool EnumerateLeavesAlongRay(Ray_t const& ray, ISpatialLeafEnumerator* pEnum, intp context) = 0;

	virtual int Mod_GetMaterialCount() = 0;
	virtual int Mod_GetModelMaterials(int count, IMaterial** ppMaterial) = 0;
	virtual float				GetRadius() const = 0;
};

#endif // CMODEL_H

	
#include "gametrace.h"

