//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Defines the player specific data that is sent only to the player
//			to whom it belongs.
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_PLAYERLOCALDATA_H
#define C_PLAYERLOCALDATA_H
#ifdef _WIN32
#pragma once
#endif

#include "basetypes.h"
#include "mathlib/vector.h"
#include "playernet_vars.h"

//-----------------------------------------------------------------------------
// Purpose: Player specific data ( sent only to local player, too )
//-----------------------------------------------------------------------------
class CPlayerLocalData
{
public:
	DECLARE_PREDICTABLE();
	DECLARE_CLASS_NOBASE( CPlayerLocalData );
	DECLARE_EMBEDDED_NETWORKVAR();

	CPlayerLocalData() :
		m_iv_vecPunchAngle( "CPlayerLocalData::m_iv_vecPunchAngle" ),
		m_iv_vecPunchAngleVel( "CPlayerLocalData::m_iv_vecPunchAngleVel" )
	{
		m_iv_vecPunchAngle.Setup( &m_vecPunchAngle.m_Value, LATCH_SIMULATION_VAR );
		m_iv_vecPunchAngleVel.Setup( &m_vecPunchAngleVel.m_Value, LATCH_SIMULATION_VAR );
		m_flFOVRate = 0;
	}

	CNetworkArray( unsigned char,			m_chAreaBits,MAX_AREA_STATE_BYTES);				// Area visibility flags.
	CNetworkArray( unsigned char,			m_chAreaPortalBits,MAX_AREA_PORTAL_STATE_BYTES);// Area portal visibility flags.

	CNetworkVar( int,						m_iHideHUD);			// bitfields containing sections of the HUD to hide
	
	CNetworkVar( float,					m_flFOVRate);		// rate at which the FOV changes
	

	CNetworkVar( bool,					m_bDucked);
	CNetworkVar( bool,					m_bDucking);
	CNetworkVar( bool,					m_bInDuckJump);
	CNetworkVar( float,					m_flDucktime);
	CNetworkVar( float,					m_flDuckJumpTime);
	CNetworkVar( float,					m_flJumpTime);
	int						m_nStepside;
	CNetworkVar( float,					m_flFallVelocity);
	int						m_nOldButtons;
	// Base velocity that was passed in to server physics so 
	//  client can predict conveyors correctly.  Server zeroes it, so we need to store here, too.
	Vector					m_vecClientBaseVelocity;  
	CNetworkQAngle( m_vecPunchAngle );		// auto-decaying view angle adjustment
	CInterpolatedVar< QAngle >	m_iv_vecPunchAngle;

	CNetworkQAngle( m_vecPunchAngleVel );		// velocity of auto-decaying view angle adjustment
	CInterpolatedVar< QAngle >	m_iv_vecPunchAngleVel;
	CNetworkVar( bool,					m_bDrawViewmodel);
	CNetworkVar( bool,					m_bWearingSuit);
	CNetworkVar( bool,					m_bPoisoned);
	CNetworkVar( float,					m_flStepSize);
	CNetworkVar( bool,					m_bAllowAutoMovement);

	// 3d skybox
	sky3dparams_t			m_skybox3d;
	// fog params
	fogplayerparams_t		m_PlayerFog;
	// audio environment
	audioparams_t			m_audio;

	bool					m_bSlowMovement;

public:
	BEGIN_INIT_RECV_TABLE(CPlayerLocalData)
	BEGIN_RECV_TABLE_NOBASE(CPlayerLocalData, DT_Local)
		RecvPropArray3(RECVINFO_ARRAY(m_chAreaBits), RecvPropInt(RECVINFO_ARRAY3(m_chAreaBits))),
		RecvPropArray3(RECVINFO_ARRAY(m_chAreaPortalBits), RecvPropInt(RECVINFO_ARRAY3(m_chAreaPortalBits))),
		RecvPropInt(RECVINFO(m_iHideHUD)),

		// View

		RecvPropFloat(RECVINFO(m_flFOVRate)),

		RecvPropInt(RECVINFO(m_bDucked)),
		RecvPropInt(RECVINFO(m_bDucking)),
		RecvPropInt(RECVINFO(m_bInDuckJump)),
		RecvPropFloat(RECVINFO(m_flDucktime)),
		RecvPropFloat(RECVINFO(m_flDuckJumpTime)),
		RecvPropFloat(RECVINFO(m_flJumpTime)),
		RecvPropFloat(RECVINFO(m_flFallVelocity)),

#if PREDICTION_ERROR_CHECK_LEVEL > 1 
		RecvPropFloat(RECVINFO_NAME(m_vecPunchAngle.m_Value[0], m_vecPunchAngle[0])),
		RecvPropFloat(RECVINFO_NAME(m_vecPunchAngle.m_Value[1], m_vecPunchAngle[1])),
		RecvPropFloat(RECVINFO_NAME(m_vecPunchAngle.m_Value[2], m_vecPunchAngle[2])),
		RecvPropFloat(RECVINFO_NAME(m_vecPunchAngleVel.m_Value[0], m_vecPunchAngleVel[0])),
		RecvPropFloat(RECVINFO_NAME(m_vecPunchAngleVel.m_Value[1], m_vecPunchAngleVel[1])),
		RecvPropFloat(RECVINFO_NAME(m_vecPunchAngleVel.m_Value[2], m_vecPunchAngleVel[2])),
#else
		RecvPropVector(RECVINFO(m_vecPunchAngle)),
		RecvPropVector(RECVINFO(m_vecPunchAngleVel)),
#endif

		RecvPropInt(RECVINFO(m_bDrawViewmodel)),
		RecvPropInt(RECVINFO(m_bWearingSuit)),
		RecvPropBool(RECVINFO(m_bPoisoned)),
		RecvPropFloat(RECVINFO(m_flStepSize)),
		RecvPropInt(RECVINFO(m_bAllowAutoMovement)),

		// 3d skybox data
		RecvPropInt(RECVINFO(m_skybox3d.scale)),
		RecvPropVector(RECVINFO(m_skybox3d.origin)),
		RecvPropInt(RECVINFO(m_skybox3d.area)),

		// 3d skybox fog data
		RecvPropInt(RECVINFO(m_skybox3d.fog.enable)),
		RecvPropInt(RECVINFO(m_skybox3d.fog.blend)),
		RecvPropVector(RECVINFO(m_skybox3d.fog.dirPrimary)),
		RecvPropColor32(RECVINFO(m_skybox3d.fog.colorPrimary)),
		RecvPropColor32(RECVINFO(m_skybox3d.fog.colorSecondary)),
		RecvPropFloat(RECVINFO(m_skybox3d.fog.start)),
		RecvPropFloat(RECVINFO(m_skybox3d.fog.end)),
		RecvPropFloat(RECVINFO(m_skybox3d.fog.maxdensity)),

		// fog data
		RecvPropEHandle(RECVINFO(m_PlayerFog.m_hCtrl)),

		// audio data
		RecvPropVector(RECVINFO_STRUCTARRAYELEM(m_audio.localSound,0)),
		RecvPropVector(RECVINFO_STRUCTARRAYELEM(m_audio.localSound,1)),
		RecvPropVector(RECVINFO_STRUCTARRAYELEM(m_audio.localSound,2)),
		RecvPropVector(RECVINFO_STRUCTARRAYELEM(m_audio.localSound,3)),
		RecvPropVector(RECVINFO_STRUCTARRAYELEM(m_audio.localSound,4)),
		RecvPropVector(RECVINFO_STRUCTARRAYELEM(m_audio.localSound,5)),
		RecvPropVector(RECVINFO_STRUCTARRAYELEM(m_audio.localSound,6)),
		RecvPropVector(RECVINFO_STRUCTARRAYELEM(m_audio.localSound,7)),
		RecvPropInt(RECVINFO(m_audio.soundscapeIndex)),
		RecvPropInt(RECVINFO(m_audio.localBits)),
		RecvPropEHandle(RECVINFO(m_audio.ent)),
	END_RECV_TABLE(DT_Local)
	END_INIT_RECV_TABLE()
};

#endif // C_PLAYERLOCALDATA_H
