//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef ENTITYDISSOLVE_H
#define ENTITYDISSOLVE_H

#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "sendproxy.h"

class CEntityDissolve : public CBaseEntity 
{
public:
	DECLARE_SERVERCLASS();
	DECLARE_SEND_TABLE_ACCESS(DT_EntityDissolve);
	DECLARE_CLASS( CEntityDissolve, CBaseEntity );

	CEntityDissolve( void );
	~CEntityDissolve( void );

	static CEntityDissolve	*Create( CBaseEntity *pTarget, const char *pMaterialName, 
		float flStartTime, int nDissolveType = 0, bool *pRagdollCreated = NULL );
	static CEntityDissolve	*Create( CBaseEntity *pTarget, CBaseEntity *pSource );
	
	void	Precache();
	void	Spawn();
	void	AttachToEntity( CBaseEntity *pTarget );
	void	SetStartTime( float flStartTime );
	void	SetDissolverOrigin( Vector vOrigin ) { m_vDissolverOrigin = vOrigin; }
	void	SetMagnitude( int iMagnitude ){ m_nMagnitude = iMagnitude; }
	void	SetDissolveType( int iType ) { m_nDissolveType = iType;	}

	Vector	GetDissolverOrigin( void ) 
	{ 
		Vector vReturn = m_vDissolverOrigin; 
		return vReturn;	
	}
	int		GetMagnitude( void ) { return m_nMagnitude;	}
	int		GetDissolveType( void ) { return m_nDissolveType;	}

	DECLARE_DATADESC();

	CNetworkVar( float, m_flStartTime );
	CNetworkVar( float, m_flFadeInStart );
	CNetworkVar( float, m_flFadeInLength );
	CNetworkVar( float, m_flFadeOutModelStart );
	CNetworkVar( float, m_flFadeOutModelLength );
	CNetworkVar( float, m_flFadeOutStart );
	CNetworkVar( float, m_flFadeOutLength );

protected:
	void	InputDissolve( inputdata_t &inputdata );
	void	DissolveThink( void );
	void	ElectrocuteThink( void );

	CNetworkVar( int, m_nDissolveType );
	CNetworkVector( m_vDissolverOrigin );
	CNetworkVar( int, m_nMagnitude );

public:
	BEGIN_INIT_SEND_TABLE(CEntityDissolve)
	BEGIN_SEND_TABLE(CEntityDissolve, DT_EntityDissolve, DT_BaseEntity)
		SendPropTime(SENDINFO(m_flStartTime)),
		SendPropFloat(SENDINFO(m_flFadeInStart), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_flFadeInLength), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_flFadeOutModelStart), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_flFadeOutModelLength), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_flFadeOutStart), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_flFadeOutLength), 0, SPROP_NOSCALE),
		SendPropInt(SENDINFO(m_nDissolveType), ENTITY_DISSOLVE_BITS, SPROP_UNSIGNED),
		SendPropVector(SENDINFO(m_vDissolverOrigin), 0, SPROP_NOSCALE),
		SendPropInt(SENDINFO(m_nMagnitude), 8, SPROP_UNSIGNED),
	END_SEND_TABLE(DT_EntityDissolve)
	END_INIT_SEND_TABLE()
};

#endif // ENTITYDISSOLVE_H
