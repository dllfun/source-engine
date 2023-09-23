//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef PARTICLE_FIRE_H
#define PARTICLE_FIRE_H


#include "baseparticleentity.h"


class CParticleFire : public CBaseParticleEntity
{
	DECLARE_DATADESC();

public:
	CParticleFire();

	DECLARE_CLASS( CParticleFire, CBaseParticleEntity );

					DECLARE_SERVERCLASS();

	// The client shoots a ray out and starts creating fire where it hits.
	CNetworkVector( m_vOrigin );
	CNetworkVector( m_vDirection );

	BEGIN_INIT_SEND_TABLE(CParticleFire)
	BEGIN_SEND_TABLE_NOBASE(CParticleFire, DT_ParticleFire)
		SendPropVector(SENDINFO(m_vOrigin), 0, SPROP_COORD),
		SendPropVector(SENDINFO(m_vDirection), 0, SPROP_NOSCALE)
	END_SEND_TABLE(DT_ParticleFire)
	END_INIT_SEND_TABLE()
};


#endif



