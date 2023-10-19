//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "basetempentity.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Dispatches sparks
//-----------------------------------------------------------------------------
class CTEMetalSparks : public CBaseTempEntity
{
DECLARE_CLASS( CTEMetalSparks, CBaseTempEntity );

public:
					CTEMetalSparks() {};
					CTEMetalSparks( const char *name );
	virtual			~CTEMetalSparks( void );

	virtual void	Test( const Vector& current_origin, const QAngle& current_angles );

	DECLARE_SERVERCLASS();

public:
	CNetworkVector( m_vecPos );
	CNetworkVector( m_vecDir );

	BEGIN_INIT_SEND_TABLE(CTEMetalSparks)
	BEGIN_SEND_TABLE(CTEMetalSparks, DT_TEMetalSparks, DT_BaseTempEntity)
		SendPropVector(SENDINFO(m_vecPos), -1, SPROP_COORD),
		SendPropVector(SENDINFO(m_vecDir), -1, SPROP_COORD),
	END_SEND_TABLE(DT_TEMetalSparks)
	END_INIT_SEND_TABLE()
};

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *name - 
//-----------------------------------------------------------------------------
CTEMetalSparks::CTEMetalSparks( const char *name ) :
	CBaseTempEntity( name )
{
	m_vecPos.Init();
	m_vecDir.Init();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTEMetalSparks::~CTEMetalSparks( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *current_origin - 
//			*current_angles - 
//-----------------------------------------------------------------------------
void CTEMetalSparks::Test( const Vector& current_origin, const QAngle& current_angles )
{
	// Fill in data
	m_vecPos = current_origin;
	
	AngleVectors( current_angles, &m_vecDir.GetForModify() );
	
	Vector forward;

	m_vecPos += Vector( 0, 0, 24 );

	AngleVectors( current_angles, &forward );
	forward[2] = 0.0;
	VectorNormalize( forward );

	VectorMA( m_vecPos, 100.0, forward, m_vecPos.GetForModify() );
	
	CBroadcastRecipientFilter filter;
	Create( filter, 0.0 );
}

IMPLEMENT_SERVERCLASS(CTEMetalSparks, DT_TEMetalSparks)
LINK_ENTITY_TO_CLASS(TEMetalSparks, CTEMetalSparks);
PRECACHE_REGISTER(TEMetalSparks);

// Singleton to fire TEMetalSparks objects
static CTEMetalSparks g_TEMetalSparks( "Metal Sparks" );

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : msg_dest - 
//			delay - 
//			*origin - 
//			*recipient - 
//			*pos - 
//			scale - 
//-----------------------------------------------------------------------------
void TE_MetalSparks( IRecipientFilter& filter, float delay,
	const Vector* pos, const Vector* dir )
{
	g_TEMetalSparks.m_vecPos = *pos;
	g_TEMetalSparks.m_vecDir = *dir;

	Assert( dir->Length() < 1.01 ); // make sure it's a normal

	// Send it over the wire
	g_TEMetalSparks.Create( filter, delay );
}

class CTEArmorRicochet : public CTEMetalSparks
{
DECLARE_CLASS( CTEArmorRicochet, CTEMetalSparks );

public:
	CTEArmorRicochet() {};
	CTEArmorRicochet( const char *name ) : CTEMetalSparks(name) {}
	DECLARE_SERVERCLASS();
	DECLARE_SEND_TABLE_ACCESS(DT_TEArmorRicochet);

	BEGIN_INIT_SEND_TABLE(CTEArmorRicochet)
	BEGIN_SEND_TABLE(CTEArmorRicochet, DT_TEArmorRicochet, DT_TEMetalSparks)

	END_SEND_TABLE(DT_TEArmorRicochet)
	END_INIT_SEND_TABLE()
};

IMPLEMENT_SERVERCLASS(CTEArmorRicochet, DT_TEArmorRicochet, DT_TEMetalSparks)
LINK_ENTITY_TO_CLASS(TEArmorRicochet, CTEArmorRicochet);
PRECACHE_REGISTER(TEArmorRicochet);

static CTEArmorRicochet g_TEArmorRicochet( "Armor Ricochet" );
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : msg_dest - 
//			delay - 
//			*origin - 
//			*recipient - 
//			*pos - 
//			scale - 
//-----------------------------------------------------------------------------
void TE_ArmorRicochet( IRecipientFilter& filter, float delay,
	const Vector* pos, const Vector* dir )
{
	g_TEArmorRicochet.m_vecPos = *pos;
	g_TEArmorRicochet.m_vecDir = *dir;

	// Send it over the wire
	g_TEArmorRicochet.Create( filter, delay );
}
