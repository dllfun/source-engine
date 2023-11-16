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
#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern IPhysicsSurfaceProps *physprops;

class C_TEImpact : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEImpact, C_BaseTempEntity );
	
	DECLARE_CLIENTCLASS();

	C_TEImpact( void );
	virtual	~C_TEImpact( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );
	virtual void	Precache( void );

	virtual void	PlayImpactSound( trace_t &tr );
	virtual void	PerformCustomEffects( trace_t &tr, Vector &shotDir );
public:
	CNetworkVector(			m_vecOrigin);
	CNetworkVector(			m_vecNormal);
	CNetworkVar( int,				m_iType);
	CNetworkVar( byte,			m_ucFlags);

public:
	BEGIN_INIT_RECV_TABLE(C_TEImpact)
	BEGIN_RECV_TABLE(C_TEImpact, DT_TEImpact, DT_BaseTempEntity)
		RecvPropVector(RECVINFO(m_vecOrigin)),
		RecvPropVector(RECVINFO(m_vecNormal)),
		RecvPropInt(RECVINFO(m_iType)),
		RecvPropInt(RECVINFO(m_ucFlags)),
	END_RECV_TABLE(DT_TEImpact)
	END_INIT_RECV_TABLE()
};

//-----------------------------------------------------------------------------
// Purpose: 
// Output : 
//-----------------------------------------------------------------------------
C_TEImpact::C_TEImpact( void )
{
	m_vecOrigin.Init();
	m_vecNormal.Init();
	
	m_iType		= -1;
	m_ucFlags	= 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : 
//-----------------------------------------------------------------------------
C_TEImpact::~C_TEImpact( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TEImpact::Precache( void )
{
	//TODO: Precache all materials/sounds used by impacts here
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : unused - 
//-----------------------------------------------------------------------------
void C_TEImpact::PostDataUpdate( DataUpdateType_t updateType )
{
	VPROF( "C_TEImpact::PostDataUpdate" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TEImpact::PlayImpactSound( trace_t &tr )
{
}

//-----------------------------------------------------------------------------
// Purpose: Perform custom effects based on the Decal index
//-----------------------------------------------------------------------------
void C_TEImpact::PerformCustomEffects( trace_t &tr, Vector &shotDir )
{
}

//Receive data table
IMPLEMENT_CLIENTCLASS_EVENT(C_TEImpact, DT_TEImpact, CTEImpact)
LINK_ENTITY_TO_CLASS(TEImpact, C_TEImpact);
PRECACHE_REGISTER(TEImpact);