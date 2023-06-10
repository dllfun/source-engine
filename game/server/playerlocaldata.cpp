//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "playerlocaldata.h"
#include "player.h"
#include "mathlib/mathlib.h"
#include "entitylist.h"
#include "SkyCamera.h"
#include "playernet_vars.h"
#include "fogcontroller.h"
#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================


BEGIN_SIMPLE_DATADESC( fogplayerparams_t )
	DEFINE_FIELD( m_hCtrl, FIELD_EHANDLE ),
	DEFINE_FIELD( m_flTransitionTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_OldColor, FIELD_COLOR32 ),
	DEFINE_FIELD( m_flOldStart, FIELD_FLOAT ),
	DEFINE_FIELD( m_flOldEnd, FIELD_FLOAT ),
	DEFINE_FIELD( m_NewColor, FIELD_COLOR32 ),
	DEFINE_FIELD( m_flNewStart, FIELD_FLOAT ),
	DEFINE_FIELD( m_flNewEnd, FIELD_FLOAT ),
END_DATADESC()

BEGIN_SIMPLE_DATADESC( fogparams_t )

	DEFINE_FIELD( enable, FIELD_BOOLEAN ),
	DEFINE_FIELD( blend, FIELD_BOOLEAN ),
	DEFINE_FIELD( dirPrimary, FIELD_VECTOR ),
	DEFINE_FIELD( colorPrimary, FIELD_COLOR32 ),
	DEFINE_FIELD( colorSecondary, FIELD_COLOR32 ),
	DEFINE_FIELD( start, FIELD_FLOAT ),
	DEFINE_FIELD( end, FIELD_FLOAT ),
	DEFINE_FIELD( farz, FIELD_FLOAT ),
	DEFINE_FIELD( maxdensity, FIELD_FLOAT ),
	DEFINE_FIELD( colorPrimaryLerpTo, FIELD_COLOR32 ),
	DEFINE_FIELD( colorSecondaryLerpTo, FIELD_COLOR32 ),
	DEFINE_FIELD( startLerpTo, FIELD_FLOAT ),
	DEFINE_FIELD( endLerpTo, FIELD_FLOAT ),
	DEFINE_FIELD( lerptime, FIELD_TIME ),
	DEFINE_FIELD( duration, FIELD_FLOAT ),
END_DATADESC()

BEGIN_SIMPLE_DATADESC( sky3dparams_t )

	DEFINE_FIELD( scale, FIELD_INTEGER ),
	DEFINE_FIELD( origin, FIELD_VECTOR ),
	DEFINE_FIELD( area, FIELD_INTEGER ),
	DEFINE_EMBEDDED( fog ),

END_DATADESC()

BEGIN_SIMPLE_DATADESC( audioparams_t )

	DEFINE_AUTO_ARRAY( localSound, FIELD_VECTOR ),
	DEFINE_FIELD( soundscapeIndex, FIELD_INTEGER ),
	DEFINE_FIELD( localBits, FIELD_INTEGER ),
	DEFINE_FIELD( ent, FIELD_EHANDLE ),

END_DATADESC()

BEGIN_SIMPLE_DATADESC( CPlayerLocalData )
	DEFINE_AUTO_ARRAY( m_chAreaBits, FIELD_CHARACTER ),
	DEFINE_AUTO_ARRAY( m_chAreaPortalBits, FIELD_CHARACTER ),
	DEFINE_FIELD( m_iHideHUD, FIELD_INTEGER ),
	DEFINE_FIELD( m_flFOVRate, FIELD_FLOAT ),
	DEFINE_FIELD( m_vecOverViewpoint, FIELD_VECTOR ),
	DEFINE_FIELD( m_bDucked, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bDucking, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bInDuckJump, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flDucktime, FIELD_TIME ),
	DEFINE_FIELD( m_flDuckJumpTime, FIELD_TIME ),
	DEFINE_FIELD( m_flJumpTime, FIELD_TIME ),
	DEFINE_FIELD( m_nStepside, FIELD_INTEGER ),
	DEFINE_FIELD( m_flFallVelocity, FIELD_FLOAT ),
	DEFINE_FIELD( m_nOldButtons, FIELD_INTEGER ),
	DEFINE_FIELD( m_vecPunchAngle, FIELD_VECTOR ),
	DEFINE_FIELD( m_vecPunchAngleVel, FIELD_VECTOR ),
	DEFINE_FIELD( m_bDrawViewmodel, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bWearingSuit, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bPoisoned, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flStepSize, FIELD_FLOAT ),
	DEFINE_FIELD( m_bAllowAutoMovement, FIELD_BOOLEAN ),
	DEFINE_EMBEDDED( m_skybox3d ),
	DEFINE_EMBEDDED( m_PlayerFog ),
	DEFINE_EMBEDDED( m_fog ),
	DEFINE_EMBEDDED( m_audio ),
	
	// "Why don't we save this field, grandpa?"
	//
	// "You see Billy, trigger_vphysics_motion uses vPhysics to touch the player,
	// so if the trigger is Disabled via an input while the player is inside it,
	// the trigger won't get its EndTouch until the player moves. Since touchlinks
	// aren't saved and restored, if the we save before EndTouch is called, it
	// will never be called after we load."
	//DEFINE_FIELD( m_bSlowMovement, FIELD_BOOLEAN ),
	
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CPlayerLocalData::CPlayerLocalData()
{
#ifdef _DEBUG
	m_vecOverViewpoint.Init();
	m_vecPunchAngle.Init();
#endif
	m_audio.soundscapeIndex = 0;
	m_audio.localBits = 0;
	m_audio.ent.Set( NULL );
	m_pOldSkyCamera = NULL;
	m_bDrawViewmodel = true;
}


void CPlayerLocalData::UpdateAreaBits( CBasePlayer *pl, unsigned char chAreaPortalBits[MAX_AREA_PORTAL_STATE_BYTES] )
{
	Vector origin = pl->EyePosition();

	unsigned char tempBits[32];
	COMPILE_TIME_ASSERT( sizeof( tempBits ) >= sizeof( ((CPlayerLocalData*)0)->m_chAreaBits ) );

	int i;
	int area = engineServer->GetArea( origin );
	engineServer->GetAreaBits( area, tempBits, sizeof( tempBits ) );
	for ( i=0; i < m_chAreaBits.Count(); i++ )
	{
		if ( tempBits[i] != m_chAreaBits[ i ] )
		{
			m_chAreaBits.Set( i, tempBits[i] );
		}
	}

	for ( i=0; i < MAX_AREA_PORTAL_STATE_BYTES; i++ )
	{
		if ( chAreaPortalBits[i] != m_chAreaPortalBits[i] )
			m_chAreaPortalBits.Set( i, chAreaPortalBits[i] );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Fills in CClientData values for local player just before sending over wire
// Input  : player - 
//-----------------------------------------------------------------------------

void ClientData_Update( CBasePlayer *pl )
{
	// HACKHACK: for 3d skybox 
	// UNDONE: Support multiple sky cameras?
	CSkyCamera *pSkyCamera = GetCurrentSkyCamera();
	if ( pSkyCamera != pl->m_Local.m_pOldSkyCamera )
	{
		pl->m_Local.m_pOldSkyCamera = pSkyCamera;
		pl->m_Local.m_skybox3d.CopyFrom(pSkyCamera->m_skyboxData);
	}
	else if ( !pSkyCamera )
	{
		pl->m_Local.m_skybox3d.area = 255;
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void UpdateAllClientData( void )
{
	VPROF( "UpdateAllClientData" );
	int i;
	CBasePlayer *pl;

	for ( i = 1; i <= gpGlobals->GetMaxClients(); i++ )
	{
		pl = ( CBasePlayer * )UTIL_PlayerByIndex( i );
		if ( !pl )
			continue;

		ClientData_Update( pl );
	}
}

