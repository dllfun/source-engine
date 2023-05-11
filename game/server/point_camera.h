//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CAMERA_H
#define CAMERA_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CPointCamera : public CBaseEntity
{
public:
	DECLARE_CLASS( CPointCamera, CBaseEntity );
	DECLARE_SERVERCLASS();
	DECLARE_SEND_TABLE_ACCESS(DT_PointCamera);
	DECLARE_DATADESC();
	CPointCamera();
	~CPointCamera();

	void Spawn( void );

	// Tell the client that this camera needs to be rendered
	void SetActive( bool bActive );
	int  UpdateTransmitState(void);

	void ChangeFOVThink( void );

	void InputChangeFOV( inputdata_t &inputdata );
	void InputSetOnAndTurnOthersOff( inputdata_t &inputdata );
	void InputSetOn( inputdata_t &inputdata );
	void InputSetOff( inputdata_t &inputdata );

private:
	float m_TargetFOV;
	float m_DegreesPerSecond;

	CNetworkVar( float, m_FOV );
	CNetworkVar( float, m_Resolution );
	CNetworkVar( bool, m_bFogEnable );
	CNetworkColor32( m_FogColor );
	CNetworkVar( float, m_flFogStart );
	CNetworkVar( float, m_flFogEnd );
	CNetworkVar( float, m_flFogMaxDensity );
	CNetworkVar( bool, m_bActive );
	CNetworkVar( bool, m_bUseScreenAspectRatio );

	// Allows the mapmaker to control whether a camera is active or not
	bool	m_bIsOn;

public:
	CPointCamera	*m_pNext;

	BEGIN_SEND_TABLE(CPointCamera, DT_PointCamera, DT_BaseEntity)
		SendPropFloat(SENDINFO(m_FOV), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_Resolution), 0, SPROP_NOSCALE),
		SendPropInt(SENDINFO(m_bFogEnable), 1, SPROP_UNSIGNED),
		SendPropInt(SENDINFO_STRUCTELEM(m_FogColor), 32, SPROP_UNSIGNED),
		SendPropFloat(SENDINFO(m_flFogStart), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_flFogEnd), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_flFogMaxDensity), 0, SPROP_NOSCALE),
		SendPropInt(SENDINFO(m_bActive), 1, SPROP_UNSIGNED),
		SendPropInt(SENDINFO(m_bUseScreenAspectRatio), 1, SPROP_UNSIGNED),
	END_SEND_TABLE(DT_PointCamera)
};

CPointCamera *GetPointCameraList();
#endif // CAMERA_H
