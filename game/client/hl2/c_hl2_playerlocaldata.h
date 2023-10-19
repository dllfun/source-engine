//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $NoKeywords: $
//=============================================================================//

#if !defined( C_HL2_PLAYERLOCALDATA_H )
#define C_HL2_PLAYERLOCALDATA_H
#ifdef _WIN32
#pragma once
#endif


#include "dt_recv.h"

#include "hl2/hl_movedata.h"

EXTERN_RECV_TABLE( DT_HL2Local );


class C_HL2PlayerLocalData
{
public:
	DECLARE_PREDICTABLE();
	DECLARE_CLASS_NOBASE( C_HL2PlayerLocalData );
	DECLARE_EMBEDDED_NETWORKVAR();

	C_HL2PlayerLocalData();

	float	m_flSuitPower;
	bool	m_bZooming;
	int		m_bitsActiveDevices;
	int		m_iSquadMemberCount;
	int		m_iSquadMedicCount;
	bool	m_fSquadInFollowMode;
	bool	m_bWeaponLowered;
	EHANDLE m_hAutoAimTarget;
	Vector	m_vecAutoAimPoint;
	bool	m_bDisplayReticle;
	bool	m_bStickyAutoAim;
	bool	m_bAutoAimTarget;
#ifdef HL2_EPISODIC
	float	m_flFlashBattery;
	Vector	m_vecLocatorOrigin;
#endif

	// Ladder related data
	EHANDLE			m_hLadder;
	LadderMove_t	m_LadderMove;

public:
	BEGIN_INIT_RECV_TABLE(C_HL2PlayerLocalData)
	BEGIN_RECV_TABLE_NOBASE(C_HL2PlayerLocalData, DT_HL2Local)
		RecvPropFloat(RECVINFO(m_flSuitPower)),
		RecvPropInt(RECVINFO(m_bZooming)),
		RecvPropInt(RECVINFO(m_bitsActiveDevices)),
		RecvPropInt(RECVINFO(m_iSquadMemberCount)),
		RecvPropInt(RECVINFO(m_iSquadMedicCount)),
		RecvPropBool(RECVINFO(m_fSquadInFollowMode)),
		RecvPropBool(RECVINFO(m_bWeaponLowered)),
		RecvPropEHandle(RECVINFO(m_hAutoAimTarget)),
		RecvPropVector(RECVINFO(m_vecAutoAimPoint)),
		RecvPropEHandle(RECVINFO(m_hLadder)),
		RecvPropBool(RECVINFO(m_bDisplayReticle)),
		RecvPropBool(RECVINFO(m_bStickyAutoAim)),
		RecvPropBool(RECVINFO(m_bAutoAimTarget)),
#ifdef HL2_EPISODIC
		RecvPropFloat(RECVINFO(m_flFlashBattery)),
		RecvPropVector(RECVINFO(m_vecLocatorOrigin)),
#endif
	END_RECV_TABLE()
	END_INIT_RECV_TABLE()
};


#endif
