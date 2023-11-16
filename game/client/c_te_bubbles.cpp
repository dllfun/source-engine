//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_basetempentity.h"
#include "c_te_legacytempents.h"
#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Bubbles TE
//-----------------------------------------------------------------------------
class C_TEBubbles : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEBubbles, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

					C_TEBubbles( void );
	virtual			~C_TEBubbles( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

public:
	CNetworkVector(			m_vecMins);
	CNetworkVector(			m_vecMaxs);
	CNetworkVar( float,			m_fHeight);
	CNetworkVar( int,				m_nModelIndex);
	CNetworkVar( int,				m_nCount);
	CNetworkVar( float,			m_fSpeed);

public:
	BEGIN_INIT_RECV_TABLE(C_TEBubbles)
	BEGIN_RECV_TABLE(C_TEBubbles, DT_TEBubbles, DT_BaseTempEntity)
		RecvPropVector(RECVINFO(m_vecMins)),
		RecvPropVector(RECVINFO(m_vecMaxs)),
		RecvPropInt(RECVINFO(m_nModelIndex)),
		RecvPropFloat(RECVINFO(m_fHeight)),
		RecvPropInt(RECVINFO(m_nCount)),
		RecvPropFloat(RECVINFO(m_fSpeed)),
	END_RECV_TABLE(DT_TEBubbles)
	END_INIT_RECV_TABLE()
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEBubbles::C_TEBubbles( void )
{
	m_vecMins.Init();
	m_vecMaxs.Init();
	m_fHeight = 0.0;
	m_nModelIndex = 0;
	m_nCount = 0;
	m_fSpeed = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEBubbles::~C_TEBubbles( void )
{
}

void TE_Bubbles( IRecipientFilter& filter, float delay,
	const Vector* mins, const Vector* maxs, float height, int modelindex, int count, float speed )
{
	tempents->Bubbles( *mins, *maxs, height, modelindex, count, speed );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_TEBubbles::PostDataUpdate( DataUpdateType_t updateType )
{
	VPROF( "C_TEBubbles::PostDataUpdate" );

	tempents->Bubbles( m_vecMins, m_vecMaxs, m_fHeight, m_nModelIndex, m_nCount, m_fSpeed );
}

IMPLEMENT_CLIENTCLASS_EVENT(C_TEBubbles, DT_TEBubbles, CTEBubbles)
LINK_ENTITY_TO_CLASS(TEBubbles, C_TEBubbles);
PRECACHE_REGISTER(TEBubbles);
