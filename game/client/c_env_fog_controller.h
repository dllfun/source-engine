//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_ENV_FOG_CONTROLLER_H
#define C_ENV_FOG_CONTROLLER_H

#define CFogController C_FogController

//=============================================================================
//
// Class Fog Controller:
// Compares a set of integer inputs to the one main input
// Outputs true if they are all equivalant, false otherwise
//
class C_FogController : public C_BaseEntity
{
public:
	DECLARE_NETWORKCLASS();
	DECLARE_CLASS( C_FogController, C_BaseEntity );

	C_FogController();

public:

	fogparams_t				m_fog;

	//-----------------------------------------------------------------------------
// Datatable
//-----------------------------------------------------------------------------
public:
	BEGIN_INIT_RECV_TABLE(CFogController)
	BEGIN_NETWORK_TABLE_NOBASE(CFogController, DT_FogController)
		// fog data
		RecvPropInt(RECVINFO(m_fog.enable)),
		RecvPropInt(RECVINFO(m_fog.blend)),
		RecvPropVector(RECVINFO(m_fog.dirPrimary)),
		RecvPropInt(RECVINFO(m_fog.colorPrimary)),
		RecvPropInt(RECVINFO(m_fog.colorSecondary)),
		RecvPropFloat(RECVINFO(m_fog.start)),
		RecvPropFloat(RECVINFO(m_fog.end)),
		RecvPropFloat(RECVINFO(m_fog.farz)),
		RecvPropFloat(RECVINFO(m_fog.maxdensity)),

		RecvPropInt(RECVINFO(m_fog.colorPrimaryLerpTo)),
		RecvPropInt(RECVINFO(m_fog.colorSecondaryLerpTo)),
		RecvPropFloat(RECVINFO(m_fog.startLerpTo)),
		RecvPropFloat(RECVINFO(m_fog.endLerpTo)),
		RecvPropFloat(RECVINFO(m_fog.lerptime)),
		RecvPropFloat(RECVINFO(m_fog.duration)),
	END_NETWORK_TABLE(DT_FogController)
	END_INIT_RECV_TABLE()
};


#endif // C_ENV_FOG_CONTROLLER_H