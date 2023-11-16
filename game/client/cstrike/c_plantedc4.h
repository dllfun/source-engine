//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef C_PLANTEDC4_H
#define C_PLANTEDC4_H

#include "cbase.h"
#include "in_buttons.h"
#include "decals.h"

#include "c_cs_player.h"

#include "utlvector.h"

// ------------------------------------------------------------------------------------------ //
// CPlantedC4 class.
// For now to show the planted c4 on the radar - client proxy to remove the CBaseAnimating 
// network vars?
// ------------------------------------------------------------------------------------------ //

class C_PlantedC4 : public C_BaseAnimating
{
public:
	DECLARE_CLASS( C_PlantedC4, CBaseAnimating );
	DECLARE_CLIENTCLASS();

	C_PlantedC4();
	virtual ~C_PlantedC4();

	void Explode( void );
	void Spawn( void );
	virtual void SetDormant( bool bDormant );

	void ClientThink( void );

	int GetSecondsRemaining( void ) { return ceil( m_flC4Blow - gpGlobals->GetCurTime() ); }

	inline bool IsBombActive( void ) { return m_bBombTicking; }
	CNetworkVar( bool, m_bBombTicking );

	float m_flNextGlow;
	float m_flNextBeep;

	CNetworkVar( float, m_flC4Blow);
	CNetworkVar( float, m_flTimerLength);

	CNetworkVar( float, m_flDefuseLength );	
	CNetworkVar( float, m_flDefuseCountDown ); 

	float GetDefuseProgress( void )
	{	
		float flProgress = 1.0f;

		if( m_flDefuseLength > 0.0 )
		{
			flProgress = ( ( m_flDefuseCountDown - gpGlobals->GetCurTime() ) / m_flDefuseLength );
		}

		return flProgress;
	}

	float	m_flNextRadarFlashTime;	// next time to change flash state
	bool	m_bRadarFlash;			// is the flash on or off
	CNewParticleEffect *m_pC4Explosion; // client side explosion particle effect for the bomb

public:
	BEGIN_INIT_RECV_TABLE(C_PlantedC4)
	BEGIN_RECV_TABLE(C_PlantedC4, DT_PlantedC4, DT_BaseAnimating)
		RecvPropBool(RECVINFO(m_bBombTicking)),
		RecvPropFloat(RECVINFO(m_flC4Blow)),
		RecvPropFloat(RECVINFO(m_flTimerLength)),
		RecvPropFloat(RECVINFO(m_flDefuseLength)),
		RecvPropFloat(RECVINFO(m_flDefuseCountDown)),
	END_RECV_TABLE(DT_PlantedC4)
	END_INIT_RECV_TABLE()
};

extern CUtlVector< C_PlantedC4* > g_PlantedC4s;

#endif //C_PLANTEDC4_H