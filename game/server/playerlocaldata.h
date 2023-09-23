//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef PLAYERLOCALDATA_H
#define PLAYERLOCALDATA_H
#ifdef _WIN32
#pragma once
#endif


#include "playernet_vars.h"
#include "networkvar.h"
#include "fogcontroller.h"

//-----------------------------------------------------------------------------
// Purpose: Player specific data ( sent only to local player, too )
//-----------------------------------------------------------------------------
class CPlayerLocalData
{
public:
	// Save/restore
	DECLARE_SIMPLE_DATADESC();
	// Prediction data copying
	DECLARE_CLASS_NOBASE( CPlayerLocalData );
	DECLARE_EMBEDDED_NETWORKVAR();
	DECLARE_SEND_TABLE_ACCESS(DT_Local);

	CPlayerLocalData();

	void UpdateAreaBits( CBasePlayer *pl, unsigned char chAreaPortalBits[MAX_AREA_PORTAL_STATE_BYTES] );


public:

	CNetworkArray( unsigned char, m_chAreaBits, MAX_AREA_STATE_BYTES );								// Which areas are potentially visible to the client?
	CNetworkArray( unsigned char, m_chAreaPortalBits, MAX_AREA_PORTAL_STATE_BYTES );	// Which area portals are open?

	CNetworkVar( int,	m_iHideHUD );		// bitfields containing sections of the HUD to hide
	CNetworkVar( float, m_flFOVRate );		// rate at which the FOV changes (defaults to 0)
		
	Vector				m_vecOverViewpoint;			// Viewpoint overriding the real player's viewpoint
	
	// Fully ducked
	CNetworkVar( bool, m_bDucked );
	// In process of ducking
	CNetworkVar( bool, m_bDucking );
	// In process of duck-jumping
	CNetworkVar( bool, m_bInDuckJump );
	// During ducking process, amount of time before full duc
	CNetworkVar( float, m_flDucktime );
	CNetworkVar( float, m_flDuckJumpTime );
	// Jump time, time to auto unduck (since we auto crouch jump now).
	CNetworkVar( float, m_flJumpTime );
	// Step sound side flip/flip
	int m_nStepside;;
	// Velocity at time when we hit ground
	CNetworkVar( float, m_flFallVelocity );
	// Previous button state
	int m_nOldButtons;
	class CSkyCamera *m_pOldSkyCamera;
	// Base velocity that was passed in to server physics so 
	//  client can predict conveyors correctly.  Server zeroes it, so we need to store here, too.
	// auto-decaying view angle adjustment
	CNetworkQAngle( m_vecPunchAngle );		
	CNetworkQAngle( m_vecPunchAngleVel );
	// Draw view model for the player
	CNetworkVar( bool, m_bDrawViewmodel );

	// Is the player wearing the HEV suit
	CNetworkVar( bool, m_bWearingSuit );
	CNetworkVar( bool, m_bPoisoned );
	CNetworkVar( float, m_flStepSize );
	CNetworkVar( bool, m_bAllowAutoMovement );

	// 3d skybox
	CNetworkVarEmbedded( sky3dparams_t, m_skybox3d );
	// world fog
	CNetworkVarEmbedded( fogplayerparams_t, m_PlayerFog );
	fogparams_t			m_fog;
	// audio environment
	CNetworkVarEmbedded( audioparams_t, m_audio );

	CNetworkVar( bool, m_bSlowMovement );

	BEGIN_INIT_SEND_TABLE(CPlayerLocalData)
	BEGIN_SEND_TABLE_NOBASE(CPlayerLocalData, DT_Local)

		SendPropArray3(SENDINFO_ARRAY3(m_chAreaBits), SendPropInt(SENDINFO_ARRAY(m_chAreaBits), 8, SPROP_UNSIGNED)),
		SendPropArray3(SENDINFO_ARRAY3(m_chAreaPortalBits), SendPropInt(SENDINFO_ARRAY(m_chAreaPortalBits), 8, SPROP_UNSIGNED)),

		SendPropInt(SENDINFO(m_iHideHUD), HIDEHUD_BITCOUNT, SPROP_UNSIGNED),
		SendPropFloat(SENDINFO(m_flFOVRate), 0, SPROP_NOSCALE),
		SendPropInt(SENDINFO(m_bDucked), 1, SPROP_UNSIGNED),
		SendPropInt(SENDINFO(m_bDucking), 1, SPROP_UNSIGNED),
		SendPropInt(SENDINFO(m_bInDuckJump), 1, SPROP_UNSIGNED),
		SendPropFloat(SENDINFO(m_flDucktime), 12, SPROP_ROUNDDOWN | SPROP_CHANGES_OFTEN, 0.0f, 2048.0f),
		SendPropFloat(SENDINFO(m_flDuckJumpTime), 12, SPROP_ROUNDDOWN, 0.0f, 2048.0f),
		SendPropFloat(SENDINFO(m_flJumpTime), 12, SPROP_ROUNDDOWN, 0.0f, 2048.0f),
#if PREDICTION_ERROR_CHECK_LEVEL > 1 
		SendPropFloat(SENDINFO(m_flFallVelocity), 32, SPROP_NOSCALE),

		SendPropFloat(SENDINFO_VECTORELEM(m_vecPunchAngle, 0), 32, SPROP_NOSCALE | SPROP_CHANGES_OFTEN),
		SendPropFloat(SENDINFO_VECTORELEM(m_vecPunchAngle, 1), 32, SPROP_NOSCALE | SPROP_CHANGES_OFTEN),
		SendPropFloat(SENDINFO_VECTORELEM(m_vecPunchAngle, 2), 32, SPROP_NOSCALE | SPROP_CHANGES_OFTEN),

		SendPropFloat(SENDINFO_VECTORELEM(m_vecPunchAngleVel, 0), 32, SPROP_NOSCALE | SPROP_CHANGES_OFTEN),
		SendPropFloat(SENDINFO_VECTORELEM(m_vecPunchAngleVel, 1), 32, SPROP_NOSCALE | SPROP_CHANGES_OFTEN),
		SendPropFloat(SENDINFO_VECTORELEM(m_vecPunchAngleVel, 2), 32, SPROP_NOSCALE | SPROP_CHANGES_OFTEN),

#else
		SendPropFloat(SENDINFO(m_flFallVelocity), 17, SPROP_CHANGES_OFTEN, -4096.0f, 4096.0f),
		SendPropVector(SENDINFO(m_vecPunchAngle), -1, SPROP_COORD | SPROP_CHANGES_OFTEN),
		SendPropVector(SENDINFO(m_vecPunchAngleVel), -1, SPROP_COORD),
#endif
		SendPropInt(SENDINFO(m_bDrawViewmodel), 1, SPROP_UNSIGNED),
		SendPropInt(SENDINFO(m_bWearingSuit), 1, SPROP_UNSIGNED),
		SendPropBool(SENDINFO(m_bPoisoned)),

		SendPropFloat(SENDINFO(m_flStepSize), 16, SPROP_ROUNDUP, 0.0f, 128.0f),
		SendPropInt(SENDINFO(m_bAllowAutoMovement), 1, SPROP_UNSIGNED),

		// 3d skybox data
		SendPropInt(SENDINFO_STRUCTELEM(m_skybox3d.scale), 12),
		SendPropVector(SENDINFO_STRUCTELEM(m_skybox3d.origin), -1, SPROP_COORD),
		SendPropInt(SENDINFO_STRUCTELEM(m_skybox3d.area), 8, SPROP_UNSIGNED),
		SendPropInt(SENDINFO_STRUCTELEM(m_skybox3d.fog.enable), 1, SPROP_UNSIGNED),
		SendPropInt(SENDINFO_STRUCTELEM(m_skybox3d.fog.blend), 1, SPROP_UNSIGNED),
		SendPropVector(SENDINFO_STRUCTELEM(m_skybox3d.fog.dirPrimary), -1, SPROP_COORD),
		SendPropInt(SENDINFO_STRUCTELEM(m_skybox3d.fog.colorPrimary), 32, SPROP_UNSIGNED),
		SendPropInt(SENDINFO_STRUCTELEM(m_skybox3d.fog.colorSecondary), 32, SPROP_UNSIGNED),
		SendPropFloat(SENDINFO_STRUCTELEM(m_skybox3d.fog.start), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO_STRUCTELEM(m_skybox3d.fog.end), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO_STRUCTELEM(m_skybox3d.fog.maxdensity), 0, SPROP_NOSCALE),

		SendPropEHandle(SENDINFO_STRUCTELEM(m_PlayerFog.m_hCtrl)),

		// audio data
		SendPropVector(SENDINFO_STRUCTARRAYELEM(m_audio.localSound, 0), -1, SPROP_COORD),
		SendPropVector(SENDINFO_STRUCTARRAYELEM(m_audio.localSound, 1), -1, SPROP_COORD),
		SendPropVector(SENDINFO_STRUCTARRAYELEM(m_audio.localSound, 2), -1, SPROP_COORD),
		SendPropVector(SENDINFO_STRUCTARRAYELEM(m_audio.localSound, 3), -1, SPROP_COORD),
		SendPropVector(SENDINFO_STRUCTARRAYELEM(m_audio.localSound, 4), -1, SPROP_COORD),
		SendPropVector(SENDINFO_STRUCTARRAYELEM(m_audio.localSound, 5), -1, SPROP_COORD),
		SendPropVector(SENDINFO_STRUCTARRAYELEM(m_audio.localSound, 6), -1, SPROP_COORD),
		SendPropVector(SENDINFO_STRUCTARRAYELEM(m_audio.localSound, 7), -1, SPROP_COORD),
		SendPropInt(SENDINFO_STRUCTELEM(m_audio.soundscapeIndex), 17, 0),
		SendPropInt(SENDINFO_STRUCTELEM(m_audio.localBits), NUM_AUDIO_LOCAL_SOUNDS, SPROP_UNSIGNED),
		SendPropEHandle(SENDINFO_STRUCTELEM(m_audio.ent)),
	END_SEND_TABLE(DT_Local)
	END_INIT_SEND_TABLE()
};

EXTERN_SEND_TABLE(DT_Local);


#endif // PLAYERLOCALDATA_H
