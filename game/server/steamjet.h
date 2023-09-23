//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Defines the server side of a steam jet particle system entity.
//
// $NoKeywords: $
//=============================================================================//

#ifndef STEAMJET_H
#define STEAMJET_H
#pragma	once

#include "baseparticleentity.h"

//NOTENOTE: Mirrored in cl_dlls\c_steamjet.cpp
#define	STEAM_NORMAL	0
#define	STEAM_HEATWAVE	1

//==================================================
// CSteamJet
//==================================================

class CSteamJet : public CBaseParticleEntity
{
public:
	CSteamJet();
	DECLARE_CLASS( CSteamJet, CBaseParticleEntity );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
	DECLARE_SEND_TABLE_ACCESS(DT_SteamJet);

	virtual void	Spawn( void );
	virtual void	Precache( void );

protected:

	// Input handlers.
	void InputTurnOn(inputdata_t &data);
	void InputTurnOff(inputdata_t &data);
	void InputToggle(inputdata_t &data);

// Stuff from the datatable.
public:
	CNetworkVar( float, m_SpreadSpeed );
	CNetworkVar( float, m_Speed );
	CNetworkVar( float, m_StartSize );
	CNetworkVar( float, m_EndSize );
	CNetworkVar( float, m_Rate );
	CNetworkVar( float, m_JetLength );	// Length of the jet. Lifetime is derived from this.

	CNetworkVar( int, m_bEmit );		// Emit particles?
	CNetworkVar( bool, m_bFaceLeft );	// For support of legacy env_steamjet, which faced left instead of forward.
	bool			m_InitialState;

	CNetworkVar( int, m_nType );		// Type of steam (normal, heatwave)
	CNetworkVar( float, m_flRollSpeed );

	virtual void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	BEGIN_INIT_SEND_TABLE(CSteamJet)
	BEGIN_SEND_TABLE(CSteamJet, DT_SteamJet, DT_BaseParticleEntity)
		SendPropFloat(SENDINFO(m_SpreadSpeed), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_Speed), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_StartSize), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_EndSize), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_Rate), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_JetLength), 0, SPROP_NOSCALE),
		SendPropInt(SENDINFO(m_bEmit), 1, SPROP_UNSIGNED),
		SendPropInt(SENDINFO(m_bFaceLeft), 1, SPROP_UNSIGNED), // For support of legacy env_steamjet, which faced left instead of forward.
		SendPropInt(SENDINFO(m_nType), 32, SPROP_UNSIGNED),
		SendPropInt(SENDINFO(m_spawnflags), 8, SPROP_UNSIGNED),
		SendPropFloat(SENDINFO(m_flRollSpeed), 0, SPROP_NOSCALE),
	END_SEND_TABLE(DT_SteamJet)
	END_INIT_SEND_TABLE()
};

#endif // STEAMJET_H

