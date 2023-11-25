//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Defines the server side of a steam jet particle system entity.
//
// $NoKeywords: $
//=============================================================================//

#ifndef SMOKESTACK_H
#define SMOKESTACK_H
#pragma	once

#include "baseparticleentity.h"

//==================================================
// CSmokeStack
//==================================================

class CSmokeStackLightInfo
{
public:
	DECLARE_CLASS_NOBASE( CSmokeStackLightInfo );
	DECLARE_SIMPLE_DATADESC();
	//DECLARE_NETWORKVAR_CHAIN();
	DECLARE_EMBEDDED_NETWORKVAR();

	CNetworkVector( m_vPos );
	CNetworkVector( m_vColor );
	CNetworkVar( float, m_flIntensity );
};

class CSmokeStack : public CBaseParticleEntity
{
public:
	DECLARE_CLASS( CSmokeStack, CBaseParticleEntity );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

					CSmokeStack();
					~CSmokeStack();

	virtual void	Spawn( void );
	virtual void	Activate();
	virtual bool	KeyValue( const char *szKeyName, const char *szValue );
	virtual void	Precache();


protected:

	// Input handlers.
	void	InputTurnOn(inputdata_t &data);
	void	InputTurnOff(inputdata_t &data);
	void	InputToggle(inputdata_t &data);

	void	RecalcWindVector();


// Stuff from the datatable.
public:
	CNetworkVar( float, m_SpreadSpeed );
	CNetworkVar( float, m_Speed );
	CNetworkVar( float, m_StartSize );
	CNetworkVar( float, m_EndSize );
	CNetworkVar( float, m_Rate );
	CNetworkVar( float, m_JetLength );	// Length of the jet. Lifetime is derived from this.
	CNetworkVar( float, m_flRollSpeed );

	CNetworkVar( int, m_bEmit );		// Emit particles?
	CNetworkVar( float, m_flBaseSpread );
	
	CNetworkVarEmbedded(CSmokeStackLightInfo,m_AmbientLight);
	CNetworkVarEmbedded(CSmokeStackLightInfo,m_DirLight);

	CNetworkVar( float, m_flTwist );
	
	string_t		m_strMaterialModel;
	CNetworkVar( int, m_iMaterialModel );

	int				m_WindAngle;
	int				m_WindSpeed;
	CNetworkVector( m_vWind );		// m_vWind is just calculated from m_WindAngle and m_WindSpeed.

	bool			m_InitialState;

	BEGIN_INIT_SEND_TABLE(CSmokeStack)
	BEGIN_SEND_TABLE(CSmokeStack, DT_SmokeStack, DT_BaseParticleEntity)
		SendPropFloat(SENDINFO(m_SpreadSpeed), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_Speed), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_StartSize), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_EndSize), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_Rate), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_JetLength), 0, SPROP_NOSCALE),
		SendPropInt(SENDINFO(m_bEmit), 1, SPROP_UNSIGNED),
		SendPropFloat(SENDINFO(m_flBaseSpread), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_flRollSpeed), 0, SPROP_NOSCALE),

		// Note: the base color is specified in the smokestack entity, but the directional
		// and ambient light must come from env_particlelight entities.
		SendPropVector(SENDINFO_NOCHECK(m_DirLight.m_vPos), 0, SPROP_NOSCALE),
		SendPropVector(SENDINFO_NOCHECK(m_DirLight.m_vColor), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO_NOCHECK(m_DirLight.m_flIntensity), 0, SPROP_NOSCALE),

		SendPropVector(SENDINFO_NOCHECK(m_AmbientLight.m_vPos), 0, SPROP_NOSCALE),
		SendPropVector(SENDINFO_NOCHECK(m_AmbientLight.m_vColor), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO_NOCHECK(m_AmbientLight.m_flIntensity), 0, SPROP_NOSCALE),

		SendPropVector(SENDINFO(m_vWind), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_flTwist), 0, SPROP_NOSCALE),
		SendPropIntWithMinusOneFlag(SENDINFO(m_iMaterialModel), 16)

	END_SEND_TABLE(DT_SmokeStack)
	END_INIT_SEND_TABLE()
};

#endif // SMOKESTACK_H

