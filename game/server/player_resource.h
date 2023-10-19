//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Entity that propagates general data needed by clients for every player.
//
// $NoKeywords: $
//=============================================================================//

#ifndef PLAYER_RESOURCE_H
#define PLAYER_RESOURCE_H
#ifdef _WIN32
#pragma once
#endif

#include "shareddefs.h"
#include "baseentity.h"


class CPlayerResource : public CBaseEntity
{
	DECLARE_CLASS( CPlayerResource, CBaseEntity );
public:
	DECLARE_SERVERCLASS();
	DECLARE_SEND_TABLE_ACCESS(DT_PlayerResource);
	DECLARE_DATADESC();

	virtual void Spawn( void );
	virtual	int	 ObjectCaps( void ) { return BaseClass::ObjectCaps() | FCAP_DONT_SAVE; }
	virtual void ResourceThink( void );
	virtual void UpdatePlayerData( void );
	virtual int  UpdateTransmitState(void);

protected:
	// Data for each player that's propagated to all clients
	// Stored in individual arrays so they can be sent down via datatables
	CNetworkArray( int, m_iPing, MAX_PLAYERS+1 );
	CNetworkArray( int, m_iScore, MAX_PLAYERS+1 );
	CNetworkArray( int, m_iDeaths, MAX_PLAYERS+1 );
	CNetworkArray( int, m_bConnected, MAX_PLAYERS+1 );
	CNetworkArray( int, m_iTeam, MAX_PLAYERS+1 );
	CNetworkArray( int, m_bAlive, MAX_PLAYERS+1 );
	CNetworkArray( int, m_iHealth, MAX_PLAYERS+1 );
		
	int	m_nUpdateCounter;

public:
	BEGIN_INIT_SEND_TABLE(CPlayerResource)
	BEGIN_SEND_TABLE(CPlayerResource, DT_PlayerResource, DT_BaseEntity)
		//	SendPropInternalArray( SendPropString( SENDINFO(m_szName[0]) ), SENDARRAYINFO(m_szName) ),
		SendPropArray3(SENDINFO_ARRAY3(m_iPing), SendPropInt(SENDINFO_ARRAY(m_iPing), 10, SPROP_UNSIGNED)),
		//	SendPropInternalArray( SendPropInt( SENDINFO_ARRAY(m_iPacketloss), 7, SPROP_UNSIGNED ), m_iPacketloss ),
		SendPropArray3(SENDINFO_ARRAY3(m_iScore), SendPropInt(SENDINFO_ARRAY(m_iScore), 12)),
		SendPropArray3(SENDINFO_ARRAY3(m_iDeaths), SendPropInt(SENDINFO_ARRAY(m_iDeaths), 12)),
		SendPropArray3(SENDINFO_ARRAY3(m_bConnected), SendPropInt(SENDINFO_ARRAY(m_bConnected), 1, SPROP_UNSIGNED)),
		SendPropArray3(SENDINFO_ARRAY3(m_iTeam), SendPropInt(SENDINFO_ARRAY(m_iTeam), 4)),
		SendPropArray3(SENDINFO_ARRAY3(m_bAlive), SendPropInt(SENDINFO_ARRAY(m_bAlive), 1, SPROP_UNSIGNED)),
		SendPropArray3(SENDINFO_ARRAY3(m_iHealth), SendPropInt(SENDINFO_ARRAY(m_iHealth), -1, SPROP_VARINT | SPROP_UNSIGNED)),
	END_SEND_TABLE(DT_PlayerResource)
	END_INIT_SEND_TABLE()
};

extern CPlayerResource *g_pPlayerResource;

#endif // PLAYER_RESOURCE_H
