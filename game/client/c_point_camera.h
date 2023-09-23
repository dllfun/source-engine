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
	float m_FOV;
	float m_Resolution;
	bool m_bFogEnable;
	color32 m_FogColor;
	float m_flFogStart;
	float m_flFogEnd;
	float m_flFogMaxDensity;
	bool m_bActive;
	bool m_bUseScreenAspectRatio;

public:
	C_PointCamera	*m_pNext;

public:
	BEGIN_INIT_RECV_TABLE(C_PointCamera)
	BEGIN_RECV_TABLE(C_PointCamera, DT_PointCamera, DT_BaseEntity)
		RecvPropFloat(RECVINFO(m_FOV)),
		RecvPropFloat(RECVINFO(m_Resolution)),
		RecvPropInt(RECVINFO(m_bFogEnable)),
		RecvPropInt(RECVINFO(m_FogColor)),
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
