//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "ragdoll_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_RagdollManager : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_RagdollManager, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	C_RagdollManager();

// C_BaseEntity overrides.
public:

	virtual void	OnDataChanged( DataUpdateType_t updateType );

public:

	CNetworkVar( int,		m_iCurrentMaxRagdollCount);

public:
	BEGIN_INIT_RECV_TABLE(C_RagdollManager)
	BEGIN_RECV_TABLE(C_RagdollManager, DT_RagdollManager, DT_BaseEntity)
		RecvPropInt(RECVINFO(m_iCurrentMaxRagdollCount)),
	END_RECV_TABLE(DT_RagdollManager)
	END_INIT_RECV_TABLE()
};

IMPLEMENT_CLIENTCLASS( C_RagdollManager, DT_RagdollManager, CRagdollManager )


//-----------------------------------------------------------------------------
// Constructor 
//-----------------------------------------------------------------------------
C_RagdollManager::C_RagdollManager()
{
	m_iCurrentMaxRagdollCount = -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_RagdollManager::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	s_RagdollLRU.SetMaxRagdollCount( m_iCurrentMaxRagdollCount );
}
