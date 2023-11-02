//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef HL2_PLAYERLOCALDATA_H
#define HL2_PLAYERLOCALDATA_H
#ifdef _WIN32
#pragma once
#endif

#include "networkvar.h"

#include "hl_movedata.h"

//-----------------------------------------------------------------------------
// Purpose: Player specific data for HL2 ( sent only to local player, too )
//-----------------------------------------------------------------------------
class CHL2PlayerLocalData
{
public:
	// Save/restore
	DECLARE_SIMPLE_DATADESC();
	DECLARE_CLASS_NOBASE( CHL2PlayerLocalData );
	DECLARE_EMBEDDED_NETWORKVAR();

	CHL2PlayerLocalData();

	CNetworkVar( float, m_flSuitPower );
	CNetworkVar( bool,	m_bZooming );
	CNetworkVar( int,	m_bitsActiveDevices );
	CNetworkVar( int,	m_iSquadMemberCount );
	CNetworkVar( int,	m_iSquadMedicCount );
	CNetworkVar( bool,	m_fSquadInFollowMode );
	CNetworkVar( bool,	m_bWeaponLowered );
	CNetworkVar( EHANDLE, m_hAutoAimTarget );
	CNetworkVector( m_vecAutoAimPoint );
	CNetworkVar( bool,	m_bDisplayReticle );
	CNetworkVar( bool,	m_bStickyAutoAim );
	CNetworkVar( bool,	m_bAutoAimTarget );
#ifdef HL2_EPISODIC
	CNetworkVar( float, m_flFlashBattery );
	CNetworkVector( m_vecLocatorOrigin );
#endif

	// Ladder related data
	CNetworkVar( EHANDLE, m_hLadder );
	LadderMove_t			m_LadderMove;

public:
	BEGIN_INIT_SEND_TABLE(CHL2PlayerLocalData)
	BEGIN_SEND_TABLE_NOBASE(CHL2PlayerLocalData, DT_HL2Local)
		SendPropFloat(SENDINFO(m_flSuitPower), 10, SPROP_UNSIGNED | SPROP_ROUNDUP, 0.0, 100.0),
		SendPropInt(SENDINFO(m_bZooming), 1, SPROP_UNSIGNED),
		SendPropInt(SENDINFO(m_bitsActiveDevices), MAX_SUIT_DEVICES, SPROP_UNSIGNED),
		SendPropInt(SENDINFO(m_iSquadMemberCount)),
		SendPropInt(SENDINFO(m_iSquadMedicCount)),
		SendPropBool(SENDINFO(m_fSquadInFollowMode)),
		SendPropBool(SENDINFO(m_bWeaponLowered)),
		SendPropEHandle(SENDINFO(m_hAutoAimTarget)),
		SendPropVector(SENDINFO(m_vecAutoAimPoint)),
		SendPropEHandle(SENDINFO(m_hLadder)),
		SendPropBool(SENDINFO(m_bDisplayReticle)),
		SendPropBool(SENDINFO(m_bStickyAutoAim)),
		SendPropBool(SENDINFO(m_bAutoAimTarget)),
#ifdef HL2_EPISODIC
		SendPropFloat(SENDINFO(m_flFlashBattery)),
		SendPropVector(SENDINFO(m_vecLocatorOrigin)),
#endif
	END_SEND_TABLE()
	END_INIT_SEND_TABLE()
};

EXTERN_SEND_TABLE(DT_HL2Local);


#endif // HL2_PLAYERLOCALDATA_H
