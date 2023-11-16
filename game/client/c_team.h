//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side CTeam class
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_TEAM_H
#define C_TEAM_H
#ifdef _WIN32
#pragma once
#endif

#include "shareddefs.h"
#include "utlvector.h"
#include "client_thinklist.h"


class C_BasePlayer;
class C_Team;

template<typename T= C_Team>
void RecvProxy_PlayerList(const CRecvProxyData* pData, void* pStruct, void* pOut);

class RecvPropPlayerList : public RecvPropInt {
public:
	RecvPropPlayerList() {}

	template<typename T = int>
	RecvPropPlayerList(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,	// Handled by RECVINFO macro, but set to SIZEOF_IGNORE if you don't want to bother.
		int flags = 0,
		RecvVarProxyFn varProxy = RecvProxy_PlayerList<T>
	);
	virtual	~RecvPropPlayerList() {}
	RecvPropPlayerList& operator=(const RecvPropPlayerList& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropPlayerList* pRecvProp = new RecvPropPlayerList;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

template<typename T>
RecvPropPlayerList::RecvPropPlayerList(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar,
	int flags,
	RecvVarProxyFn varProxy
):RecvPropInt((int*)0, pVarName, offset, sizeofVar, flags, varProxy)
{
	
}

void RecvProxyArrayLength_PlayerArray(void* pStruct, int objectID, int currentArrayLength);

class C_Team : public C_BaseEntity
{
	DECLARE_CLASS( C_Team, C_BaseEntity );
public:
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

					C_Team();
	virtual			~C_Team();

	virtual void	PreDataUpdate( DataUpdateType_t updateType );

	// Data Handling
	virtual char	*Get_Name( void );
	virtual int		Get_Score( void );
	virtual int		Get_Deaths( void );
	virtual int		Get_Ping( void );

	// Player Handling
	virtual int		Get_Number_Players( void );
	virtual bool	ContainsPlayer( int iPlayerIndex );
	C_BasePlayer*	GetPlayer( int idx );

	// for shared code, use the same function name
	virtual int		GetNumPlayers( void ) { return Get_Number_Players(); }

	int		GetTeamNumber() const;

	int		GetRoundsWon(void) { return m_iRoundsWon; }

	void	RemoveAllPlayers();


// IClientThinkable overrides.
public:

	virtual	void				ClientThink();


public:

	// Data received from the server
	CUtlVector< int > m_aPlayers;
	CNetworkString( 	m_szTeamname, MAX_TEAM_NAME_LENGTH );
	CNetworkVar( int,		m_iScore);
	CNetworkVar( int,		m_iRoundsWon);

	// Data for the scoreboard
	int		m_iDeaths;
	int		m_iPing;
	int		m_iPacketloss;
	CNetworkVar( int,		m_iTeamNum);

public:
	BEGIN_INIT_RECV_TABLE(C_Team)
	BEGIN_RECV_TABLE(C_Team, DT_Team, DT_BaseEntity)
		RecvPropInt(RECVINFO(m_iTeamNum)),
		RecvPropInt(RECVINFO(m_iScore)),
		RecvPropInt(RECVINFO(m_iRoundsWon)),
		RecvPropString(RECVINFO(m_szTeamname)),

		RecvPropInternalArray(
			MAX_PLAYERS,
			0,
			"player_array",
			RecvPropPlayerList((C_Team*)0, "player_array_element", 0, SIZEOF_IGNORE, 0),//, RecvProxy_PlayerList
			RecvProxyArrayLength_PlayerArray
			)
	END_RECV_TABLE(DT_Team)
	END_INIT_RECV_TABLE()
};


//-----------------------------------------------------------------------------
// Purpose: RecvProxy that converts the Team's player UtlVector to entindexes
//-----------------------------------------------------------------------------
template<typename T>
void RecvProxy_PlayerList(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	T* pTeam = (T*)pOut;//C_Team
	pTeam->m_aPlayers[pData->m_iElement] = pData->m_Value.m_Int;
}

// Global list of client side team entities
extern CUtlVector< C_Team * > g_Teams;

// Global team handling functions
C_Team *GetLocalTeam( void );
C_Team *GetGlobalTeam( int iTeamNumber );
C_Team *GetPlayersTeam( int iPlayerIndex );
C_Team *GetPlayersTeam( C_BasePlayer *pPlayer );
bool ArePlayersOnSameTeam( int iPlayerIndex1, int iPlayerIndex2 );
extern int GetNumberOfTeams( void );

#endif // C_TEAM_H
