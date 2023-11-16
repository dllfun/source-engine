//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: CS's custom C_PlayerResource
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_CS_PLAYERRESOURCE_H
#define C_CS_PLAYERRESOURCE_H
#ifdef _WIN32
#pragma once
#endif

#include "cs_shareddefs.h"
#include "c_playerresource.h"

struct Clan {
public:
	Clan() {}
	char buf[MAX_CLAN_TAG_LENGTH];
	const char* ToCStr() {
		return buf;
	}
	char& operator[](int i) {
		return buf[i];
	}
};

inline void NetworkVarConstruct(Clan& x) {  }

class C_CS_PlayerResource : public C_PlayerResource
{
	DECLARE_CLASS( C_CS_PlayerResource, C_PlayerResource );
public:
	DECLARE_CLIENTCLASS();

					C_CS_PlayerResource();
	virtual			~C_CS_PlayerResource();

	bool			IsVIP(int iIndex );
	bool			HasC4(int iIndex );
	bool			IsHostageAlive(int iIndex);
	bool			IsHostageFollowingSomeone(int iIndex);
	const Vector	GetHostagePosition( int index );
	int				GetHostageEntityID(int iIndex);
	const Vector	GetC4Postion();
	const Vector	GetBombsiteAPosition();
	const Vector	GetBombsiteBPosition();
	const Vector	GetHostageRescuePosition( int index );
	int				GetPlayerClass( int iIndex );

	bool			IsBombSpotted( void ) const;
	bool			IsPlayerSpotted( int iIndex );

	const char		*GetClanTag( int index );

	int				GetNumMVPs( int iIndex );
	bool			HasDefuser( int iIndex );

protected:

	CNetworkVar( int,		m_iPlayerC4);	// entity index of C4 carrier or 0
	CNetworkVar( int,		m_iPlayerVIP);	// entity index of VIP player or 0
	CNetworkVector(	m_vecC4);		// position of C4
	CNetworkVector(	m_bombsiteCenterA);	
	CNetworkVector(	m_bombsiteCenterB);	

	CNetworkArray( bool,	m_bHostageAlive,MAX_HOSTAGES);
	CNetworkArray( bool,	m_isHostageFollowingSomeone,MAX_HOSTAGES);
	CNetworkArray( int,		m_iHostageEntityIDs,MAX_HOSTAGES);
	CNetworkArray( int,		m_iHostageX,MAX_HOSTAGES);
	CNetworkArray( int,		m_iHostageY,MAX_HOSTAGES);
	CNetworkArray( int,		m_iHostageZ,MAX_HOSTAGES);

	CNetworkArray( int,		m_hostageRescueX,MAX_HOSTAGE_RESCUES);
	CNetworkArray( int,		m_hostageRescueY,MAX_HOSTAGE_RESCUES);
	CNetworkArray( int,		m_hostageRescueZ,MAX_HOSTAGE_RESCUES);

	CNetworkVar( bool,	m_bBombSpotted);
	CNetworkArray( bool,	m_bPlayerSpotted, MAX_PLAYERS + 1 );
	int		m_iPlayerClasses[ MAX_PLAYERS + 1 ];

	CNetworkArray(Clan,	m_szClan,MAX_PLAYERS+1);

	CNetworkArray( int,		m_iMVPs, MAX_PLAYERS + 1 );	 
	CNetworkArray( bool,	m_bHasDefuser, MAX_PLAYERS + 1 );

public:
	BEGIN_INIT_RECV_TABLE(C_CS_PlayerResource)
	BEGIN_RECV_TABLE(C_CS_PlayerResource, DT_CSPlayerResource, DT_PlayerResource)
		RecvPropInt(RECVINFO(m_iPlayerC4)),
		RecvPropInt(RECVINFO(m_iPlayerVIP)),
		RecvPropVector(RECVINFO(m_vecC4)),
		RecvPropArray3(RECVINFO_ARRAY(m_bHostageAlive), RecvPropInt(RECVINFO_ARRAY3(m_bHostageAlive))),
		RecvPropArray3(RECVINFO_ARRAY(m_isHostageFollowingSomeone), RecvPropInt(RECVINFO_ARRAY3(m_isHostageFollowingSomeone))),
		RecvPropArray3(RECVINFO_ARRAY(m_iHostageEntityIDs), RecvPropInt(RECVINFO_ARRAY3(m_iHostageEntityIDs))),
		RecvPropArray3(RECVINFO_ARRAY(m_iHostageX), RecvPropInt(RECVINFO_ARRAY3(m_iHostageX))),
		RecvPropArray3(RECVINFO_ARRAY(m_iHostageY), RecvPropInt(RECVINFO_ARRAY3(m_iHostageY))),
		RecvPropArray3(RECVINFO_ARRAY(m_iHostageZ), RecvPropInt(RECVINFO_ARRAY3(m_iHostageZ))),
		RecvPropVector(RECVINFO(m_bombsiteCenterA)),
		RecvPropVector(RECVINFO(m_bombsiteCenterB)),
		RecvPropArray3(RECVINFO_ARRAY(m_hostageRescueX), RecvPropInt(RECVINFO_ARRAY3(m_hostageRescueX))),
		RecvPropArray3(RECVINFO_ARRAY(m_hostageRescueY), RecvPropInt(RECVINFO_ARRAY3(m_hostageRescueY))),
		RecvPropArray3(RECVINFO_ARRAY(m_hostageRescueZ), RecvPropInt(RECVINFO_ARRAY3(m_hostageRescueZ))),
		RecvPropInt(RECVINFO(m_bBombSpotted)),
		RecvPropArray3(RECVINFO_ARRAY(m_bPlayerSpotted), RecvPropInt(RECVINFO_ARRAY3(m_bPlayerSpotted))),
		RecvPropArray3(RECVINFO_ARRAY(m_iMVPs), RecvPropInt(RECVINFO_ARRAY3(m_iMVPs))),
		RecvPropArray3(RECVINFO_ARRAY(m_bHasDefuser), RecvPropInt(RECVINFO_ARRAY3(m_bHasDefuser))),
		RecvPropArray3(RECVINFO_ARRAY(m_szClan), RecvPropString(RECVINFO_ARRAY3(m_szClan))),
	END_RECV_TABLE(DT_CSPlayerResource)
	END_INIT_RECV_TABLE()
};


#endif // C_CS_PLAYERRESOURCE_H
