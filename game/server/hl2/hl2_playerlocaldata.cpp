//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hl2_playerlocaldata.h"
#include "hl2_player.h"
#include "mathlib/mathlib.h"
#include "entitylist.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"



BEGIN_SIMPLE_DATADESC( CHL2PlayerLocalData )
	DEFINE_FIELD( m_flSuitPower, FIELD_FLOAT ),
	DEFINE_FIELD( m_bZooming, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bitsActiveDevices, FIELD_INTEGER ),
	DEFINE_FIELD( m_iSquadMemberCount, FIELD_INTEGER ),
	DEFINE_FIELD( m_iSquadMedicCount, FIELD_INTEGER ),
	DEFINE_FIELD( m_fSquadInFollowMode, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bWeaponLowered, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bDisplayReticle, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bStickyAutoAim, FIELD_BOOLEAN ),
#ifdef HL2_EPISODIC
	DEFINE_FIELD( m_flFlashBattery, FIELD_FLOAT ),
	DEFINE_FIELD( m_vecLocatorOrigin, FIELD_POSITION_VECTOR ),
#endif
	// Ladder related stuff
	DEFINE_FIELD( m_hLadder, FIELD_EHANDLE ),
	DEFINE_EMBEDDED( m_LadderMove ),
END_DATADESC()

CHL2PlayerLocalData::CHL2PlayerLocalData()
{
	m_flSuitPower = 0.0;
	m_bZooming = false;
	m_bWeaponLowered = false;
	m_hAutoAimTarget.Set(NULL);
	m_hLadder.Set(NULL);
	m_vecAutoAimPoint.GetForModify().Init();
	m_bDisplayReticle = false;
#ifdef HL2_EPISODIC
	m_flFlashBattery = 0.0f;
#endif
}

