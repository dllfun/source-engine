//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//

#ifndef CS_TEAM_H
#define CS_TEAM_H

#ifdef _WIN32
#pragma once
#endif


#include "utlvector.h"
#include "team.h"


//-----------------------------------------------------------------------------
// Purpose: Team Manager
//-----------------------------------------------------------------------------
class CCSTeam : public CTeam
{
	DECLARE_CLASS( CCSTeam, CTeam );
public:
	virtual ~CCSTeam( void );

	DECLARE_SERVERCLASS();
	DECLARE_SEND_TABLE_ACCESS(DT_CSTeam);

	// Initialization
	virtual void Init( const char *pName, int iNumber );
	
	virtual void Precache( void );
	virtual void Think( void );

	//-----------------------------------------------------------------------------
	// Players
	//-----------------------------------------------------------------------------
	virtual void AddPlayer( CBasePlayer *pPlayer );
	virtual void RemovePlayer( CBasePlayer *pPlayer );

	//-----------------------------------------------------------------------------
	// Utility funcs
	//-----------------------------------------------------------------------------
	CCSTeam*		GetEnemyTeam();

private:

	// Used to distribute resources to a team
	float	m_flNextResourceTime;

	int		m_iLastUpdateSentAt;

public:
	BEGIN_INIT_SEND_TABLE(CCSTeam)
	BEGIN_SEND_TABLE(CCSTeam, DT_CSTeam, DT_Team)

	END_SEND_TABLE(DT_CSTeam)
	END_INIT_SEND_TABLE()
};


extern CCSTeam *GetGlobalTFTeam( int iIndex );


#endif // TF_TEAM_H
