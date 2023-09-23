//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side C_CSTeam class
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "engine/IEngineSound.h"
#include "hud.h"
#include "recvproxy.h"
#include "c_cs_team.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_BuyZone : public C_BaseEntity {
public:
	DECLARE_CLIENTCLASS();
	C_BuyZone() {}

public:
	BEGIN_INIT_RECV_TABLE(C_BuyZone)
	BEGIN_RECV_TABLE(C_BuyZone, DT_BuyZone, DT_BaseEntity)

	END_RECV_TABLE(DT_BuyZone)
	END_INIT_RECV_TABLE()
};

IMPLEMENT_CLIENTCLASS(C_BuyZone, DT_BuyZone, CBuyZone)

IMPLEMENT_CLIENTCLASS(C_CSTeam, DT_CSTeam, CCSTeam)


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_CSTeam::C_CSTeam()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_CSTeam::~C_CSTeam()
{
}

