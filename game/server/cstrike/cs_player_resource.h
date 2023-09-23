//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: CS's custom CPlayerResource
//
// $NoKeywords: $
//=============================================================================//

#ifndef CS_PLAYER_RESOURCE_H
#define CS_PLAYER_RESOURCE_H
#ifdef _WIN32
#pragma once
#endif

#include <coordsize.h>

class CCSPlayerResource : public CPlayerResource
{
	DECLARE_CLASS( CCSPlayerResource, CPlayerResource );
	
public:
	DECLARE_SERVERCLASS();
	DECLARE_SEND_TABLE_ACCESS(DT_CSPlayerResource);
	DECLARE_DATADESC();

	CCSPlayerResource();

	virtual void UpdatePlayerData( void );
	virtual void Spawn( void );
protected:

	CNetworkVar( int, m_iPlayerC4 );  // entity index of C4 carrier or 0
	CNetworkVar( int, m_iPlayerVIP ); // entity index of VIP player or 0
	CNetworkVector( m_vecC4 );  // position of C4
	CNetworkArray( bool, m_bHostageAlive, MAX_HOSTAGES );
	CNetworkArray( bool, m_isHostageFollowingSomeone, MAX_HOSTAGES );
	CNetworkArray( int, m_iHostageEntityIDs, MAX_HOSTAGES );
	CNetworkArray( int, m_iHostageX, MAX_HOSTAGES );
	CNetworkArray( int, m_iHostageY, MAX_HOSTAGES );
	CNetworkArray( int, m_iHostageZ, MAX_HOSTAGES );
	CNetworkVector( m_bombsiteCenterA );// Location of bombsite A
	CNetworkVector( m_bombsiteCenterB );// Location of bombsite B
	CNetworkArray( int, m_hostageRescueX, MAX_HOSTAGE_RESCUES );// Locations of all hostage rescue spots
	CNetworkArray( int, m_hostageRescueY, MAX_HOSTAGE_RESCUES );
	CNetworkArray( int, m_hostageRescueZ, MAX_HOSTAGE_RESCUES );

	CNetworkVar( bool, m_bBombSpotted );
	CNetworkArray( bool, m_bPlayerSpotted, MAX_PLAYERS+1 );

	CNetworkArray( string_t, m_szClan, MAX_PLAYERS+1 );

	CNetworkArray( int, m_iMVPs, MAX_PLAYERS + 1 );
	CNetworkArray( bool, m_bHasDefuser, MAX_PLAYERS + 1);

private:
	bool m_foundGoalPositions;

public:
	BEGIN_INIT_SEND_TABLE(CCSPlayerResource)
	BEGIN_SEND_TABLE(CCSPlayerResource, DT_CSPlayerResource, DT_PlayerResource)
		SendPropInt(SENDINFO(m_iPlayerC4), 8, SPROP_UNSIGNED),
		SendPropInt(SENDINFO(m_iPlayerVIP), 8, SPROP_UNSIGNED),
		SendPropVector(SENDINFO(m_vecC4), -1, SPROP_COORD),
		SendPropArray3(SENDINFO_ARRAY3(m_bHostageAlive), SendPropInt(SENDINFO_ARRAY(m_bHostageAlive), 1, SPROP_UNSIGNED)),
		SendPropArray3(SENDINFO_ARRAY3(m_isHostageFollowingSomeone), SendPropInt(SENDINFO_ARRAY(m_isHostageFollowingSomeone), 1, SPROP_UNSIGNED)),
		SendPropArray3(SENDINFO_ARRAY3(m_iHostageEntityIDs), SendPropInt(SENDINFO_ARRAY(m_iHostageEntityIDs), -1, SPROP_UNSIGNED)),
		SendPropArray3(SENDINFO_ARRAY3(m_iHostageY), SendPropInt(SENDINFO_ARRAY(m_iHostageY), COORD_INTEGER_BITS + 1, 0)),
		SendPropArray3(SENDINFO_ARRAY3(m_iHostageX), SendPropInt(SENDINFO_ARRAY(m_iHostageX), COORD_INTEGER_BITS + 1, 0)),
		SendPropArray3(SENDINFO_ARRAY3(m_iHostageZ), SendPropInt(SENDINFO_ARRAY(m_iHostageZ), COORD_INTEGER_BITS + 1, 0)),
		SendPropVector(SENDINFO(m_bombsiteCenterA), -1, SPROP_COORD),
		SendPropVector(SENDINFO(m_bombsiteCenterB), -1, SPROP_COORD),
		SendPropArray3(SENDINFO_ARRAY3(m_hostageRescueX), SendPropInt(SENDINFO_ARRAY(m_hostageRescueX), COORD_INTEGER_BITS + 1, 0)),
		SendPropArray3(SENDINFO_ARRAY3(m_hostageRescueY), SendPropInt(SENDINFO_ARRAY(m_hostageRescueY), COORD_INTEGER_BITS + 1, 0)),
		SendPropArray3(SENDINFO_ARRAY3(m_hostageRescueZ), SendPropInt(SENDINFO_ARRAY(m_hostageRescueZ), COORD_INTEGER_BITS + 1, 0)),
		SendPropBool(SENDINFO(m_bBombSpotted)),
		SendPropArray3(SENDINFO_ARRAY3(m_bPlayerSpotted), SendPropInt(SENDINFO_ARRAY(m_bPlayerSpotted), 1, SPROP_UNSIGNED)),
		SendPropArray3(SENDINFO_ARRAY3(m_iMVPs), SendPropInt(SENDINFO_ARRAY(m_iMVPs), COORD_INTEGER_BITS + 1, SPROP_UNSIGNED)),
		SendPropArray3(SENDINFO_ARRAY3(m_bHasDefuser), SendPropInt(SENDINFO_ARRAY(m_bHasDefuser), 1, SPROP_UNSIGNED)),
		SendPropArray3(SENDINFO_ARRAY3(m_szClan), SendPropStringT(SENDINFO_ARRAY(m_szClan))),
	END_SEND_TABLE(DT_CSPlayerResource)
	END_INIT_SEND_TABLE()
};

#endif // CS_PLAYER_RESOURCE_H
