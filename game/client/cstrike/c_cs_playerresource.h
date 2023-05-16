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

	int		m_iPlayerC4;	// entity index of C4 carrier or 0
	int		m_iPlayerVIP;	// entity index of VIP player or 0
	Vector	m_vecC4;		// position of C4
	Vector	m_bombsiteCenterA;	
	Vector	m_bombsiteCenterB;	

	bool	m_bHostageAlive[MAX_HOSTAGES];
	bool	m_isHostageFollowingSomeone[MAX_HOSTAGES];
	int		m_iHostageEntityIDs[MAX_HOSTAGES];
	int		m_iHostageX[MAX_HOSTAGES];
	int		m_iHostageY[MAX_HOSTAGES];
	int		m_iHostageZ[MAX_HOSTAGES];

	int		m_hostageRescueX[MAX_HOSTAGE_RESCUES];
	int		m_hostageRescueY[MAX_HOSTAGE_RESCUES];
	int		m_hostageRescueZ[MAX_HOSTAGE_RESCUES];

	bool	m_bBombSpotted;
	bool	m_bPlayerSpotted[ MAX_PLAYERS + 1 ];
	int		m_iPlayerClasses[ MAX_PLAYERS + 1 ];

	char	m_szClan[MAX_PLAYERS+1][MAX_CLAN_TAG_LENGTH];

	int		m_iMVPs[ MAX_PLAYERS + 1 ];	 
	bool	m_bHasDefuser[ MAX_PLAYERS + 1 ];

	BEGIN_RECV_TABLE(C_CS_PlayerResource, DT_CSPlayerResource, DT_PlayerResource)
		RecvPropInt(RECVINFO(m_iPlayerC4)),
		RecvPropInt(RECVINFO(m_iPlayerVIP)),
		RecvPropVector(RECVINFO(m_vecC4)),
		RecvPropArray3(RECVINFO_ARRAY(m_bHostageAlive), RecvPropInt(RECVINFO(m_bHostageAlive[0]))),
		RecvPropArray3(RECVINFO_ARRAY(m_isHostageFollowingSomeone), RecvPropInt(RECVINFO(m_isHostageFollowingSomeone[0]))),
		RecvPropArray3(RECVINFO_ARRAY(m_iHostageEntityIDs), RecvPropInt(RECVINFO(m_iHostageEntityIDs[0]))),
		RecvPropArray3(RECVINFO_ARRAY(m_iHostageX), RecvPropInt(RECVINFO(m_iHostageX[0]))),
		RecvPropArray3(RECVINFO_ARRAY(m_iHostageY), RecvPropInt(RECVINFO(m_iHostageY[0]))),
		RecvPropArray3(RECVINFO_ARRAY(m_iHostageZ), RecvPropInt(RECVINFO(m_iHostageZ[0]))),
		RecvPropVector(RECVINFO(m_bombsiteCenterA)),
		RecvPropVector(RECVINFO(m_bombsiteCenterB)),
		RecvPropArray3(RECVINFO_ARRAY(m_hostageRescueX), RecvPropInt(RECVINFO(m_hostageRescueX[0]))),
		RecvPropArray3(RECVINFO_ARRAY(m_hostageRescueY), RecvPropInt(RECVINFO(m_hostageRescueY[0]))),
		RecvPropArray3(RECVINFO_ARRAY(m_hostageRescueZ), RecvPropInt(RECVINFO(m_hostageRescueZ[0]))),
		RecvPropInt(RECVINFO(m_bBombSpotted)),
		RecvPropArray3(RECVINFO_ARRAY(m_bPlayerSpotted), RecvPropInt(RECVINFO(m_bPlayerSpotted[0]))),
		RecvPropArray3(RECVINFO_ARRAY(m_iMVPs), RecvPropInt(RECVINFO(m_iMVPs[0]))),
		RecvPropArray3(RECVINFO_ARRAY(m_bHasDefuser), RecvPropInt(RECVINFO(m_bHasDefuser[0]))),
		RecvPropArray3(RECVINFO_ARRAY(m_szClan), RecvPropString(RECVINFO(m_szClan[0]))),
	END_RECV_TABLE(DT_CSPlayerResource)
};


#endif // C_CS_PLAYERRESOURCE_H
