//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef ENTITYPARTICLETRAIL_SHARED_H
#define ENTITYPARTICLETRAIL_SHARED_H

#ifdef _WIN32
#pragma once
#endif


//-----------------------------------------------------------------------------
// For networking this bad boy
//-----------------------------------------------------------------------------
#ifdef CLIENT_DLL
EXTERN_RECV_TABLE( DT_EntityParticleTrailInfo );
#else
EXTERN_SEND_TABLE( DT_EntityParticleTrailInfo );
#endif


//-----------------------------------------------------------------------------
// Particle trail info
//-----------------------------------------------------------------------------
struct EntityParticleTrailInfo_t
{
	EntityParticleTrailInfo_t();

	DECLARE_CLASS_NOBASE( EntityParticleTrailInfo_t );
	DECLARE_SIMPLE_DATADESC();
	DECLARE_EMBEDDED_NETWORKVAR();

	string_t m_strMaterialName;
	CNetworkVar( float, m_flLifetime );
	CNetworkVar( float, m_flStartSize );
	CNetworkVar( float, m_flEndSize );

#ifndef CLIENT_DLL
	BEGIN_INIT_SEND_TABLE(EntityParticleTrailInfo_t)
	BEGIN_NETWORK_TABLE_NOBASE(EntityParticleTrailInfo_t, DT_EntityParticleTrailInfo)
		SendPropFloat(SENDINFO(m_flLifetime), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_flStartSize), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_flEndSize), 0, SPROP_NOSCALE),
	END_NETWORK_TABLE(DT_EntityParticleTrailInfo)
	END_INIT_SEND_TABLE()
#endif

		//-----------------------------------------------------------------------------
// Networking
//-----------------------------------------------------------------------------
#ifdef CLIENT_DLL
	BEGIN_INIT_RECV_TABLE(EntityParticleTrailInfo_t)
	BEGIN_NETWORK_TABLE_NOBASE(EntityParticleTrailInfo_t, DT_EntityParticleTrailInfo)
		RecvPropFloat(RECVINFO(m_flLifetime)),
		RecvPropFloat(RECVINFO(m_flStartSize)),
		RecvPropFloat(RECVINFO(m_flEndSize)),
	END_NETWORK_TABLE(DT_EntityParticleTrailInfo)
	END_INIT_RECV_TABLE()
#endif
};



#endif // ENTITYPARTICLETRAIL_SHARED_H
