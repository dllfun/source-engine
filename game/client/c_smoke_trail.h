//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//
// This defines the client-side SmokeTrail entity. It can also be used without
// an entity, in which case you must pass calls to it and set its position each frame.

#ifndef PARTICLE_SMOKETRAIL_H
#define PARTICLE_SMOKETRAIL_H

#include "particlemgr.h"
#include "particle_prototype.h"
#include "particle_util.h"
#include "particles_simple.h"
#include "c_baseentity.h"
#include "baseparticleentity.h"

#include "fx_trail.h"

//
// Smoke Trail
//

class C_SmokeTrail : public C_BaseParticleEntity, public IPrototypeAppEffect
{
public:
	DECLARE_CLASS( C_SmokeTrail, C_BaseParticleEntity );
	DECLARE_CLIENTCLASS();
	
					C_SmokeTrail();
	virtual			~C_SmokeTrail();

public:

	//For attachments
	void			GetAimEntOrigin( IClientEntity *pAttachedTo, Vector *pAbsOrigin, QAngle *pAbsAngles );

	// Enable/disable emission.
	void			SetEmit(bool bEmit);

	// Change the spawn rate.
	void			SetSpawnRate(float rate);


// C_BaseEntity.
public:
	virtual	void	OnDataChanged(DataUpdateType_t updateType);

	virtual void	CleanupToolRecordingState( KeyValues *msg );

// IPrototypeAppEffect.
public:
	virtual void	Start(CParticleMgr *pParticleMgr, IPrototypeArgAccess *pArgs);

// IParticleEffect.
public:
	virtual void	Update(float fTimeDelta);
	virtual void RenderParticles( CParticleRenderIterator *pIterator );
	virtual void SimulateParticles( CParticleSimulateIterator *pIterator );


public:
	// Effect parameters. These will assume default values but you can change them.
	CNetworkVar( float,			m_SpawnRate);			// How many particles per second.

	CNetworkVector(			m_StartColor);			// Fade between these colors.
	CNetworkVector(			m_EndColor);
	CNetworkVar( float,			m_Opacity);

	CNetworkVar( float,			m_ParticleLifetime);		// How long do the particles live?
	CNetworkVar( float,			m_StopEmitTime);			// When do I stop emitting particles? (-1 = never)
	
	CNetworkVar( float,			m_MinSpeed);				// Speed range.
	CNetworkVar( float,			m_MaxSpeed);
	
	CNetworkVar( float,			m_MinDirectedSpeed);		// Directed speed range.
	CNetworkVar( float,			m_MaxDirectedSpeed);

	CNetworkVar( float,			m_StartSize);			// Size ramp.
	CNetworkVar( float,			m_EndSize);

	CNetworkVar( float,			m_SpawnRadius);

	Vector			m_VelocityOffset;		// Emit the particles in a certain direction.

	CNetworkVar( bool,			m_bEmit);				// Keep emitting particles?

	CNetworkVar( int,				m_nAttachment);

private:
	C_SmokeTrail( const C_SmokeTrail & );

	PMaterialHandle	m_MaterialHandle[2];
	TimedEvent		m_ParticleSpawn;

	CParticleMgr	*m_pParticleMgr;
	CSmartPtr<CSimpleEmitter> m_pSmokeEmitter;

public:
	BEGIN_INIT_RECV_TABLE(C_SmokeTrail)
	BEGIN_RECV_TABLE(C_SmokeTrail, DT_SmokeTrail, DT_BaseParticleEntity)
		RecvPropFloat(RECVINFO(m_SpawnRate)),
		RecvPropVector(RECVINFO(m_StartColor)),
		RecvPropVector(RECVINFO(m_EndColor)),
		RecvPropFloat(RECVINFO(m_ParticleLifetime)),
		RecvPropFloat(RECVINFO(m_StopEmitTime)),
		RecvPropFloat(RECVINFO(m_MinSpeed)),
		RecvPropFloat(RECVINFO(m_MaxSpeed)),
		RecvPropFloat(RECVINFO(m_MinDirectedSpeed)),
		RecvPropFloat(RECVINFO(m_MaxDirectedSpeed)),
		RecvPropFloat(RECVINFO(m_StartSize)),
		RecvPropFloat(RECVINFO(m_EndSize)),
		RecvPropFloat(RECVINFO(m_SpawnRadius)),
		RecvPropInt(RECVINFO(m_bEmit)),
		RecvPropInt(RECVINFO(m_nAttachment)),
		RecvPropFloat(RECVINFO(m_Opacity)),
	END_RECV_TABLE(DT_SmokeTrail)
	END_INIT_RECV_TABLE()
};

//==================================================
// C_RocketTrail
//==================================================

class C_RocketTrail : public C_BaseParticleEntity, public IPrototypeAppEffect
{
public:
	DECLARE_CLASS( C_RocketTrail, C_BaseParticleEntity );
	DECLARE_CLIENTCLASS();
	
					C_RocketTrail();
	virtual			~C_RocketTrail();

public:

	//For attachments
	void			GetAimEntOrigin( IClientEntity *pAttachedTo, Vector *pAbsOrigin, QAngle *pAbsAngles );

	// Enable/disable emission.
	void			SetEmit(bool bEmit);

	// Change the spawn rate.
	void			SetSpawnRate(float rate);


// C_BaseEntity.
public:
	virtual	void	OnDataChanged(DataUpdateType_t updateType);

// IPrototypeAppEffect.
public:
	virtual void	Start(CParticleMgr *pParticleMgr, IPrototypeArgAccess *pArgs);

// IParticleEffect.
public:
	virtual void	Update(float fTimeDelta);
	virtual void RenderParticles( CParticleRenderIterator *pIterator );
	virtual void SimulateParticles( CParticleSimulateIterator *pIterator );


public:
	// Effect parameters. These will assume default values but you can change them.
	CNetworkVar( float,			m_SpawnRate);			// How many particles per second.

	CNetworkVector(			m_StartColor);			// Fade between these colors.
	CNetworkVector(			m_EndColor);
	CNetworkVar( float,			m_Opacity);

	CNetworkVar( float,			m_ParticleLifetime);		// How long do the particles live?
	CNetworkVar( float,			m_StopEmitTime);			// When do I stop emitting particles? (-1 = never)
	
	CNetworkVar( float,			m_MinSpeed);				// Speed range.
	CNetworkVar( float,			m_MaxSpeed);
	
	CNetworkVar( float,			m_StartSize);			// Size ramp.
	CNetworkVar( float,			m_EndSize);

	CNetworkVar( float,			m_SpawnRadius);

	Vector			m_VelocityOffset;		// Emit the particles in a certain direction.

	CNetworkVar( bool,			m_bEmit);				// Keep emitting particles?
	CNetworkVar( bool,			m_bDamaged);				// Has been shot down (should be on fire, etc)

	CNetworkVar( int,				m_nAttachment);

	Vector			m_vecLastPosition;		// Last known position of the rocket
	CNetworkVar( float,			m_flFlareScale);			// Size of the flare

private:
	C_RocketTrail( const C_RocketTrail & );

	PMaterialHandle	m_MaterialHandle[2];
	TimedEvent		m_ParticleSpawn;

	CParticleMgr	*m_pParticleMgr;
	CSmartPtr<CSimpleEmitter> m_pRocketEmitter;

public:
	BEGIN_INIT_RECV_TABLE(C_RocketTrail)
	BEGIN_RECV_TABLE(C_RocketTrail, DT_RocketTrail, DT_BaseParticleEntity)
		RecvPropFloat(RECVINFO(m_SpawnRate)),
		RecvPropVector(RECVINFO(m_StartColor)),
		RecvPropVector(RECVINFO(m_EndColor)),
		RecvPropFloat(RECVINFO(m_ParticleLifetime)),
		RecvPropFloat(RECVINFO(m_StopEmitTime)),
		RecvPropFloat(RECVINFO(m_MinSpeed)),
		RecvPropFloat(RECVINFO(m_MaxSpeed)),
		RecvPropFloat(RECVINFO(m_StartSize)),
		RecvPropFloat(RECVINFO(m_EndSize)),
		RecvPropFloat(RECVINFO(m_SpawnRadius)),
		RecvPropInt(RECVINFO(m_bEmit)),
		RecvPropInt(RECVINFO(m_nAttachment)),
		RecvPropFloat(RECVINFO(m_Opacity)),
		RecvPropInt(RECVINFO(m_bDamaged)),
		RecvPropFloat(RECVINFO(m_flFlareScale)),
	END_RECV_TABLE(DT_RocketTrail)
	END_INIT_RECV_TABLE()
};

class SporeSmokeEffect;


//==================================================
// SporeEffect
//==================================================

class SporeEffect : public CSimpleEmitter
{
public:
							SporeEffect( const char *pDebugName );
	static SporeEffect*		Create( const char *pDebugName );

	virtual void			UpdateVelocity( SimpleParticle *pParticle, float timeDelta );
	virtual Vector			UpdateColor( const SimpleParticle *pParticle );
	virtual float			UpdateAlpha( const SimpleParticle *pParticle );

private:
	SporeEffect( const SporeEffect & );
};

//==================================================
// C_SporeExplosion
//==================================================

class C_SporeExplosion : public C_BaseParticleEntity, public IPrototypeAppEffect
{
public:
	DECLARE_CLASS( C_SporeExplosion, C_BaseParticleEntity );
	DECLARE_CLIENTCLASS();
	
	C_SporeExplosion( void );
	virtual	~C_SporeExplosion( void );

public:

// C_BaseEntity
public:
	virtual	void	OnDataChanged( DataUpdateType_t updateType );

// IPrototypeAppEffect
public:
	virtual void	Start( CParticleMgr *pParticleMgr, IPrototypeArgAccess *pArgs );

// IParticleEffect
public:
	virtual void	Update( float fTimeDelta );
	virtual void RenderParticles( CParticleRenderIterator *pIterator );
	virtual void SimulateParticles( CParticleSimulateIterator *pIterator );


public:
	CNetworkVar( float,	m_flSpawnRate);
	CNetworkVar( float,	m_flParticleLifetime);
	CNetworkVar( float,	m_flStartSize);
	CNetworkVar( float,	m_flEndSize);
	CNetworkVar( float,	m_flSpawnRadius);
	float	m_flPreviousSpawnRate;

	CNetworkVar( bool,	m_bEmit);
	CNetworkVar( bool,	m_bDontRemove);

private:
	C_SporeExplosion( const C_SporeExplosion & );

	void	AddParticles( void );

	PMaterialHandle		m_hMaterial;
	TimedEvent			m_teParticleSpawn;

	SporeEffect			*m_pSporeEffect;
	CParticleMgr		*m_pParticleMgr;

public:
	BEGIN_INIT_RECV_TABLE(C_SporeExplosion)
	BEGIN_RECV_TABLE(C_SporeExplosion, DT_SporeExplosion, DT_BaseParticleEntity)
		RecvPropFloat(RECVINFO(m_flSpawnRate)),
		RecvPropFloat(RECVINFO(m_flParticleLifetime)),
		RecvPropFloat(RECVINFO(m_flStartSize)),
		RecvPropFloat(RECVINFO(m_flEndSize)),
		RecvPropFloat(RECVINFO(m_flSpawnRadius)),
		RecvPropBool(RECVINFO(m_bEmit)),
		RecvPropBool(RECVINFO(m_bDontRemove)),
	END_RECV_TABLE(DT_SporeExplosion)
	END_INIT_RECV_TABLE()
};

//
// Particle trail
//

class CSmokeParticle;

class C_FireTrail : public C_ParticleTrail
{
public:
	DECLARE_CLASS( C_FireTrail, C_ParticleTrail );
	DECLARE_CLIENTCLASS();

	C_FireTrail( void );
	virtual ~C_FireTrail( void );

	virtual void	Start( CParticleMgr *pParticleMgr, IPrototypeArgAccess *pArgs );
	virtual void	Update( float fTimeDelta );

private:

	enum
	{
		// Smoke
		FTRAIL_SMOKE1,
		FTRAIL_SMOKE2,

		// Large flame
		FTRAIL_FLAME1,
		FTRAIL_FLAME2,
		FTRAIL_FLAME3,
		FTRAIL_FLAME4,
		FTRAIL_FLAME5,

		NUM_FTRAIL_MATERIALS
	};

	CSmartPtr<CSimpleEmitter>	m_pTrailEmitter;
	CSmartPtr<CSmokeParticle>	m_pSmokeEmitter;

	PMaterialHandle				m_hMaterial[NUM_FTRAIL_MATERIALS];

	Vector						m_vecLastPosition;

	C_FireTrail( const C_FireTrail & );

public:
	BEGIN_INIT_RECV_TABLE(C_FireTrail)
	BEGIN_RECV_TABLE(C_FireTrail, DT_FireTrail, DT_BaseParticleEntity)
		RecvPropInt(RECVINFO(m_nAttachment)),
		RecvPropFloat(RECVINFO(m_flLifetime)),
	END_RECV_TABLE(DT_FireTrail)
	END_INIT_RECV_TABLE()
};













//==================================================
// C_DustTrail
//==================================================

class C_DustTrail : public C_BaseParticleEntity, public IPrototypeAppEffect
{
public:
	DECLARE_CLASS( C_DustTrail, C_BaseParticleEntity );
	DECLARE_CLIENTCLASS();
	
					C_DustTrail();
	virtual			~C_DustTrail();

public:

	// Enable/disable emission.
	void			SetEmit(bool bEmit);

	// Change the spawn rate.
	void			SetSpawnRate(float rate);


// C_BaseEntity.
public:
	virtual	void	OnDataChanged(DataUpdateType_t updateType);

	virtual void	CleanupToolRecordingState( KeyValues *msg );

// IPrototypeAppEffect.
public:
	virtual void	Start(CParticleMgr *pParticleMgr, IPrototypeArgAccess *pArgs);

// IParticleEffect.
public:
	virtual void	Update(float fTimeDelta);
	virtual void RenderParticles( CParticleRenderIterator *pIterator );
	virtual void SimulateParticles( CParticleSimulateIterator *pIterator );


public:
	// Effect parameters. These will assume default values but you can change them.
	CNetworkVar( float,			m_SpawnRate);			// How many particles per second.

	CNetworkVector(		m_Color);
	CNetworkVar( float,			m_Opacity);

	CNetworkVar( float,			m_ParticleLifetime);		// How long do the particles live?
	float			m_StartEmitTime;		// When did I start emitting particles?
	CNetworkVar( float,			m_StopEmitTime);			// When do I stop emitting particles? (-1 = never)
	
	CNetworkVar( float,			m_MinSpeed);				// Speed range.
	CNetworkVar( float,			m_MaxSpeed);
	
	CNetworkVar( float,			m_MinDirectedSpeed);		// Directed speed range.
	CNetworkVar( float,			m_MaxDirectedSpeed);

	CNetworkVar( float,			m_StartSize);			// Size ramp.
	CNetworkVar( float,			m_EndSize);

	CNetworkVar( float,			m_SpawnRadius);

	Vector			m_VelocityOffset;		// Emit the particles in a certain direction.

	CNetworkVar( bool,			m_bEmit);				// Keep emitting particles?

private:
	C_DustTrail( const C_DustTrail & );

#define DUSTTRAIL_MATERIALS 16
	PMaterialHandle	m_MaterialHandle[DUSTTRAIL_MATERIALS];
	TimedEvent		m_ParticleSpawn;

	CParticleMgr	*m_pParticleMgr;
	CSmartPtr<CSimpleEmitter> m_pDustEmitter;

public:
	BEGIN_INIT_RECV_TABLE(C_DustTrail)
	BEGIN_RECV_TABLE(C_DustTrail, DT_DustTrail, DT_BaseParticleEntity)
		RecvPropFloat(RECVINFO(m_SpawnRate)),
		RecvPropVector(RECVINFO(m_Color)),
		RecvPropFloat(RECVINFO(m_ParticleLifetime)),
		RecvPropFloat(RECVINFO(m_StopEmitTime)),
		RecvPropFloat(RECVINFO(m_MinSpeed)),
		RecvPropFloat(RECVINFO(m_MaxSpeed)),
		RecvPropFloat(RECVINFO(m_MinDirectedSpeed)),
		RecvPropFloat(RECVINFO(m_MaxDirectedSpeed)),
		RecvPropFloat(RECVINFO(m_StartSize)),
		RecvPropFloat(RECVINFO(m_EndSize)),
		RecvPropFloat(RECVINFO(m_SpawnRadius)),
		RecvPropInt(RECVINFO(m_bEmit)),
		RecvPropFloat(RECVINFO(m_Opacity)),
	END_RECV_TABLE(DT_DustTrail)
	END_INIT_RECV_TABLE()
};

#endif
