//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

//-----------------------------------------------------------------------------
// Purpose: An entity that spawns and controls a particle system
//-----------------------------------------------------------------------------
class CParticleSystem : public CBaseEntity
{
	DECLARE_CLASS( CParticleSystem, CBaseEntity );
public:
	DECLARE_SERVERCLASS();
	DECLARE_SEND_TABLE_ACCESS(DT_ParticleSystem);
	DECLARE_DATADESC();

	CParticleSystem();

	virtual void Precache( void );
	virtual void Spawn( void );
	virtual void Activate( void );
	virtual int  UpdateTransmitState(void);

	void		StartParticleSystem( void );
	void		StopParticleSystem( void );

	void		InputStart( inputdata_t &inputdata );
	void		InputStop( inputdata_t &inputdata );
	void		StartParticleSystemThink( void );

	enum { kMAXCONTROLPOINTS = 63 }; ///< actually one less than the total number of cpoints since 0 is assumed to be me

protected:

	/// Load up and resolve the entities that are supposed to be the control points 
	void ReadControlPointEnts( void );

	bool				m_bStartActive;
	string_t			m_iszEffectName;
	
	CNetworkVar( bool,	m_bActive );
	CNetworkVar( int,	m_iEffectIndex )
	CNetworkVar( float,	m_flStartTime );	// Time at which this effect was started.  This is used after restoring an active effect.

	string_t			m_iszControlPointNames[kMAXCONTROLPOINTS];
	CNetworkArray( EHANDLE, m_hControlPointEnts, kMAXCONTROLPOINTS );
	CNetworkArray( unsigned char, m_iControlPointParents, kMAXCONTROLPOINTS );
	CNetworkVar( bool,	m_bWeatherEffect );

	BEGIN_INIT_SEND_TABLE(CParticleSystem)
	BEGIN_SEND_TABLE_NOBASE(CParticleSystem, DT_ParticleSystem)
		SendPropVector(SENDINFO(m_vecOrigin), -1, SPROP_COORD | SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin),
		SendPropEHandle(SENDINFO(m_hOwnerEntity)),
		SendPropEHandle(SENDINFO_NAME(m_hMoveParent, moveparent)),
		SendPropInt(SENDINFO(m_iParentAttachment), NUM_PARENTATTACHMENT_BITS, SPROP_UNSIGNED),
		SendPropQAngles(SENDINFO(m_angRotation), 13, SPROP_CHANGES_OFTEN, SendProxy_Angles),

		SendPropInt(SENDINFO(m_iEffectIndex), MAX_PARTICLESYSTEMS_STRING_BITS, SPROP_UNSIGNED),
		SendPropBool(SENDINFO(m_bActive)),
		SendPropFloat(SENDINFO(m_flStartTime)),

		SendPropArray3(SENDINFO_ARRAY3(m_hControlPointEnts), SendPropEHandle(SENDINFO_ARRAY(m_hControlPointEnts))),
		SendPropArray3(SENDINFO_ARRAY3(m_iControlPointParents), SendPropInt(SENDINFO_ARRAY(m_iControlPointParents), 3, SPROP_UNSIGNED)),
		SendPropBool(SENDINFO(m_bWeatherEffect)),
	END_SEND_TABLE(DT_ParticleSystem)
	END_INIT_SEND_TABLE()
};

#endif // PARTICLE_SYSTEM_H
