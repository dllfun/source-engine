//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef PARTICLE_LIGHT_H
#define PARTICLE_LIGHT_H
#ifdef _WIN32
#pragma once
#endif


#include "baseentity.h"


//==================================================
// CParticleLight. These are tied to 
//==================================================

#define PARTICLELIGHT_ENTNAME	"env_particlelight"

class CParticleLight : public CServerOnlyPointEntity
{
public:
	DECLARE_CLASS( CParticleLight, CServerOnlyPointEntity );
	DECLARE_DATADESC();

					CParticleLight();
	DECLARE_SERVERCLASS();

public:
	float			m_flIntensity;
	Vector			m_vColor;	// 0-255
	string_t		m_PSName;	// Name of the particle system entity this light affects.
	bool			m_bDirectional;

	BEGIN_SEND_TABLE(CParticleLight, DT_ParticleLight, DT_BaseEntity)

	END_SEND_TABLE(DT_ParticleLight)
};


#endif // PARTICLE_LIGHT_H
