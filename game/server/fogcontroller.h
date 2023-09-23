//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef FOGCONTROLLER_H
#define FOGCONTROLLER_H
#ifdef _WIN32
#pragma once
#endif

#include "playernet_vars.h"
#include "igamesystem.h"

// Spawn Flags
#define SF_FOG_MASTER		0x0001

//=============================================================================
//
// Class Fog Controller:
// Compares a set of integer inputs to the one main input
// Outputs true if they are all equivalant, false otherwise
//
class CFogController : public CBaseEntity
{
public:
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DECLARE_CLASS( CFogController, CBaseEntity );

	CFogController();
	~CFogController();

	// Parse data from a map file
	virtual void Activate();
	virtual int UpdateTransmitState();

	// Input handlers
	void InputSetStartDist(inputdata_t &data);
	void InputSetEndDist(inputdata_t &data);
	void InputTurnOn(inputdata_t &data);
	void InputTurnOff(inputdata_t &data);
	void InputSetColor(inputdata_t &data);
	void InputSetColorSecondary(inputdata_t &data);
	void InputSetFarZ( inputdata_t &data );
	void InputSetAngles( inputdata_t &inputdata );
	void InputSetMaxDensity( inputdata_t &inputdata );

	void InputSetColorLerpTo(inputdata_t &data);
	void InputSetColorSecondaryLerpTo(inputdata_t &data);
	void InputSetStartDistLerpTo(inputdata_t &data);
	void InputSetEndDistLerpTo(inputdata_t &data);

	void InputStartFogTransition(inputdata_t &data);

	int DrawDebugTextOverlays(void);

	void SetLerpValues( void );
	void Spawn( void );

	bool IsMaster( void )					{ return HasSpawnFlags( SF_FOG_MASTER ); }

public:

	CNetworkVarEmbedded( fogparams_t, m_fog );
	bool					m_bUseAngles;
	int						m_iChangedVariables;

	BEGIN_INIT_SEND_TABLE(CFogController)
	BEGIN_SEND_TABLE_NOBASE(CFogController, DT_FogController)
		// fog data
		SendPropInt(SENDINFO_STRUCTELEM(m_fog.enable), 1, SPROP_UNSIGNED),
		SendPropInt(SENDINFO_STRUCTELEM(m_fog.blend), 1, SPROP_UNSIGNED),
		SendPropVector(SENDINFO_STRUCTELEM(m_fog.dirPrimary), -1, SPROP_COORD),
		SendPropInt(SENDINFO_STRUCTELEM(m_fog.colorPrimary), 32, SPROP_UNSIGNED),
		SendPropInt(SENDINFO_STRUCTELEM(m_fog.colorSecondary), 32, SPROP_UNSIGNED),
		SendPropFloat(SENDINFO_STRUCTELEM(m_fog.start), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO_STRUCTELEM(m_fog.end), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO_STRUCTELEM(m_fog.maxdensity), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO_STRUCTELEM(m_fog.farz), 0, SPROP_NOSCALE),

		SendPropInt(SENDINFO_STRUCTELEM(m_fog.colorPrimaryLerpTo), 32, SPROP_UNSIGNED),
		SendPropInt(SENDINFO_STRUCTELEM(m_fog.colorSecondaryLerpTo), 32, SPROP_UNSIGNED),
		SendPropFloat(SENDINFO_STRUCTELEM(m_fog.startLerpTo), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO_STRUCTELEM(m_fog.endLerpTo), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO_STRUCTELEM(m_fog.lerptime), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO_STRUCTELEM(m_fog.duration), 0, SPROP_NOSCALE),
	END_SEND_TABLE(DT_FogController)
	END_INIT_SEND_TABLE()
};

//=============================================================================
//
// Fog Controller System.
//
class CFogSystem : public CAutoGameSystem
{
public:

	// Creation/Init.
	CFogSystem( char const *name ) : CAutoGameSystem( name ) 
	{
		m_pMasterController = NULL;
	}

	~CFogSystem()
	{
		m_pMasterController = NULL;
	}

	virtual void LevelInitPreEntity();
	virtual void LevelInitPostEntity();
	CFogController *GetMasterFogController( void )			{ return m_pMasterController; }

private:

	CFogController	*m_pMasterController;
};

CFogSystem *FogSystem( void );

#endif // FOGCONTROLLER_H
