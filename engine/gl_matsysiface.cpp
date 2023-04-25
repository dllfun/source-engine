//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//===========================================================================//

// wrapper for the material system for the engine.

#include "render_pch.h"
#include "view.h"
#include "zone.h"
#include <float.h>
#include "sys_dll.h"
#include "materialsystem/imesh.h"
#include "gl_water.h"
#include "utlrbtree.h"
#include "istudiorender.h"
#include "tier0/dbg.h"
#include "KeyValues.h"
#include "vstdlib/random.h"
#include "lightcache.h"
#include "sysexternal.h"
#include "cmd.h"
#include "modelloader.h"
#include "tier0/icommandline.h"
#include "materialsystem/imaterial.h"
#include "toolframework/itoolframework.h"
#include "toolframework/itoolsystem.h"
#include "tier2/p4helpers.h"
#include "p4lib/ip4.h"
#include "vgui/ISystem.h"
#include <vgui_controls/Controls.h>


extern ConVar developer;
#ifdef _WIN32
#include <crtdbg.h>
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifndef SWDS
extern IMaterialSystem *materials;

IMaterial*	g_materialWireframe;
IMaterial*	g_materialTranslucentSingleColor;
IMaterial*	g_materialTranslucentVertexColor;
IMaterial*	g_materialWorldWireframe;
IMaterial*	g_materialWorldWireframeZBuffer;
IMaterial*	g_materialBrushWireframe;
IMaterial*	g_materialDecalWireframe;
IMaterial*	g_materialDebugLightmap;
IMaterial*	g_materialDebugLightmapZBuffer;
IMaterial*	g_materialDebugLuxels;
IMaterial*	g_materialLeafVisWireframe;
IMaterial*	g_pMaterialWireframeVertexColor;
IMaterial*  g_pMaterialWireframeVertexColorIgnoreZ;
IMaterial*  g_pMaterialLightSprite;
IMaterial*  g_pMaterialShadowBuild;
IMaterial*  g_pMaterialMRMWireframe;
IMaterial*  g_pMaterialWriteZ;
IMaterial*  g_pMaterialWaterDuDv;
IMaterial*  g_pMaterialWaterFirstPass;
IMaterial*  g_pMaterialWaterSecondPass;
IMaterial*	g_pMaterialAmbientCube;
IMaterial*	g_pMaterialDebugFlat;
IMaterial*	g_pMaterialDepthWrite[2][2];
IMaterial*	g_pMaterialSSAODepthWrite[2][2];

#ifdef NEWMESH
CUtlVector<IVertexBuffer *> g_WorldStaticMeshes;  // fixme - rename to g_WorldStaticVertexBuffers
#else
CUtlVector<IMesh *> g_WorldStaticMeshes;
#endif

void WorldStaticMeshCreate( model_t* pWorld );
void WorldStaticMeshDestroy( void );


bool TangentSpaceSurfaceSetup(model_t* pWorld, SurfaceHandle_t surfID, Vector &tVect );
void TangentSpaceComputeBasis( Vector& tangent, Vector& binormal, const Vector& normal, const Vector& tVect, bool negateTangent );


//-----------------------------------------------------------------------------
// A console command edit a particular material
//-----------------------------------------------------------------------------
CON_COMMAND_F( mat_edit, "Bring up the material under the crosshair in the editor", FCVAR_CHEAT )
{
	if ( !toolframework->InToolMode() )
		return;

	IMaterial* pMaterial = NULL;
	if ( args.ArgC() < 2 )
	{
		pMaterial = GetMaterialAtCrossHair(g_pHost->Host_GetWorldModel());
	}
	else
	{
		const char *pMaterialName = args[ 1 ];
		pMaterial = materials->FindMaterial( pMaterialName, "edited materials", false );
	}

	if ( !pMaterial )
	{
		ConMsg( "no/bad material\n" );
	}
	else
	{
		IToolSystem *pToolSystem = toolframework->SwitchToTool( "Material Editor" );
		if ( pToolSystem )
		{
			ConMsg( "editing material \"%s\"\n", pMaterial->GetName() );

			KeyValues *pKeyValues = new KeyValues( "EditMaterial" );
			pKeyValues->SetString( "material", pMaterial->GetName() );
			pToolSystem->PostMessage( 0, pKeyValues );
			pKeyValues->deleteThis();
		}
	}
}


//-----------------------------------------------------------------------------
// A console command to spew out the material under the crosshair
//-----------------------------------------------------------------------------
CON_COMMAND_F( mat_crosshair, "Display the name of the material under the crosshair", FCVAR_CHEAT )
{
	IMaterial* pMaterial = GetMaterialAtCrossHair(g_pHost->Host_GetWorldModel());
	if (!pMaterial)
		ConMsg ("no/bad material\n");
	else
		ConMsg ("hit material \"%s\"\n", pMaterial->GetName());
}

//-----------------------------------------------------------------------------
// A console command to open the material under the crosshair in the associated editor.
//-----------------------------------------------------------------------------
CON_COMMAND_F( mat_crosshair_edit, "open the material under the crosshair in the editor defined by mat_crosshair_edit_editor", FCVAR_CHEAT )
{
	IMaterial* pMaterial = GetMaterialAtCrossHair(g_pHost->Host_GetWorldModel());
	if (!pMaterial)
	{
		ConMsg ("no/bad material\n");
	}
	else
	{
		char chResolveName[ 256 ] = {0}, chResolveNameArg[ 256 ] = {0};
		Q_snprintf( chResolveNameArg, sizeof( chResolveNameArg ) - 1, "materials/%s.vmt", pMaterial->GetName() );
		char const *szResolvedName = g_pFileSystem->RelativePathToFullPath( chResolveNameArg, "game", chResolveName, sizeof( chResolveName ) - 1 );
		if ( p4 )
		{
			CP4AutoEditAddFile autop4( szResolvedName );
		}
		else
		{
			Warning( "run with -p4 to get p4 operations upon mat_crosshair_edit\n" );
		}
		vgui::system()->ShellExecute( "open", szResolvedName );
	}
}

//-----------------------------------------------------------------------------
// A console command to open the material under the crosshair in the associated editor.
//-----------------------------------------------------------------------------
CON_COMMAND_F( mat_crosshair_explorer, "open the material under the crosshair in explorer and highlight the vmt file", FCVAR_CHEAT )
{
	IMaterial* pMaterial = GetMaterialAtCrossHair(g_pHost->Host_GetWorldModel());
	if (!pMaterial)
	{
		ConMsg ("no/bad material\n");
	}
	else
	{
		char chResolveName[ 256 ] = {0}, chResolveNameArg[ 256 ] = {0};
		Q_snprintf( chResolveNameArg, sizeof( chResolveNameArg ) - 1, "materials/%s.vmt", pMaterial->GetName() );
		char const *szResolvedName = g_pFileSystem->RelativePathToFullPath( chResolveNameArg, "game", chResolveName, sizeof( chResolveName ) - 1 );
		char params[256];
		Q_snprintf( params, sizeof( params ) - 1, "/E,/SELECT,%s", szResolvedName );
		vgui::system()->ShellExecuteEx( "open", "explorer.exe", params );
	}
}

//-----------------------------------------------------------------------------
// A console command to open the material under the crosshair in the associated editor.
//-----------------------------------------------------------------------------
CON_COMMAND_F( mat_crosshair_reloadmaterial, "reload the material under the crosshair", FCVAR_CHEAT )
{
	IMaterial* pMaterial = GetMaterialAtCrossHair(g_pHost->Host_GetWorldModel());
	if (!pMaterial)
	{
		ConMsg ("no/bad material\n");
	}
	else
	{
		materials->ReloadMaterials( pMaterial->GetName() );
	}
}

CON_COMMAND_F( mat_crosshair_printmaterial, "print the material under the crosshair", FCVAR_CHEAT )
{
	IMaterial* pMaterial = GetMaterialAtCrossHair(g_pHost->Host_GetWorldModel());
	if (!pMaterial)
	{
		ConMsg ("no/bad material\n");
	}
	else
	{
		materials->DebugPrintUsedMaterials( pMaterial->GetName(), true );
	}
}

static void RegisterLightmappedSurface(model_t* pWorld, SurfaceHandle_t surfID )
{
	int lightmapSize[2];
	int allocationWidth, allocationHeight;
	bool bNeedsBumpmap;
	
	// fixme: lightmapSize needs to be in msurface_t once we
	// switch over to having lightmap size untied to base texture
	// size
	lightmapSize[0] = (pWorld->MSurf_LightmapExtents( surfID )[0] ) + 1;
	lightmapSize[1] = (pWorld->MSurf_LightmapExtents( surfID )[1] ) + 1;
	
	// Allocate all bumped lightmaps next to each other so that we can just 
	// increment the s texcoord by pSurf->bumpSTexCoordOffset to render the next
	// of the three lightmaps
	bNeedsBumpmap = SurfNeedsBumpedLightmaps(pWorld, surfID );
	if( bNeedsBumpmap )
	{
		pWorld->MSurf_Flags( surfID ) |= SURFDRAW_BUMPLIGHT;
		allocationWidth = lightmapSize[0] * ( NUM_BUMP_VECTS+1 );
	}
	else
	{
		pWorld->MSurf_Flags( surfID ) &= ~SURFDRAW_BUMPLIGHT;
		allocationWidth = lightmapSize[0];
	}
	allocationHeight = lightmapSize[1];

	// register this surface's lightmap
	int offsetIntoLightmapPage[2];
	pWorld->MSurf_MaterialSortID( surfID ) = materials->AllocateLightmap(
		allocationWidth, 
		allocationHeight,
		offsetIntoLightmapPage,
		pWorld->MSurf_TexInfo( surfID )->material );

	pWorld->MSurf_OffsetIntoLightmapPage( surfID )[0] = offsetIntoLightmapPage[0];
	pWorld->MSurf_OffsetIntoLightmapPage( surfID )[1] = offsetIntoLightmapPage[1];
}

static void RegisterUnlightmappedSurface(model_t* pWorld, SurfaceHandle_t surfID )
{
	pWorld->MSurf_MaterialSortID( surfID ) = materials->AllocateWhiteLightmap(pWorld->MSurf_TexInfo( surfID )->material );
	pWorld->MSurf_OffsetIntoLightmapPage( surfID )[0] = 0;
	pWorld->MSurf_OffsetIntoLightmapPage( surfID )[1] = 0;
}
struct SurfIdWrapper{
	model_t* pWorld;
	SurfaceHandle_t surf;
};
static bool LightmapLess(const SurfIdWrapper& surfID1, const SurfIdWrapper& surfID2 )
{
	// FIXME: This really should be in the material system,
	// as it completely depends on the behavior of the lightmap packer
	bool hasLightmap1 = (surfID1.pWorld->MSurf_Flags( surfID1.surf ) & SURFDRAW_NOLIGHT) == 0;
	bool hasLightmap2 = (surfID1.pWorld->MSurf_Flags( surfID2.surf) & SURFDRAW_NOLIGHT) == 0;

	// We want lightmapped surfaces to show up first
	if (hasLightmap1 != hasLightmap2)
		return hasLightmap1 > hasLightmap2;
	
	// The sort by enumeration ID
	IMaterial* pMaterial1 = surfID1.pWorld->MSurf_TexInfo( surfID1.surf)->material;
	IMaterial* pMaterial2 = surfID1.pWorld->MSurf_TexInfo( surfID2.surf)->material;
	int enum1 = pMaterial1->GetEnumerationID();
	int enum2 = pMaterial2->GetEnumerationID();
	if (enum1 != enum2)
		return enum1 < enum2;

	bool hasLightstyle1 = (surfID1.pWorld->MSurf_Flags( surfID1.surf) & SURFDRAW_HASLIGHTSYTLES) == 0;
	bool hasLightstyle2 = (surfID1.pWorld->MSurf_Flags( surfID2.surf) & SURFDRAW_HASLIGHTSYTLES) == 0;

	// We want Lightstyled surfaces to show up first
	if (hasLightstyle1 != hasLightstyle2)
		return hasLightstyle1 > hasLightstyle2;

	// Then sort by lightmap area for better packing... (big areas first)
	// NOTE: Don't care about bumpmap increasing area here because it is a linear factor 
	// (all surfs with the same material have the same bumpmapping cost)
#if 1
	int area1 = surfID1.pWorld->MSurf_LightmapExtents( surfID1.surf)[0] * surfID1.pWorld->MSurf_LightmapExtents( surfID1.surf)[1];
	int area2 = surfID1.pWorld->MSurf_LightmapExtents( surfID2.surf)[0] * surfID1.pWorld->MSurf_LightmapExtents( surfID2.surf)[1];
	return area2 < area1;
#else
	// Previous algorithm: pack minimum height first
	// NOTE: In d1_trainstation_05, greatest area results in fewer material splits
	//		so I've switched over to that heuristic
	return MSurf_LightmapExtents( surfID1 )[1] < MSurf_LightmapExtents( surfID2 )[1];
#endif
}

void MaterialSystem_RegisterLightmapSurfaces(model_t* pWorld)
{
	SurfaceHandle_t surfID = SURFACE_HANDLE_INVALID;

	materials->BeginLightmapAllocation();

	// Add all the surfaces to a list, sorted by lightmapped
	// then by material enumeration then by area
	CUtlRBTree< SurfIdWrapper, int >	surfaces( 0, pWorld->GetSurfacesCount(), LightmapLess);
	for( int surfaceIndex = 0; surfaceIndex < pWorld->GetSurfacesCount(); surfaceIndex++ )
	{
		surfID = pWorld->SurfaceHandleFromIndex( surfaceIndex );
		if( (pWorld->MSurf_TexInfo( surfID )->flags & SURF_NOLIGHT ) ||
			(pWorld->MSurf_Flags( surfID ) & SURFDRAW_NOLIGHT) )
		{
			pWorld->MSurf_Flags( surfID ) |= SURFDRAW_NOLIGHT;
		}
		else
		{
			pWorld->MSurf_Flags( surfID ) &= ~SURFDRAW_NOLIGHT;
		}
		SurfIdWrapper surfW;
		surfW.pWorld = pWorld;
		surfW.surf = surfID;
		surfaces.Insert(surfW);
	}

	// iterate sorted surfaces
	surfID = SURFACE_HANDLE_INVALID;
	for (int i = surfaces.FirstInorder(); i != surfaces.InvalidIndex(); i = surfaces.NextInorder(i) )
	{
		surfID = surfaces[i].surf;

		bool hasLightmap = (pWorld->MSurf_Flags( surfID ) & SURFDRAW_NOLIGHT) == 0;
		if ( hasLightmap )
		{
			RegisterLightmappedSurface(pWorld, surfID );
		}
		else
		{
			RegisterUnlightmappedSurface(pWorld, surfID );
		}
	}
	materials->EndLightmapAllocation();
}

static void TestBumpSanity(model_t* pWorld, SurfaceHandle_t surfID )
{
	ASSERT_SURF_VALID( surfID );
	// use the last one to check if we need a bumped lightmap, but don't have it so that we can warn.
	bool needsBumpmap = SurfNeedsBumpedLightmaps(pWorld, surfID );
	bool hasBumpmap = SurfHasBumpedLightmaps(pWorld, surfID );
	
	if ( needsBumpmap && !hasBumpmap && pWorld->MSurf_Samples( surfID ) )
	{
		Warning( "Need to rebuild map to get bumped lighting on material %s\n", 
			materialSortInfoArray[pWorld->MSurf_MaterialSortID( surfID )].material->GetName() );
	}
}

void MaterialSytsem_DoBumpWarnings(model_t* pWorld)
{
	int sortID;
	IMaterial *pPrevMaterial = NULL;

	for( sortID = 0; sortID < g_WorldStaticMeshes.Count(); sortID++ )
	{
		if( pPrevMaterial == materialSortInfoArray[sortID].material )
		{
			continue;
		}
		// Find one surface in each material sort info type
		for ( int surfaceIndex = 0; surfaceIndex < pWorld->GetSurfacesCount(); surfaceIndex++ )
		{
			SurfaceHandle_t surfID = pWorld->SurfaceHandleFromIndex( surfaceIndex );

			if(pWorld->MSurf_MaterialSortID( surfID ) == sortID )
			{
				TestBumpSanity(pWorld, surfID );
				break;
			}
		}
		pPrevMaterial = materialSortInfoArray[sortID].material;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
static void GenerateTexCoordsForPrimVerts(model_t* pWorld)
{
	int j, k, l;
	for ( int surfaceIndex = 0; surfaceIndex < pWorld->GetSurfacesCount(); surfaceIndex++ )
	{
		SurfaceHandle_t surfID = pWorld->SurfaceHandleFromIndex( surfaceIndex );
/*
		if( pSurf->numPrims > 0 )
		{
			ConMsg( "pSurf %d has %d prims (normal: %f %f %f dist: %f)\n", 
				( int )i, ( int )pSurf->numPrims, 
				pSurf->plane->normal[0], pSurf->plane->normal[1], pSurf->plane->normal[2], 
				pSurf->plane->dist );
			ConMsg( "\tfirst primID: %d\n", ( int )pSurf->firstPrimID );
		}
*/
		for( j = 0; j < pWorld->MSurf_NumPrims( surfID ); j++ )
		{
			mprimitive_t *pPrim;
			assert(pWorld->MSurf_FirstPrimID( surfID ) + j < pWorld->GetPrimitivesCount() );
			pPrim = pWorld->GetPrimitives(pWorld->MSurf_FirstPrimID( surfID ) + j);
			for( k = 0; k < pPrim->vertCount; k++ )
			{
				int lightmapSize[2];
				int lightmapPageSize[2];
				float sOffset, sScale, tOffset, tScale;
				
				materials->GetLightmapPageSize( 
					SortInfoToLightmapPage( pWorld->MSurf_MaterialSortID( surfID ) ), 
					&lightmapPageSize[0], &lightmapPageSize[1] );
				lightmapSize[0] = (pWorld->MSurf_LightmapExtents( surfID )[0] ) + 1;
				lightmapSize[1] = (pWorld->MSurf_LightmapExtents( surfID )[1] ) + 1;

				sScale = 1.0f / ( float )lightmapPageSize[0];
				sOffset = ( float )pWorld->MSurf_OffsetIntoLightmapPage( surfID )[0] * sScale;
				sScale = pWorld->MSurf_LightmapExtents( surfID )[0] * sScale;

				tScale = 1.0f / ( float )lightmapPageSize[1];
				tOffset = ( float )pWorld->MSurf_OffsetIntoLightmapPage( surfID )[1] * tScale;
				tScale = pWorld->MSurf_LightmapExtents( surfID )[1] * tScale;

				for ( l = 0; l < pPrim->vertCount; l++ )
				{
					// world-space vertex
					assert( l+pPrim->firstVert < pWorld->GetPrimvertsCount() );
					mprimvert_t &vert = pWorld->GetPrimverts(l+pPrim->firstVert);
					Vector& vec = vert.pos;

					// base texture coordinate
					vert.texCoord[0] = DotProduct (vec, pWorld->MSurf_TexInfo( surfID )->textureVecsTexelsPerWorldUnits[0].AsVector3D()) +
						pWorld->MSurf_TexInfo( surfID )->textureVecsTexelsPerWorldUnits[0][3];
					vert.texCoord[0] /= pWorld->MSurf_TexInfo( surfID )->material->GetMappingWidth();

					vert.texCoord[1] = DotProduct (vec, pWorld->MSurf_TexInfo( surfID )->textureVecsTexelsPerWorldUnits[1].AsVector3D()) +
						pWorld->MSurf_TexInfo( surfID )->textureVecsTexelsPerWorldUnits[1][3];
					vert.texCoord[1] /= pWorld->MSurf_TexInfo( surfID )->material->GetMappingHeight();

					if ( (pWorld->MSurf_Flags( surfID ) & SURFDRAW_NOLIGHT) )
					{
						vert.lightCoord[0] = 0.5f;
						vert.lightCoord[1] = 0.5f;
					}
					else if (pWorld->MSurf_LightmapExtents( surfID )[0] == 0 )
					{
						vert.lightCoord[0] = sOffset;
						vert.lightCoord[1] = tOffset;
					}
					else
					{
						vert.lightCoord[0] = DotProduct (vec, pWorld->MSurf_TexInfo( surfID )->lightmapVecsLuxelsPerWorldUnits[0].AsVector3D()) +
							pWorld->MSurf_TexInfo( surfID )->lightmapVecsLuxelsPerWorldUnits[0][3];
						vert.lightCoord[0] -= pWorld->MSurf_LightmapMins( surfID )[0];
						vert.lightCoord[0] += 0.5f;
						vert.lightCoord[0] /= ( float )pWorld->MSurf_LightmapExtents( surfID )[0]; //pSurf->texinfo->texture->width;

						vert.lightCoord[1] = DotProduct (vec, pWorld->MSurf_TexInfo( surfID )->lightmapVecsLuxelsPerWorldUnits[1].AsVector3D()) +
							pWorld->MSurf_TexInfo( surfID )->lightmapVecsLuxelsPerWorldUnits[1][3];
						vert.lightCoord[1] -= pWorld->MSurf_LightmapMins( surfID )[1];
						vert.lightCoord[1] += 0.5f;
						vert.lightCoord[1] /= ( float )pWorld->MSurf_LightmapExtents( surfID )[1]; //pSurf->texinfo->texture->height;
						
						vert.lightCoord[0] = sOffset + vert.lightCoord[0] * sScale;
						vert.lightCoord[1] = tOffset + vert.lightCoord[1] * tScale;
					}
				}
			}			
		}
	}
}

struct sortmap_t
{
	MaterialSystem_SortInfo_t info;
	int index;
};

IMatRenderContext *pSortMapRenderContext;

int __cdecl SortMapCompareFunc( const void *pElem0, const void *pElem1 )
{
	const sortmap_t *pMap0 = (const sortmap_t *)pElem0;
	const sortmap_t *pMap1 = (const sortmap_t *)pElem1;
	return pSortMapRenderContext->CompareMaterialCombos( pMap0->info.material, pMap1->info.material, pMap0->info.lightmapPageID, pMap1->info.lightmapPageID );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void MaterialSystem_CreateSortinfo(model_t* pWorld)
{
	Assert( !materialSortInfoArray );

	int nSortIDs = materials->GetNumSortIDs();
	materialSortInfoArray = ( MaterialSystem_SortInfo_t * )new MaterialSystem_SortInfo_t[ nSortIDs ];
	Assert( materialSortInfoArray );
	materials->GetSortInfo( materialSortInfoArray );

	int i = 0;
	sortmap_t *pMap = (sortmap_t *)_alloca( sizeof(sortmap_t) * nSortIDs );
	for ( i = 0; i < nSortIDs; i++ )
	{
		pMap[i].info = materialSortInfoArray[i];
		pMap[i].index = i;
	}

	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	pSortMapRenderContext = pRenderContext;

	qsort( pMap, nSortIDs, sizeof( sortmap_t ), SortMapCompareFunc );

	int *pSortIDRemap = (int *)_alloca( sizeof(int) * nSortIDs );
	for ( i = 0; i < nSortIDs; i++ )
	{
		materialSortInfoArray[i] = pMap[i].info;
		pSortIDRemap[pMap[i].index] = i;
		//Msg("Material %s, lightmap %d ", materialSortInfoArray[i].material->GetName(), materialSortInfoArray[i].lightmapPageID );
	}

	for ( int surfaceIndex = 0; surfaceIndex < pWorld->GetSurfacesCount(); surfaceIndex++ )
	{
		SurfaceHandle_t surfID = pWorld->SurfaceHandleFromIndex( surfaceIndex );
		int sortID = pWorld->MSurf_MaterialSortID( surfID );
#if _DEBUG
		IMaterial *pMaterial = MSurf_TexInfo( surfID )->material;

		if ( !HushAsserts() )
		{
			Assert ( materialSortInfoArray[pSortIDRemap[sortID]].material == pMaterial );
		}
#endif
		pWorld->MSurf_MaterialSortID( surfID ) = pSortIDRemap[sortID];
	}

	// Create texcoords for subdivided surfaces
	GenerateTexCoordsForPrimVerts(pWorld);
	// Create the hardware vertex buffers for each face
	WorldStaticMeshCreate(pWorld);
	if ( developer.GetInt() )
	{
		MaterialSytsem_DoBumpWarnings(pWorld);
	}
}

bool SurfHasBumpedLightmaps(model_t* pWorld, SurfaceHandle_t surfID )
{
	ASSERT_SURF_VALID( surfID );
	bool hasBumpmap = false;
	if( (pWorld->MSurf_TexInfo( surfID )->flags & SURF_BUMPLIGHT ) &&
		( !(pWorld->MSurf_TexInfo( surfID )->flags & SURF_NOLIGHT ) ) &&
		(pWorld->GetLightdata() ) &&
		(pWorld->MSurf_Samples( surfID ) ) )
	{
		hasBumpmap = true;
	}
	return hasBumpmap;
}

bool SurfNeedsBumpedLightmaps(model_t* pWorld, SurfaceHandle_t surfID )
{
	ASSERT_SURF_VALID( surfID );
	assert(pWorld->MSurf_TexInfo( surfID ) );
	assert(pWorld->MSurf_TexInfo( surfID )->material );
	return pWorld->MSurf_TexInfo( surfID )->material->GetPropertyFlag( MATERIAL_PROPERTY_NEEDS_BUMPED_LIGHTMAPS );
}

bool SurfHasLightmap(model_t* pWorld, SurfaceHandle_t surfID )
{
	ASSERT_SURF_VALID( surfID );

	bool hasLightmap = false;
	if( ( !(pWorld->MSurf_TexInfo( surfID )->flags & SURF_NOLIGHT ) ) &&
		(pWorld->GetLightdata() ) &&
		(pWorld->MSurf_Samples( surfID ) ) )
	{
		hasLightmap = true;
	}
	return hasLightmap;
}

bool SurfNeedsLightmap(model_t* pWorld, SurfaceHandle_t surfID )
{
	ASSERT_SURF_VALID( surfID );
	assert(pWorld->MSurf_TexInfo( surfID ) );
	assert(pWorld->MSurf_TexInfo( surfID )->material );
	if (pWorld->MSurf_TexInfo( surfID )->flags & SURF_NOLIGHT)
		return false;

	return pWorld->MSurf_TexInfo( surfID )->material->GetPropertyFlag( MATERIAL_PROPERTY_NEEDS_LIGHTMAP );
}



//-----------------------------------------------------------------------------
// Purpose: This builds the surface info for a terrain face
//-----------------------------------------------------------------------------

void BuildMSurfaceVerts( model_t* pWorld, SurfaceHandle_t surfID, Vector *verts,
							Vector2D *texCoords, Vector2D lightCoords[][4] )
{ 
	SurfaceCtx_t ctx;
	SurfSetupSurfaceContext(pWorld, ctx, surfID );

	int vertCount = pWorld->MSurf_VertCount( surfID );
	int vertFirstIndex = pWorld->MSurf_FirstVertIndex( surfID );
	for ( int i = 0; i < vertCount; i++ )
	{
		int vertIndex = *pWorld->GetVertindices(vertFirstIndex + i);

		// world-space vertex
		Vector& vec = pWorld->GetVertexes(vertIndex)->position;

		// output to mesh
		if ( verts )
		{
			VectorCopy( vec, verts[i] );
		}

		if ( texCoords )
		{
			SurfComputeTextureCoordinate(pWorld, ctx, surfID, vec, texCoords[i] );
		}

		//
		// garymct: normalized (within space of surface) lightmap texture coordinates
		//
		if ( lightCoords )
		{
			SurfComputeLightmapCoordinate(pWorld, ctx, surfID, vec, lightCoords[i][0] );

			if (pWorld->MSurf_Flags( surfID ) & SURFDRAW_BUMPLIGHT )
			{
				// bump maps appear left to right in lightmap page memory, calculate the offset for the
				// width of a single map
				for ( int bumpID = 1; bumpID <= NUM_BUMP_VECTS; bumpID++ )
				{
					lightCoords[i][bumpID][0] = lightCoords[i][0][0] + (bumpID * ctx.m_BumpSTexCoordOffset);
					lightCoords[i][bumpID][1] = lightCoords[i][0][1];
				}
			}
		}
	}
}



void BuildMSurfacePrimVerts(model_t* pWorld, mprimitive_t *prim, CMeshBuilder &builder, SurfaceHandle_t surfID )
{
	Vector tVect;
	bool negate = false;
// FIXME: For some reason, normals are screwed up on water surfaces.  Revisit this once we have normals started in primverts.
	if (pWorld->MSurf_Flags( surfID ) & SURFDRAW_TANGENTSPACE )
	{
		negate = TangentSpaceSurfaceSetup(pWorld, surfID, tVect );
	}
//	Vector& normal = pBrushData->vertnormals[ pBrushData->vertnormalindices[MSurf_FirstVertNormal( surfID )] ];

	for ( int i = 0; i < prim->vertCount; i++ )
	{
		mprimvert_t &primVert = pWorld->GetPrimverts(prim->firstVert + i);
		builder.Position3fv( primVert.pos.Base() );
		builder.Normal3fv(pWorld->MSurf_Plane( surfID ).normal.Base() );
//		builder.Normal3fv( normal.Base() );
		builder.TexCoord2fv( 0, primVert.texCoord );
		builder.TexCoord2fv( 1, primVert.lightCoord );
		if (pWorld->MSurf_Flags( surfID ) & SURFDRAW_TANGENTSPACE )
		{
			Vector tangentS, tangentT;
			TangentSpaceComputeBasis( tangentS, tangentT, pWorld->MSurf_Plane( surfID ).normal, tVect, false );
			builder.TangentS3fv( tangentS.Base() );
			builder.TangentT3fv( tangentT.Base() );
		}
		builder.AdvanceVertex();
	}
}

void BuildMSurfacePrimIndices(model_t* pWorld, mprimitive_t *prim, CMeshBuilder &builder )
{
	for ( int i = 0; i < prim->indexCount; i++ )
	{
		unsigned short primIndex = pWorld->GetPrimindices(prim->firstIndex + i);
		builder.Index( primIndex - prim->firstVert );
		builder.AdvanceIndex();
	}
}

//-----------------------------------------------------------------------------
// Here's a version of the mesh builder used to allow for client DLL to draw brush models
//-----------------------------------------------------------------------------

void BuildBrushModelVertexArray(model_t* pWorld, SurfaceHandle_t surfID, BrushVertex_t* pVerts )
{
	SurfaceCtx_t ctx;
	SurfSetupSurfaceContext(pWorld, ctx, surfID );

	Vector tVect;
	bool negate = false;
	if (pWorld->MSurf_Flags( surfID ) & SURFDRAW_TANGENTSPACE )
	{
		negate = TangentSpaceSurfaceSetup(pWorld, surfID, tVect );
	}

	for ( int i = 0; i < pWorld->MSurf_VertCount( surfID ); i++ )
	{
		int vertIndex = *pWorld->GetVertindices(pWorld->MSurf_FirstVertIndex( surfID ) + i);

		// world-space vertex
		Vector& vec = pWorld->GetVertexes(vertIndex)->position;

		// output to mesh
		VectorCopy( vec, pVerts[i].m_Pos );

		Vector2D uv;
		SurfComputeTextureCoordinate(pWorld, ctx, surfID, vec, pVerts[i].m_TexCoord );

		// garymct: normalized (within space of surface) lightmap texture coordinates
		SurfComputeLightmapCoordinate(pWorld, ctx, surfID, vec, pVerts[i].m_LightmapCoord );

// Activate this if necessary
//		if ( surf->flags & SURFDRAW_BUMPLIGHT )
//		{
//			// bump maps appear left to right in lightmap page memory, calculate 
//			// the offset for the width of a single map. The pixel shader will use 
//			// this to compute the actual texture coordinates
//			builder.TexCoord2f( 2, ctx.m_BumpSTexCoordOffset, 0.0f );
//		}

		Vector& normal = pWorld->GetVertnormals( pWorld->GetVertnormalindices(pWorld->MSurf_FirstVertNormal( surfID ) + i) );
		VectorCopy( normal, pVerts[i].m_Normal );

		if (pWorld->MSurf_Flags( surfID ) & SURFDRAW_TANGENTSPACE )
		{
			Vector tangentS, tangentT;
			TangentSpaceComputeBasis( tangentS, tangentT, normal, tVect, negate );
			VectorCopy( tangentS, pVerts[i].m_TangentS );
			VectorCopy( tangentT, pVerts[i].m_TangentT );
		}
	}
}
#endif // SWDS

void CMSurfaceSortList::Init( int maxSortIDs, int minMaterialLists )
{
	m_list.RemoveAll();
	m_list.EnsureCapacity(minMaterialLists);
	m_maxSortIDs = maxSortIDs;
	int groupMax = maxSortIDs*MAX_MAT_SORT_GROUPS;
	m_groups.RemoveAll();
	m_groups.EnsureCount(groupMax);
	int groupBytes = (groupMax+7)>>3;
	m_groupUsed.EnsureCount(groupBytes);
	Q_memset(m_groupUsed.Base(), 0, groupBytes);

	for ( int i = 0; i < MAX_MAT_SORT_GROUPS; i++ )
	{
		m_sortGroupLists[i].RemoveAll();
		int cap = (i==0) ? 128 : 16;
		m_sortGroupLists[i].EnsureCapacity(cap);
		groupOffset[i] = m_maxSortIDs * i;
	}
	InitGroup(&m_emptyGroup);
}

void CMSurfaceSortList::InitGroup( surfacesortgroup_t *pGroup )
{
	pGroup->listHead = -1;
	pGroup->listTail = -1;
	pGroup->vertexCount = 0;
	pGroup->groupListIndex = -1;
	pGroup->vertexCountNoDetail = 0;
	pGroup->indexCountNoDetail = 0;
	pGroup->triangleCount = 0;
	pGroup->surfaceCount = 0;
}

void CMSurfaceSortList::Shutdown()
{
}

void CMSurfaceSortList::Reset()
{
	Init( m_maxSortIDs, m_list.NumAllocated() );
}

// this resizes the groups and groupUsed arrays
void CMSurfaceSortList::EnsureMaxSortIDs( int newMaxSortIDs )
{
	if ( newMaxSortIDs > m_maxSortIDs )
	{
		int oldMax = m_maxSortIDs;
		// compute new size, expand by minimum of 256
		newMaxSortIDs += 255;
		newMaxSortIDs -= (newMaxSortIDs&255);
		int groupMax = newMaxSortIDs * MAX_MAT_SORT_GROUPS;
		int groupBytes = (groupMax+7)>>3;
		// resize the arrays
		m_groups.EnsureCount(groupMax);
		m_groupUsed.EnsureCount(groupBytes);
		// now loop through the list backwards and move the old data over
		for ( int i = MAX_MAT_SORT_GROUPS; --i >= 0; )
		{
			for ( int j = newMaxSortIDs; --j >= 0; )
			{
				int newIndex = (i * newMaxSortIDs) + j;
				if ( j < oldMax )
				{
					// when i == 0, the group indices overlap so they don't need to be remapped
					if ( i != 0 )
					{
						int oldIndex = (i * oldMax) + j;
						MarkGroupNotUsed(newIndex);
						if ( IsGroupUsed(oldIndex) )
						{
							MarkGroupNotUsed(oldIndex);
							MarkGroupUsed(newIndex);
							m_groups[newIndex] = m_groups[oldIndex];
							InitGroup( &m_groups[oldIndex] );
						}
					}
					if ( IsGroupUsed(newIndex) && m_groups[newIndex].groupListIndex >= 0 )
					{
						m_sortGroupLists[i][m_groups[newIndex].groupListIndex] = &m_groups[newIndex];
					}
				}
				else
				{
					MarkGroupNotUsed(newIndex);
				}
			}
			groupOffset[i] = i*newMaxSortIDs;
		}
		m_maxSortIDs = newMaxSortIDs;
	}
}


void CMSurfaceSortList::AddSurfaceToTail(model_t* pWorld,SurfaceHandle_t pSurface, int sortGroup, int sortID )
{
	Assert(sortGroup<MAX_MAT_SORT_GROUPS);
	int index = groupOffset[sortGroup] + sortID;
	surfacesortgroup_t *pGroup = &m_groups[index];
	if ( !IsGroupUsed(index) )
	{
		MarkGroupUsed(index);
		InitGroup(pGroup);
	}
	materiallist_t *pList = NULL;
	short prevIndex = -1;
	int vertCount = pWorld->MSurf_VertCount(pSurface);
	int triangleCount = vertCount - 2;
	pGroup->triangleCount += triangleCount;
	pGroup->surfaceCount++;
	pGroup->vertexCount += vertCount;
	if (pWorld->MSurf_Flags(pSurface) & SURFDRAW_NODE)
	{
		pGroup->vertexCountNoDetail += vertCount;
		pGroup->indexCountNoDetail += triangleCount * 3;
	}
	if ( pGroup->listTail != m_list.InvalidIndex() )
	{
		// existing block
		pList = &m_list[pGroup->listTail];
		if ( pList->count >= ARRAYSIZE(pList->pSurfaces) )
		{
			prevIndex = pGroup->listTail;
			// no space in existing block
			pList = NULL;
		}
	}
	// use existing block?
	if ( pList )
	{
		pList->pSurfaces[pList->count] = pSurface;
		pList->count++;
	}
	else
	{
		// allocate a new block
		short nextBlock = m_list.AddToTail();
		if ( prevIndex >= 0 )
		{
			m_list[prevIndex].nextBlock = nextBlock;
		}
		pGroup->listTail = nextBlock;
		// handle the first use case
		if ( pGroup->listHead == m_list.InvalidIndex() )
		{
			// UNDONE: This should really be sorted by sortID would help reduce state changes
			// NOTE: Doesn't seem to help much in benchmarks to sort this vector
			index = m_sortGroupLists[sortGroup].AddToTail(pGroup);
			pGroup->groupListIndex = index;
			pGroup->listHead = nextBlock;
		}
		pList = &m_list[nextBlock];
		pList->nextBlock = m_list.InvalidIndex();
		pList->count = 1;
		pList->pSurfaces[0] = pSurface;
	}
}

SurfaceHandle_t CMSurfaceSortList::GetSurfaceAtHead( const surfacesortgroup_t &group ) const
{
	if ( group.listHead == m_list.InvalidIndex() )
		return NULL;
	Assert(m_list[group.listHead].count>0);
	return m_list[group.listHead].pSurfaces[0];
}

void CMSurfaceSortList::GetSurfaceListForGroup( CUtlVector<SurfaceHandle_t> &list, const surfacesortgroup_t &group ) const
{
	MSL_FOREACH_SURFACE_IN_GROUP_BEGIN( *this, group, surfID )
	{
		list.AddToTail(surfID);
	}
	MSL_FOREACH_SURFACE_IN_GROUP_END()
}

#ifndef SWDS
IMaterial *GetMaterialAtCrossHair(model_t* pWorld )
{
	Vector endPoint;
	Vector lightmapColor;

	// max_range * sqrt(3)
	VectorMA( MainViewOrigin(), COORD_EXTENT * 1.74f, MainViewForward(), endPoint );
	
	SurfaceHandle_t hitSurfID = R_LightVec(pWorld, MainViewOrigin(), endPoint, false, lightmapColor );
	if( IS_SURF_VALID( hitSurfID ) )
	{
		return pWorld->MSurf_TexInfo( hitSurfID )->material;
	}
	else
	{
		return NULL;
	}
}

// hack
extern void DrawLightmapPage( int lightmapPageID );

static float textureS, textureT;
static SurfaceHandle_t s_CrossHairSurfID;;
static Vector crossHairDiffuseLightColor;
static Vector crossHairBaseColor;
static float lightmapCoords[2];

void SaveSurfAtCrossHair(model_t* pWorld)
{
	Vector endPoint;
	Vector lightmapColor;

	// max_range * sqrt(3)
	VectorMA(  MainViewOrigin(), COORD_EXTENT * 1.74f, MainViewForward(), endPoint );
	
	s_CrossHairSurfID = R_LightVec(pWorld, MainViewOrigin(), endPoint, false, lightmapColor,
		&textureS, &textureT, &lightmapCoords[0], &lightmapCoords[1] );
}


void DebugDrawLightmapAtCrossHair() // Lol
{
#if 0
	IMaterial *pMaterial;
	int lightmapPageSize[2];

	if( !s_CrossHairSurfID )
		return;
	materials->GetLightmapPageSize( materialSortInfoArray[MSurf_MaterialSortID( s_CrossHairSurfID )].lightmapPageID, 
		&lightmapPageSize[0], &lightmapPageSize[1] );
	pMaterial = MSurf_TexInfo( s_CrossHairSurfID )->material;
//	pMaterial->GetLowResColorSample( textureS, textureT, baseColor );
	DrawLightmapPage( materialSortInfoArray[MSurf_MaterialSortID( s_CrossHairSurfID )].lightmapPageID );

	int i;
	for( i = 0; i < 2; i++ )
	{
		xy[i] = 
			( ( float )pCrossHairSurf->offsetIntoLightmapPage[i] / ( float )lightmapPageSize[i] ) +
			lightmapCoord[i] * ( pCrossHairSurf->lightmapExtents[i] / ( float )lightmapPageSize[i] );
	}

	materials->Bind( g_materialWireframe );
	IMesh* pMesh = materials->GetDynamicMesh( g_materialWireframe );
	
	CMeshBuilder meshBuilder;
	meshBuilder.Begin( pMesh, MATERIAL_QUAD, 1 );

	meshBuilder.Position3f( 
	meshBuilder.AdvanceVertex();

	meshBuilder.End();
	pMesh->Draw();
#endif
}

void ReleaseMaterialSystemObjects();
void RestoreMaterialSystemObjects( int nChangeFlags );

void ForceMatSysRestore()
{
	ReleaseMaterialSystemObjects();
	RestoreMaterialSystemObjects( 0 );
}

#endif
