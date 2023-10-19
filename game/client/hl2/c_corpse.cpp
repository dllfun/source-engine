//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements C_Corpse
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_corpse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS(C_Corpse, DT_Corpse, CCorpse)





C_Corpse::C_Corpse()
{
	m_nReferencePlayer = 0;
}


int C_Corpse::DrawModel(IVModel* pWorld, int flags )
{
	int drawn = 0;
	if ( m_nReferencePlayer <= 0 || 
		 m_nReferencePlayer > gpGlobals->GetMaxClients() )
	{
		return drawn;
	};

	// Make sure m_pstudiohdr is valid for drawing
	if ( !GetModelPtr() )
	{
		return drawn;
	}

	if ( !m_bReadyToDraw )
		return 0;

	// get copy of player
	C_BasePlayer *player = dynamic_cast< C_BasePlayer *>( cl_entitylist->GetEnt( m_nReferencePlayer ) );
	if ( player )
	{
		Vector zero;
		zero.Init();

		drawn = modelrender->DrawModel( 
			pWorld,
			flags, 
			this,
			MODEL_INSTANCE_INVALID,
			m_nReferencePlayer, 
			GetModel(),
			GetAbsOrigin(),
			GetAbsAngles(),
			m_nSkin,
			m_nBody,
			m_nHitboxSet );
	}

	return drawn;
}

