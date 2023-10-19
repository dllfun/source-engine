//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#if !defined( C_TE_BASEBEAM_H )
#define C_TE_BASEBEAM_H
#ifdef _WIN32
#pragma once
#endif

#include "c_basetempentity.h"

//-----------------------------------------------------------------------------
// Purpose: Base entity for beam te's
//-----------------------------------------------------------------------------
class C_TEBaseBeam : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEBaseBeam, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

private:

public:

					C_TEBaseBeam( void );
	virtual			~C_TEBaseBeam( void );

	virtual void	PreDataUpdate( DataUpdateType_t updateType );
	virtual void	PostDataUpdate( DataUpdateType_t updateType );

public:
	int				m_nModelIndex;
	int				m_nHaloIndex;
	int				m_nStartFrame;
	int				m_nFrameRate;
	float			m_fLife;
	float			m_fWidth;
	float			m_fEndWidth;
	int				m_nFadeLength;
	float			m_fAmplitude;
	int				r, g, b, a;
	int				m_nSpeed;
	int				m_nFlags;

public:
	BEGIN_INIT_RECV_TABLE(C_TEBaseBeam)
	BEGIN_RECV_TABLE(C_TEBaseBeam, DT_BaseBeam, DT_BaseTempEntity)
		RecvPropInt(RECVINFO(m_nModelIndex)),
		RecvPropInt(RECVINFO(m_nHaloIndex)),
		RecvPropInt(RECVINFO(m_nStartFrame)),
		RecvPropInt(RECVINFO(m_nFrameRate)),
		RecvPropFloat(RECVINFO(m_fLife)),
		RecvPropFloat(RECVINFO(m_fWidth)),
		RecvPropFloat(RECVINFO(m_fEndWidth)),
		RecvPropInt(RECVINFO(m_nFadeLength)),
		RecvPropFloat(RECVINFO(m_fAmplitude)),
		RecvPropInt(RECVINFO(m_nSpeed)),
		RecvPropInt(RECVINFO(r)),
		RecvPropInt(RECVINFO(g)),
		RecvPropInt(RECVINFO(b)),
		RecvPropInt(RECVINFO(a)),
		RecvPropInt(RECVINFO(m_nFlags)),
	END_RECV_TABLE(DT_BaseBeam)
	END_INIT_RECV_TABLE()
};

EXTERN_RECV_TABLE(DT_BaseBeam);

#endif // C_TE_BASEBEAM_H