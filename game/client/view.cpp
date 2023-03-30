//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#include "cbase.h"
#include "view.h"
#include "iviewrender.h"
#include "iviewrender_beams.h"
#include "view_shared.h"
#include "ivieweffects.h"
#include "iinput.h"
#include "iclientmode.h"
#include "prediction.h"
#include "viewrender.h"
#include "c_te_legacytempents.h"
#include "cl_mat_stub.h"
#include "tier0/vprof.h"
#include "iclientvehicle.h"
#include "engine/IEngineTrace.h"
#include "mathlib/vmatrix.h"
#include "rendertexture.h"
#include "c_world.h"
#include <KeyValues.h>
#include "igameevents.h"
#include "smoke_fog_overlay.h"
#include "bitmap/tgawriter.h"
#include "hltvcamera.h"
#if defined( REPLAY_ENABLED )
#include "replay/replaycamera.h"
#include "replay/replay_screenshot.h"
#endif
#include "input.h"
#include "filesystem.h"
#include "materialsystem/itexture.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/materialsystem_config.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "toolframework_client.h"
#include "tier0/icommandline.h"
#include "ienginevgui.h"
#include <vgui_controls/Controls.h>
#include <vgui/ISurface.h>
#include "ScreenSpaceEffects.h"
#include "sourcevr/isourcevirtualreality.h"
#include "client_virtualreality.h"

#if defined( REPLAY_ENABLED )
#include "replay/ireplaysystem.h"
#include "replay/ienginereplay.h"
#endif

#if defined( HL2_CLIENT_DLL ) || defined( CSTRIKE_DLL )
#define USE_MONITORS
#endif

#ifdef PORTAL
#include "c_prop_portal.h" //portal surface rendering functions
#endif

	
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
		  
void ToolFramework_AdjustEngineViewport( int& x, int& y, int& width, int& height );
bool ToolFramework_SetupEngineView( Vector &origin, QAngle &angles, float &fov );
bool ToolFramework_SetupEngineMicrophone( Vector &origin, QAngle &angles );

//extern bool g_bRenderingScreenshot;

#if _DEBUG
bool g_bRenderingCameraView = false;
#endif


#ifdef TF_CLIENT_DLL
// 54 degrees approximates a 35mm camera - we determined that this makes the viewmodels
// and motions look the most natural.
ConVar v_viewmodel_fov( "viewmodel_fov", "54", FCVAR_ARCHIVE );
#else
ConVar v_viewmodel_fov( "viewmodel_fov", "54", FCVAR_CHEAT );
#endif
ConVar mat_viewportscale( "mat_viewportscale", "1.0", FCVAR_ARCHIVE, "Scale down the main viewport (to reduce GPU impact on CPU profiling)", true, (1.0f / 640.0f), true, 1.0f );
ConVar mat_viewportupscale( "mat_viewportupscale", "1", FCVAR_ARCHIVE, "Scale the viewport back up" );
ConVar cl_leveloverview( "cl_leveloverview", "0", FCVAR_CHEAT );


void SoftwareCursorChangedCB( IConVar *pVar, const char *pOldValue, float fOldValue )
{
	ConVar *pConVar = (ConVar *)pVar;
	vgui::ivgui()->SetSoftwareCursor( pConVar->GetBool() || UseVR() );
}
static ConVar cl_software_cursor ( "cl_software_cursor", "0", FCVAR_ARCHIVE, "Switches the game to use a larger software cursor instead of the normal OS cursor", SoftwareCursorChangedCB );



//-----------------------------------------------------------------------------
// Compute the world->camera transform
//-----------------------------------------------------------------------------
void ComputeCameraVariables( const Vector &vecOrigin, const QAngle &vecAngles, 
	Vector *pVecForward, Vector *pVecRight, Vector *pVecUp, VMatrix *pMatCamInverse )
{
	// Compute view bases
	AngleVectors( vecAngles, pVecForward, pVecRight, pVecUp );

	for (int i = 0; i < 3; ++i)
	{
		(*pMatCamInverse)[0][i] = (*pVecRight)[i];	
		(*pMatCamInverse)[1][i] = (*pVecUp)[i];	
		(*pMatCamInverse)[2][i] = -(*pVecForward)[i];	
		(*pMatCamInverse)[3][i] = 0.0F;
	}
	(*pMatCamInverse)[0][3] = -DotProduct( *pVecRight, vecOrigin );
	(*pMatCamInverse)[1][3] = -DotProduct( *pVecUp, vecOrigin );
	(*pMatCamInverse)[2][3] =  DotProduct( *pVecForward, vecOrigin );
	(*pMatCamInverse)[3][3] = 1.0F;
}


bool R_CullSphere(
	VPlane const *pPlanes,
	int nPlanes,
	Vector const *pCenter,
	float radius)
{
	for(int i=0; i < nPlanes; i++)
		if(pPlanes[i].DistTo(*pCenter) < -radius)
			return true;
	
	return false;
}

float ScaleFOVByWidthRatio(float fovDegrees, float ratio)
{
	float halfAngleRadians = fovDegrees * (0.5f * M_PI / 180.0f);
	float t = tan(halfAngleRadians);
	t *= ratio;
	float retDegrees = (180.0f / M_PI) * atan(t);
	return retDegrees * 2.0f;
}

static void GetPos( const CCommand &args, Vector &vecOrigin, QAngle &angles )
{
	vecOrigin = g_pView->MainViewOrigin();
	angles = g_pView->MainViewAngles();
	if ( args.ArgC() == 2 && atoi( args[1] ) == 2 )
	{
		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
		if ( pPlayer )
		{
			vecOrigin = pPlayer->GetAbsOrigin();
			angles = pPlayer->GetAbsAngles();
		}
	}
}

CON_COMMAND( spec_pos, "dump position and angles to the console" )
{
	Vector vecOrigin;
	QAngle angles;
	GetPos( args, vecOrigin, angles );
	Warning( "spec_goto %.1f %.1f %.1f %.1f %.1f\n", vecOrigin.x, vecOrigin.y, 
		vecOrigin.z, angles.x, angles.y );
}

CON_COMMAND( getpos, "dump position and angles to the console" )
{
	Vector vecOrigin;
	QAngle angles;
	GetPos( args, vecOrigin, angles );

	const char *pCommand1 = "setpos";
	const char *pCommand2 = "setang";
	if ( args.ArgC() == 2 && atoi( args[1] ) == 2 )
	{
		pCommand1 = "setpos_exact";
		pCommand2 = "setang_exact";
	}

	Warning( "%s %f %f %f;", pCommand1, vecOrigin.x, vecOrigin.y, vecOrigin.z );
	Warning( "%s %f %f %f\n", pCommand2, angles.x, angles.y, angles.z );
}

