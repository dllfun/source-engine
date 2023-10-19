//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef ANTLION_DUST_H
#define ANTLION_DUST_H

#include "te_particlesystem.h"

class CTEAntlionDust : public CTEParticleSystem
{
public:
	
	DECLARE_CLASS( CTEAntlionDust, CTEParticleSystem );
	DECLARE_SERVERCLASS();

					CTEAntlionDust( const char *name );
	virtual			~CTEAntlionDust( void );

	virtual void	Test( const Vector& current_origin, const QAngle& current_angles ) { };

	CNetworkVector( m_vecOrigin );
	CNetworkVar( QAngle, m_vecAngles );
	CNetworkVar( bool, m_bBlockedSpawner );

public:
	BEGIN_INIT_SEND_TABLE(CTEAntlionDust)
	BEGIN_SEND_TABLE(CTEAntlionDust, DT_TEAntlionDust, DT_TEParticleSystem)
		SendPropVector(SENDINFO(m_vecOrigin)),
		SendPropVector(SENDINFO(m_vecAngles)),
		SendPropBool(SENDINFO(m_bBlockedSpawner)),
	END_SEND_TABLE()
	END_INIT_SEND_TABLE()
};

void UTIL_CreateAntlionDust( const Vector &origin, const QAngle &angles, bool bBlockedSpawner = false );

#endif	//ANTLION_DUST_H