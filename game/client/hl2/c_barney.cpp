//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_Barney : public C_AI_BaseNPC
{
public:
	DECLARE_CLASS( C_Barney, C_AI_BaseNPC );
	DECLARE_CLIENTCLASS();

					C_Barney();
	virtual			~C_Barney();

private:
	C_Barney( const C_Barney & ); // not defined, not accessible

public:
	BEGIN_INIT_RECV_TABLE(C_Barney)
	BEGIN_RECV_TABLE(C_Barney, DT_NPC_Barney, DT_AI_BaseNPC)
	END_RECV_TABLE()
	END_INIT_RECV_TABLE()
};

IMPLEMENT_CLIENTCLASS(C_Barney, DT_NPC_Barney, CNPC_Barney)


C_Barney::C_Barney()
{
}


C_Barney::~C_Barney()
{
}


