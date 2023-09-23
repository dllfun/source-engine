//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "te_particlesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
// Gauss explosion
//=============================================================================

class CTEGaussExplosion : public CTEParticleSystem
{
public:
	DECLARE_CLASS( CTEGaussExplosion, CTEParticleSystem );
	DECLARE_SERVERCLASS();
					CTEGaussExplosion() {};
					CTEGaussExplosion( const char *name );
	virtual			~CTEGaussExplosion( void );

	virtual void	Test( const Vector& current_origin, const QAngle& current_angles ) { };

	CNetworkVar( int, m_nType );
	CNetworkVector( m_vecDirection );

	BEGIN_INIT_SEND_TABLE(CTEGaussExplosion)
	BEGIN_SEND_TABLE(CTEGaussExplosion, DT_TEGaussExplosion, DT_TEParticleSystem)
		SendPropInt(SENDINFO(m_nType), 2, SPROP_UNSIGNED),
		SendPropVector(SENDINFO(m_vecDirection), -1, SPROP_COORD),
	END_SEND_TABLE(DT_TEGaussExplosion)
	END_INIT_SEND_TABLE()
};


CTEGaussExplosion::CTEGaussExplosion( const char *name ) : BaseClass( name )
{
	m_nType = 0;
	m_vecDirection.Init();
}

CTEGaussExplosion::~CTEGaussExplosion( void )
{
}

IMPLEMENT_SERVERCLASS(CTEGaussExplosion, DT_TEGaussExplosion, DT_TEParticleSystem)
LINK_ENTITY_TO_CLASS(TEGaussExplosion, CTEGaussExplosion);
PRECACHE_REGISTER(TEGaussExplosion);

static CTEGaussExplosion g_TEGaussExplosion( "GaussExplosion" );

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &pos - 
//			&angles - 
//-----------------------------------------------------------------------------
void TE_GaussExplosion( IRecipientFilter& filter, float delay,
	const Vector &pos, const Vector &dir, int type )
{
	g_TEGaussExplosion.m_vecOrigin		= pos;
	g_TEGaussExplosion.m_vecDirection	= dir;
	g_TEGaussExplosion.m_nType			= type;

	//Send it
	g_TEGaussExplosion.Create( filter, delay );
}



