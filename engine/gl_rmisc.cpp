//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//===========================================================================//

// HDRFIXME: reduce the number of include files here.
#include "render_pch.h"
#include "cdll_int.h"
#include "client_class.h"
#include "icliententitylist.h"
#include "traceinit.h"
#include "server.h"
#include "r_decal.h"
#include "r_areaportal.h"
#include "ispatialpartitioninternal.h"
#include "cdll_engine_int.h"
#include "ivtex.h"
#include "materialsystem/itexture.h"
#include "view.h"
#include "tier0/dbg.h"
#include "staticpropmgr.h"
#include "icliententity.h"
#include "gl_drawlights.h"
#include "Overlay.h"
#include "vmodes.h"
#include "tier1/utlbuffer.h"
#include "vtf/vtf.h"
#include "bitmap/imageformat.h"
#include "cbenchmark.h"
#include "ivideomode.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "../utils/common/bsplib.h"
#include "ibsppack.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void Linefile_Read_f(void);

/*
====================
//	if( r_drawtranslucentworld )
//	{
//		bSaveDrawTranslucentWorld = r_drawtranslucentworld->GetBool();
		// NOTE! : We use to set this to 0 for HDR.
		//		r_drawtranslucentworld->SetValue( 0 );
//	}
//	if( r_drawtranslucentrenderables )
R_TimeRefresh_f

For program optimization
====================
*/
void R_TimeRefresh_f (void)
{
	int			i;
	float		start, stop, time;
	CViewSetup		view;

	materials->Flush( true );

	Q_memset(&view, 0, sizeof(view));
	view.origin = MainViewOrigin();
	view.angles[0] = 0;
	view.angles[1] = 0;
	view.angles[2] = 0;
	view.x = 0;
	view.y = 0;
	view.width = videomode->GetModeStereoWidth();
	view.height = videomode->GetModeStereoHeight();
	view.fov = 75;
	view.fovViewmodel = 75;
	view.m_flAspectRatio = 1.0f;
	view.zNear = 4;
	view.zFar = MAX_COORD_FLOAT;
	view.zNearViewmodel = 4;
	view.zFarViewmodel = MAX_COORD_FLOAT;

	int savedeveloper = developer.GetInt();
	developer.SetValue( 0 );

	start = Sys_FloatTime ();
	for (i=0 ; i<128 ; i++)
	{
		view.angles[1] = i/128.0*360.0;
		g_ClientDLL->RenderView( view, VIEW_CLEAR_COLOR, RENDERVIEW_DRAWVIEWMODEL | RENDERVIEW_DRAWHUD );
		Shader_SwapBuffers();
	}

	materials->Flush( true );
	Shader_SwapBuffers();
	stop = Sys_FloatTime ();
	time = stop-start;

	developer.SetValue( savedeveloper );

	ConMsg ("%f seconds (%f fps)\n", time, 128/time);
}

ConCommand timerefresh("timerefresh", R_TimeRefresh_f, "Profile the renderer.", FCVAR_CHEAT );	
ConCommand linefile("linefile", Linefile_Read_f, "Parses map leak data from .lin file", FCVAR_CHEAT );	

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void R_Init ()
{	
	extern byte *hunk_base;

	extern void InitMathlib( void );

	InitMathlib();

	UpdateMaterialSystemConfig();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void R_Shutdown( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void R_ResetLightStyles( void )
{
	for ( int i=0 ; i<256 ; i++ )
	{
		d_lightstylevalue[i] = 264;
		d_lightstyleframe[i] = r_framecount;
	}
}

void R_RemoveAllDecalsFromAllModels();


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CON_COMMAND_F( r_cleardecals, "Usage r_cleardecals <permanent>.", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_SERVER_CAN_EXECUTE  )
{
	if (g_pHost->Host_GetWorldModel())
	{
		bool bPermanent = false;
		if ( args.ArgC() == 2 )
		{
			if ( !Q_stricmp( args[1], "permanent" ) )
			{
				bPermanent = true;
			}
		}

		R_DecalTerm(g_pHost->Host_GetWorldModel(), bPermanent );
	}

	R_RemoveAllDecalsFromAllModels();
}


//-----------------------------------------------------------------------------
// Loads world geometry. Called when map changes or dx level changes
//-----------------------------------------------------------------------------
void R_LoadWorldGeometry(model_t* pWorld, bool bDXChange )
{
	// Recreate the sortinfo arrays ( ack, uses new/delete right now ) because doing it with Hunk_AllocName will
	//  leak through every connect that doesn't wipe the hunk ( "reconnect" )
	MaterialSystem_DestroySortinfo();

	MaterialSystem_RegisterLightmapSurfaces(pWorld);

	MaterialSystem_CreateSortinfo(pWorld);

	// UNDONE: This is a really crappy place to do this - shouldn't this stuff be in the modelloader?

	// If this is the first time we've tried to render this map, create a few one-time data structures
	// These all get cleared out if Map_UnloadModel is ever called by the modelloader interface ( and that happens
	//  any time we free the Hunk down to the low mark since these things all use the Hunk for their data ).
	if ( !bDXChange )
	{
		if ( !modelloader->Map_GetRenderInfoAllocated() )
		{
			// create the displacement surfaces for the map
			modelloader->Map_LoadDisplacements(pWorld, false );
			//if( !DispInfo_CreateStaticBuffers( host_state.worldmodel, materialSortInfoArray, false ) )
			//	Sys_Error( "Can't create static meshes for displacements" );
			
			modelloader->Map_SetRenderInfoAllocated( true );
		}
	}
	else
	{
		// create the displacement surfaces for the map
		modelloader->Map_LoadDisplacements(pWorld, true );
	}

	if ( bDXChange )
	{
		// Must be done before MarkWaterSurfaces
		modelloader->RecomputeSurfaceFlags(pWorld);
	}

	Mod_MarkWaterSurfaces(pWorld);

	// make sure and rebuild lightmaps when the level gets started.
	GL_RebuildLightmaps();

	if ( bDXChange )
	{
		R_BrushBatchInit(pWorld);
		R_DecalReSortMaterials(pWorld);
		OverlayMgr()->ReSortMaterials(pWorld);
	}
}


/*
===============
R_LevelInit
===============
*/
void R_LevelInit( model_t* pWorld )
{
	ConDMsg( "Initializing renderer...\n" );

	COM_TimestampedLog( "R_LevelInit: Start" );

	Assert( g_ClientDLL );

	r_framecount = 1; 
	R_ResetLightStyles();
	R_DecalInit(pWorld);
	R_LoadSkys();
	R_InitStudio(pWorld);

	// FIXME: Is this the best place to initialize the kd tree when we're client-only?
	if ( !sv.IsActive() )
	{
		g_pShadowMgr->LevelShutdown();
		StaticPropMgr()->LevelShutdown();
		SpatialPartition()->Init(pWorld->GetMins(), pWorld->GetMaxs());
		StaticPropMgr()->LevelInit();
		g_pShadowMgr->LevelInit(pWorld->GetSurfacesCount() );
	}

	// We've fully loaded the new level, unload any models that we don't care about any more
	modelloader->UnloadUnreferencedModels();

	if (pWorld->GetWorldlightsCount() == 0)
	{
		ConDMsg( "Level unlit, setting 'mat_fullbright 1'\n" );
		mat_fullbright.SetValue( 1 );
	}

	UpdateMaterialSystemConfig();
	
	// FIXME: E3 2003 HACK
	if ( IsPC() && mat_levelflush.GetBool() )
	{
		bool bOnLevelShutdown = false;
		materials->ResetTempHWMemory( bOnLevelShutdown );
	}

	// precache any textures that are used in this map.
	// this is a no-op for textures that are already cached from the previous map.
	materials->CacheUsedMaterials();

	// Loads the world geometry
	R_LoadWorldGeometry(pWorld);

	R_Surface_LevelInit(pWorld);
	R_Areaportal_LevelInit(pWorld);
	
	// Build the overlay fragments.
	OverlayMgr()->CreateFragments(pWorld);

#ifdef _XBOX
	extern void CompactTextureHeap();
	CompactTextureHeap();
#endif

	COM_TimestampedLog( "R_LevelInit: Finish" );
}

void R_LevelShutdown()
{
	R_Surface_LevelShutdown();
	R_Areaportal_LevelShutdown();
	g_DispLightmapSamplePositions.Purge();
}



