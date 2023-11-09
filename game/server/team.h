//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//

#ifndef TEAM_H
#define TEAM_H
#ifdef _WIN32
#pragma once
#endif

#include "shareddefs.h"
#include "utlvector.h"
#include "baseentity.h"

class CBasePlayer;
class CTeamSpawnPoint;

template<typename T>
void SendProxy_PlayerList(const SendProp* pProp, const void* pStruct, const void* pData, DVariant* pOut, int iElement, int objectID,int aaa);
class SendPropPlayerList : public SendPropInt {
public:
	SendPropPlayerList() {}

	template<typename T>
	SendPropPlayerList(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,	// Handled by SENDINFO macro.
		int nBits = -1,					// Set to -1 to automatically pick (max) number of bits based on size of element.
		int flags = 0,
		SendVarProxyFn varProxy = SendProxy_PlayerList<T>
	);
	virtual ~SendPropPlayerList() {}
	SendPropPlayerList& operator=(const SendPropPlayerList& srcSendProp) {
		SendProp::operator=(srcSendProp);
		return *this;
	}
	operator SendProp* () {
		SendPropPlayerList* pSendProp = new SendPropPlayerList;
		*pSendProp = *this;
		return pSendProp;
	}
};

template<typename T>
SendPropPlayerList::SendPropPlayerList(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar,	// Handled by SENDINFO macro.
	int nBits,					// Set to -1 to automatically pick (max) number of bits based on size of element.
	int flags,
	SendVarProxyFn varProxy
):SendPropInt((int*)0, pVarName, offset, sizeofVar, nBits, flags, varProxy)
{

}

int SendProxyArrayLength_PlayerArray(const void* pStruct, int objectID);

class CTeam : public CBaseEntity
{
	DECLARE_CLASS( CTeam, CBaseEntity );
public:
	CTeam( void );
	virtual ~CTeam( void );

	DECLARE_SERVERCLASS();

	virtual void Precache( void ) { return; };

	virtual void Think( void );
	virtual int  UpdateTransmitState( void );

	//-----------------------------------------------------------------------------
	// Initialization
	//-----------------------------------------------------------------------------
	virtual void		Init( const char *pName, int iNumber );

	//-----------------------------------------------------------------------------
	// Data Handling
	//-----------------------------------------------------------------------------
	virtual int			GetTeamNumber( void ) const;
	virtual const char *GetName( void );
	virtual void		UpdateClientData( CBasePlayer *pPlayer );
	virtual bool		ShouldTransmitToPlayer( CBasePlayer* pRecipient, CBaseEntity* pEntity );

	//-----------------------------------------------------------------------------
	// Spawnpoints
	//-----------------------------------------------------------------------------
	virtual void InitializeSpawnpoints( void );
	virtual void AddSpawnpoint( CTeamSpawnPoint *pSpawnpoint );
	virtual void RemoveSpawnpoint( CTeamSpawnPoint *pSpawnpoint );
	virtual CBaseEntity *SpawnPlayer( CBasePlayer *pPlayer );

	//-----------------------------------------------------------------------------
	// Players
	//-----------------------------------------------------------------------------
	virtual void InitializePlayers( void );
	virtual void AddPlayer( CBasePlayer *pPlayer );
	virtual void RemovePlayer( CBasePlayer *pPlayer );
	virtual int  GetNumPlayers( void );
	virtual CBasePlayer *GetPlayer( int iIndex );

	//-----------------------------------------------------------------------------
	// Scoring
	//-----------------------------------------------------------------------------
	virtual void AddScore( int iScore );
	virtual void SetScore( int iScore );
	virtual int  GetScore( void );
	virtual void ResetScores( void );

	// Round scoring
	virtual int GetRoundsWon( void ) { return m_iRoundsWon; }
	virtual void SetRoundsWon( int iRounds ) { m_iRoundsWon = iRounds; }
	virtual void IncrementRoundsWon( void ) { m_iRoundsWon++; }

	void AwardAchievement( int iAchievement );

	virtual int GetAliveMembers( void );

public:
	CUtlVector< CTeamSpawnPoint * > m_aSpawnPoints;
	CUtlVector< CBasePlayer * >		m_aPlayers;

	// Data
	CNetworkString( m_szTeamname, MAX_TEAM_NAME_LENGTH );
	CNetworkVar( int, m_iScore );
	CNetworkVar( int, m_iRoundsWon );
	int		m_iDeaths;

	// Spawnpoints
	int		m_iLastSpawn;		// Index of the last spawnpoint used

	CNetworkVar( int, m_iTeamNum );			// Which team is this?

	BEGIN_INIT_SEND_TABLE(CTeam)
	BEGIN_SEND_TABLE(CTeam, DT_Team, DT_BaseEntity)
		SendPropInt(SENDINFO(m_iTeamNum), 5),
		SendPropInt(SENDINFO(m_iScore), 0),
		SendPropInt(SENDINFO(m_iRoundsWon), 8),
		SendPropString(SENDINFO(m_szTeamname)),

		SendPropInternalArray(
			MAX_PLAYERS,
			0,
			"player_array",
			SendPropPlayerList((CTeam*)0, "player_array_element", 0, 4, 10, SPROP_UNSIGNED),//, SendProxy_PlayerList
			SendProxyArrayLength_PlayerArray
			)
	END_SEND_TABLE(DT_Team)
	END_INIT_SEND_TABLE()
};

extern CUtlVector< CTeam * > g_Teams;
extern CTeam *GetGlobalTeam( int iIndex );
extern int GetNumberOfTeams( void );

//-----------------------------------------------------------------------------
// Purpose: SendProxy that converts the Team's player UtlVector to entindexes
//-----------------------------------------------------------------------------
template<typename T>
void SendProxy_PlayerList(const SendProp* pProp, const void* pStruct, const void* pData, DVariant* pOut, int iElement, int objectID,int aaa)
{
	CTeam* pTeam = (T*)pData;//CTeam

	// If this assertion fails, then SendProxyArrayLength_PlayerArray must have failed.
	Assert(iElement < pTeam->m_aPlayers.Size());

	CBasePlayer* pPlayer = pTeam->m_aPlayers[iElement];
	pOut->m_Int = pPlayer->NetworkProp()->entindex();
}

#endif // TEAM_H
