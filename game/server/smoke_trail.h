//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef SMOKE_TRAIL_H
#define SMOKE_TRAIL_H

#include "baseparticleentity.h"

//==================================================
// SmokeTrail
//==================================================

class SmokeTrail : public CBaseParticleEntity
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( SmokeTrail, CBaseParticleEntity );
	DECLARE_SERVERCLASS();

	SmokeTrail();
	virtual bool KeyValue( const char *szKeyName, const char *szValue ); 
	void					SetEmit(bool bVal);
	void					FollowEntity( CBaseEntity *pEntity, const char *pAttachmentName = NULL);
	static	SmokeTrail*		CreateSmokeTrail();

public:
	// Effect parameters. These will assume default values but you can change them.
	CNetworkVector( m_StartColor );			// Fade between these colors.
	CNetworkVector( m_EndColor );
	CNetworkVar( float, m_Opacity );

	CNetworkVar( float, m_SpawnRate );			// How many particles per second.
	CNetworkVar( float, m_ParticleLifetime );		// How long do the particles live?
	CNetworkVar( float, m_StopEmitTime );			// When do I stop emitting particles?
	CNetworkVar( float, m_MinSpeed );				// Speed range.
	CNetworkVar( float, m_MaxSpeed );
	CNetworkVar( float, m_StartSize );			// Size ramp.
	CNetworkVar( float, m_EndSize );	
	CNetworkVar( float, m_SpawnRadius );
	CNetworkVar( float, m_MinDirectedSpeed );				// Speed range.
	CNetworkVar( float, m_MaxDirectedSpeed );
	CNetworkVar( bool, m_bEmit );

	CNetworkVar( int, m_nAttachment );

	BEGIN_INIT_SEND_TABLE(SmokeTrail)
	BEGIN_SEND_TABLE(SmokeTrail, DT_SmokeTrail, DT_BaseParticleEntity)
		SendPropFloat(SENDINFO(m_SpawnRate), 8, 0, 1, 1024),
		SendPropVector(SENDINFO(m_StartColor), 8, 0, 0, 1),
		SendPropVector(SENDINFO(m_EndColor), 8, 0, 0, 1),
		SendPropFloat(SENDINFO(m_ParticleLifetime), 16, SPROP_ROUNDUP, 0.1, 100),
		SendPropFloat(SENDINFO(m_StopEmitTime), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_MinSpeed), -1, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_MaxSpeed), -1, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_MinDirectedSpeed), -1, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_MaxDirectedSpeed), -1, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_StartSize), -1, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_EndSize), -1, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_SpawnRadius), -1, SPROP_NOSCALE),
		SendPropBool(SENDINFO(m_bEmit)),
		SendPropInt(SENDINFO(m_nAttachment), 32),
		SendPropFloat(SENDINFO(m_Opacity), -1, SPROP_NOSCALE),
	END_SEND_TABLE(DT_SmokeTrail)
	END_INIT_SEND_TABLE()
};

//==================================================
// RocketTrail
//==================================================

class RocketTrail : public CBaseParticleEntity
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( RocketTrail, CBaseParticleEntity );
	DECLARE_SERVERCLASS();

	RocketTrail();
	void					SetEmit(bool bVal);
	void					FollowEntity( CBaseEntity *pEntity, const char *pAttachmentName = NULL);
	static RocketTrail		*CreateRocketTrail();

public:
	// Effect parameters. These will assume default values but you can change them.
	CNetworkVector( m_StartColor );			// Fade between these colors.
	CNetworkVector( m_EndColor );
	CNetworkVar( float, m_Opacity );

	CNetworkVar( float, m_SpawnRate );			// How many particles per second.
	CNetworkVar( float, m_ParticleLifetime );		// How long do the particles live?
	CNetworkVar( float, m_StopEmitTime );			// When do I stop emitting particles?
	CNetworkVar( float, m_MinSpeed );				// Speed range.
	CNetworkVar( float, m_MaxSpeed );
	CNetworkVar( float, m_StartSize );			// Size ramp.
	CNetworkVar( float, m_EndSize );	
	CNetworkVar( float, m_SpawnRadius );
	
	CNetworkVar( bool, m_bEmit );

	CNetworkVar( int, m_nAttachment );
	
	CNetworkVar( bool, m_bDamaged );

	CNetworkVar( float, m_flFlareScale );			// Size of the flare

	BEGIN_INIT_SEND_TABLE(RocketTrail)
	BEGIN_SEND_TABLE(RocketTrail, DT_RocketTrail, DT_BaseParticleEntity)
		SendPropFloat(SENDINFO(m_SpawnRate), 8, 0, 1, 1024),
		SendPropVector(SENDINFO(m_StartColor), 8, 0, 0, 1),
		SendPropVector(SENDINFO(m_EndColor), 8, 0, 0, 1),
		SendPropFloat(SENDINFO(m_ParticleLifetime), 16, SPROP_ROUNDUP, 0.1, 100),
		SendPropFloat(SENDINFO(m_StopEmitTime), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_MinSpeed), -1, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_MaxSpeed), -1, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_StartSize), -1, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_EndSize), -1, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_SpawnRadius), -1, SPROP_NOSCALE),
		SendPropBool(SENDINFO(m_bEmit)),
		SendPropInt(SENDINFO(m_nAttachment), 32),
		SendPropFloat(SENDINFO(m_Opacity), -1, SPROP_NOSCALE),
		SendPropInt(SENDINFO(m_bDamaged), 1, SPROP_UNSIGNED),
		SendPropFloat(SENDINFO(m_flFlareScale), -1, SPROP_NOSCALE),
	END_SEND_TABLE(DT_RocketTrail)
	END_INIT_SEND_TABLE()
};

//==================================================
// SporeTrail
//==================================================

class SporeTrail : public CBaseParticleEntity
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( SporeTrail, CBaseParticleEntity );
	DECLARE_SERVERCLASS();

	SporeTrail( void );

	static SporeTrail*		CreateSporeTrail();

//Data members
public:

	CNetworkVector( m_vecEndColor );

	CNetworkVar( float, m_flSpawnRate );
	CNetworkVar( float, m_flParticleLifetime );
	CNetworkVar( float, m_flStartSize );
	CNetworkVar( float, m_flEndSize );
	CNetworkVar( float, m_flSpawnRadius );

	CNetworkVar( bool, m_bEmit );

	BEGIN_INIT_SEND_TABLE(SporeTrail)
	BEGIN_SEND_TABLE(SporeTrail, DT_SporeTrail, DT_BaseParticleEntity)
		SendPropFloat(SENDINFO(m_flSpawnRate), 8, 0, 1, 1024),
		SendPropVector(SENDINFO(m_vecEndColor), 8, 0, 0, 1),
		SendPropFloat(SENDINFO(m_flParticleLifetime), 16, SPROP_ROUNDUP, 0.1, 100),
		SendPropFloat(SENDINFO(m_flStartSize), -1, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_flEndSize), -1, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_flSpawnRadius), -1, SPROP_NOSCALE),
		SendPropBool(SENDINFO(m_bEmit)),
	END_SEND_TABLE(DT_SporeTrail)
	END_INIT_SEND_TABLE()
};

//==================================================
// SporeExplosion
//==================================================

class SporeExplosion : public CBaseParticleEntity
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( SporeExplosion, CBaseParticleEntity );
	DECLARE_SERVERCLASS();

	SporeExplosion( void );
	void Spawn( void );

	static SporeExplosion*		CreateSporeExplosion();

	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );

//Data members
public:

	bool m_bDisabled;

	CNetworkVar( float, m_flSpawnRate );
	CNetworkVar( float, m_flParticleLifetime );
	CNetworkVar( float, m_flStartSize );
	CNetworkVar( float, m_flEndSize );
	CNetworkVar( float, m_flSpawnRadius );

	CNetworkVar( bool, m_bEmit );
	CNetworkVar( bool, m_bDontRemove );

	BEGIN_INIT_SEND_TABLE(SporeExplosion)
	BEGIN_SEND_TABLE(SporeExplosion, DT_SporeExplosion, DT_BaseParticleEntity)
		SendPropFloat(SENDINFO(m_flSpawnRate), 8, 0, 1, 1024),
		SendPropFloat(SENDINFO(m_flParticleLifetime), 16, SPROP_ROUNDUP, 0.1, 100),
		SendPropFloat(SENDINFO(m_flStartSize), -1, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_flEndSize), -1, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_flSpawnRadius), -1, SPROP_NOSCALE),
		SendPropBool(SENDINFO(m_bEmit)),
		SendPropBool(SENDINFO(m_bDontRemove)),
	END_SEND_TABLE(DT_SporeExplosion)
	END_INIT_SEND_TABLE()
};

//==================================================
// CFireTrail
//==================================================

class CFireTrail : public CBaseParticleEntity
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( CFireTrail, CBaseParticleEntity );
	DECLARE_SERVERCLASS();

	static CFireTrail	*CreateFireTrail( void );
	void				FollowEntity( CBaseEntity *pEntity, const char *pAttachmentName );
	void				Precache( void );

	CNetworkVar( int, m_nAttachment );
	CNetworkVar( float, m_flLifetime );

	BEGIN_INIT_SEND_TABLE(CFireTrail)
	BEGIN_SEND_TABLE(CFireTrail, DT_FireTrail, DT_BaseParticleEntity)
		SendPropInt(SENDINFO(m_nAttachment), 32),
		SendPropFloat(SENDINFO(m_flLifetime), 0, SPROP_NOSCALE),
	END_SEND_TABLE(DT_FireTrail)
	END_INIT_SEND_TABLE()
};

//==================================================
// DustTrail
//==================================================

class DustTrail : public CBaseParticleEntity
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( DustTrail, CBaseParticleEntity );
	DECLARE_SERVERCLASS();

	DustTrail();
	virtual bool KeyValue( const char *szKeyName, const char *szValue ); 
	void					SetEmit(bool bVal);
	static	DustTrail*		CreateDustTrail();

public:
	// Effect parameters. These will assume default values but you can change them.
	CNetworkVector( m_Color );
	CNetworkVar( float, m_Opacity );

	CNetworkVar( float, m_SpawnRate );			// How many particles per second.
	CNetworkVar( float, m_ParticleLifetime );		// How long do the particles live?
	CNetworkVar( float, m_StopEmitTime );			// When do I stop emitting particles?
	CNetworkVar( float, m_MinSpeed );				// Speed range.
	CNetworkVar( float, m_MaxSpeed );
	CNetworkVar( float, m_StartSize );			// Size ramp.
	CNetworkVar( float, m_EndSize );	
	CNetworkVar( float, m_SpawnRadius );
	CNetworkVar( float, m_MinDirectedSpeed );				// Speed range.
	CNetworkVar( float, m_MaxDirectedSpeed );
	CNetworkVar( bool, m_bEmit );

	CNetworkVar( int, m_nAttachment );

	BEGIN_INIT_SEND_TABLE(DustTrail)
	BEGIN_SEND_TABLE(DustTrail, DT_DustTrail, DT_BaseParticleEntity)
		SendPropFloat(SENDINFO(m_SpawnRate), 8, 0, 1, 1024),
		SendPropVector(SENDINFO(m_Color), 8, 0, 0, 1),
		SendPropFloat(SENDINFO(m_ParticleLifetime), 16, SPROP_ROUNDUP, 0.1, 100),
		SendPropFloat(SENDINFO(m_StopEmitTime), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_MinSpeed), -1, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_MaxSpeed), -1, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_MinDirectedSpeed), -1, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_MaxDirectedSpeed), -1, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_StartSize), -1, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_EndSize), -1, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_SpawnRadius), -1, SPROP_NOSCALE),
		SendPropBool(SENDINFO(m_bEmit)),
		SendPropFloat(SENDINFO(m_Opacity), -1, SPROP_NOSCALE),
	END_SEND_TABLE(DT_DustTrail)
	END_INIT_SEND_TABLE()
};


#endif
