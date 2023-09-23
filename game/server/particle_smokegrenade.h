//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef PARTICLE_SMOKEGRENADE_H
#define PARTICLE_SMOKEGRENADE_H


#include "baseparticleentity.h"


#define PARTICLESMOKEGRENADE_ENTITYNAME	"env_particlesmokegrenade"


class ParticleSmokeGrenade : public CBaseParticleEntity
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( ParticleSmokeGrenade, CBaseParticleEntity );
	DECLARE_SERVERCLASS();

						ParticleSmokeGrenade();

	virtual int			UpdateTransmitState( void );

public:

	// Tell the client entity to start filling the volume.
	void				FillVolume();

	// Set the times it fades out at.
	void				SetFadeTime(float startTime, float endTime);

	// Set time to fade out relative to current time
	void				SetRelativeFadeTime(float startTime, float endTime);


public:
	
	// Stage 0 (default): make a smoke trail that follows the entity it's following.
	// Stage 1          : fill a volume with smoke.
	CNetworkVar( unsigned char, m_CurrentStage );

	CNetworkVar( float, m_flSpawnTime );

	// When to fade in and out.
	CNetworkVar( float, m_FadeStartTime );
	CNetworkVar( float, m_FadeEndTime );

	BEGIN_INIT_SEND_TABLE(ParticleSmokeGrenade)
	BEGIN_SEND_TABLE(ParticleSmokeGrenade, DT_ParticleSmokeGrenade, DT_BaseParticleEntity)
		SendPropTime(SENDINFO(m_flSpawnTime)),
		SendPropFloat(SENDINFO(m_FadeStartTime), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_FadeEndTime), 0, SPROP_NOSCALE),
		SendPropInt(SENDINFO(m_CurrentStage), 1, SPROP_UNSIGNED),
	END_SEND_TABLE(DT_ParticleSmokeGrenade)
	END_INIT_SEND_TABLE()
};


#endif


