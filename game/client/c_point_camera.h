//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_POINTCAMERA_H
#define C_POINTCAMERA_H
#ifdef _WIN32
#pragma once
#endif

#include "c_baseentity.h"
#include "basetypes.h"

class C_PointCamera : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_PointCamera, C_BaseEntity );
	DECLARE_CLIENTCLASS();

public:
	C_PointCamera();
	~C_PointCamera();

	bool IsActive();
	
	// C_BaseEntity.
	virtual bool	ShouldDraw();

	float			GetFOV();
	float			GetResolution();
	bool			IsFogEnabled();
	void			GetFogColor( unsigned char &r, unsigned char &g, unsigned char &b );
	float			GetFogStart();
	float			GetFogMaxDensity();
	float			GetFogEnd();
	bool			UseScreenAspectRatio() const { return m_bUseScreenAspectRatio; }

	virtual void	GetToolRecordingState( KeyValues *msg );

private:
	CNetworkVar( float, m_FOV);
	CNetworkVar( float, m_Resolution);
	CNetworkVar( bool, m_bFogEnable);
	CNetworkColor32( m_FogColor);
	CNetworkVar( float, m_flFogStart);
	CNetworkVar( float, m_flFogEnd);
	CNetworkVar( float, m_flFogMaxDensity);
	CNetworkVar( bool, m_bActive);
	CNetworkVar( bool, m_bUseScreenAspectRatio);

public:
	C_PointCamera	*m_pNext;

public:
	BEGIN_INIT_RECV_TABLE(C_PointCamera)
	BEGIN_RECV_TABLE(C_PointCamera, DT_PointCamera, DT_BaseEntity)
		RecvPropFloat(RECVINFO(m_FOV)),
		RecvPropFloat(RECVINFO(m_Resolution)),
		RecvPropInt(RECVINFO(m_bFogEnable)),
		RecvPropColor32(RECVINFO(m_FogColor)),
		RecvPropFloat(RECVINFO(m_flFogStart)),
		RecvPropFloat(RECVINFO(m_flFogEnd)),
		RecvPropFloat(RECVINFO(m_flFogMaxDensity)),
		RecvPropInt(RECVINFO(m_bActive)),
		RecvPropInt(RECVINFO(m_bUseScreenAspectRatio)),
	END_RECV_TABLE(DT_PointCamera)
	END_INIT_RECV_TABLE()
};

C_PointCamera *GetPointCameraList();

#endif // C_POINTCAMERA_H
