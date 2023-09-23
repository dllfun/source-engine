//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side CTFTeam class
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_CS_TEAM_H
#define C_CS_TEAM_H
#ifdef _WIN32
#pragma once
#endif

#include "c_team.h"
#include "shareddefs.h"

class C_BaseEntity;
class C_BaseObject;
class CBaseTechnology;

//-----------------------------------------------------------------------------
// Purpose: TF's Team manager
//-----------------------------------------------------------------------------
class C_CSTeam : public C_Team
{
	DECLARE_CLASS( C_CSTeam, C_Team );
public:
	DECLARE_CLIENTCLASS();

					C_CSTeam();
	virtual			~C_CSTeam();

public:
	BEGIN_INIT_RECV_TABLE(C_CSTeam)
	BEGIN_RECV_TABLE(C_CSTeam, DT_CSTeam, DT_Team)

	END_RECV_TABLE(DT_CSTeam)
	END_INIT_RECV_TABLE()
};


#endif // C_CS_TEAM_H
