//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_SUN_H
#define C_SUN_H
#ifdef _WIN32
#pragma once
#endif

#include "c_baseentity.h"
#include "utllinkedlist.h"
#include "glow_overlay.h"
#include "sun_shared.h"

//
// Special glow overlay
//

class C_SunGlowOverlay : public CGlowOverlay
{
	virtual void CalcSpriteColorAndSize( float flDot, CGlowSprite *pSprite, float *flHorzSize, float *flVertSize, Vector *vColor )
	{
		if ( m_bModulateByDot )
		{
			float alpha = RemapVal( flDot, 1.0f, 0.9f, 0.75f, 0.0f );
			alpha = clamp( alpha, 0.0f, 0.75f );

			*flHorzSize = pSprite->m_flHorzSize * 6.0f;
			*flVertSize = pSprite->m_flVertSize * 6.0f;
			*vColor = pSprite->m_vColor * alpha * m_flGlowObstructionScale;
		}
		else
		{
			*flHorzSize = pSprite->m_flHorzSize;
			*flVertSize = pSprite->m_flVertSize;
			*vColor = pSprite->m_vColor * m_flGlowObstructionScale;
		}
	}

public:

	void SetModulateByDot( bool state = true )
	{
		m_bModulateByDot = state;
	}

protected:

	bool m_bModulateByDot;
};

//
// Sun entity
//
template<typename T= float>
void RecvProxy_SunHDRColorScale(const CRecvProxyData* pData, void* pStruct, void* pOut);

class RecvPropSunHDRColorScale : public RecvPropFloat {
public:
	RecvPropSunHDRColorScale() {}

	template<typename T = float>
	RecvPropSunHDRColorScale(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,	// Handled by RECVINFO macro, but set to SIZEOF_IGNORE if you don't want to bother.
		int flags = 0,
		RecvVarProxyFn varProxy = RecvProxy_SunHDRColorScale<T>
	);
	virtual	~RecvPropSunHDRColorScale() {}
	RecvPropSunHDRColorScale& operator=(const RecvPropSunHDRColorScale& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropSunHDRColorScale* pRecvProp = new RecvPropSunHDRColorScale;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

template<typename T>
RecvPropSunHDRColorScale::RecvPropSunHDRColorScale(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar,
	int flags,
	RecvVarProxyFn varProxy
):RecvPropFloat(pType, pVarName, offset, sizeofVar, flags, varProxy)
{
	
}

class C_Sun : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_Sun, C_BaseEntity );
	DECLARE_CLIENTCLASS();

					C_Sun();
					~C_Sun();

	virtual void	OnDataChanged( DataUpdateType_t updateType );

public:
	C_SunGlowOverlay	m_Overlay;
	C_SunGlowOverlay	m_GlowOverlay;
	
	CNetworkColor32(				m_clrOverlay);
	CNetworkVar( int,					m_nSize);
	CNetworkVar( int,					m_nOverlaySize);
	CNetworkVector(				m_vDirection);
	CNetworkVar( bool,				m_bOn);

	CNetworkVar( int,					m_nMaterial);
	CNetworkVar( int,					m_nOverlayMaterial);

public:
	BEGIN_INIT_RECV_TABLE(C_Sun)
	BEGIN_RECV_TABLE(C_Sun, DT_Sun, DT_BaseEntity)
		RecvPropColor32(RECVINFO(m_clrRender), 0),//, RecvProxy_IntToColor32
		RecvPropColor32(RECVINFO(m_clrOverlay), 0),//, RecvProxy_IntToColor32
		RecvPropVector(RECVINFO(m_vDirection)),
		RecvPropInt(RECVINFO(m_bOn)),
		RecvPropInt(RECVINFO(m_nSize)),
		RecvPropInt(RECVINFO(m_nOverlaySize)),
		RecvPropInt(RECVINFO(m_nMaterial)),
		RecvPropInt(RECVINFO(m_nOverlayMaterial)),
		RecvPropSunHDRColorScale((float*)0, "HDRColorScale", 0, SIZEOF_IGNORE, 0),//, RecvProxy_SunHDRColorScale

	END_RECV_TABLE(DT_Sun)
	END_INIT_RECV_TABLE()
};

template<typename T>
void RecvProxy_SunHDRColorScale(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	C_Sun* pSun = (C_Sun*)pStruct;

	pSun->m_Overlay.m_flHDRColorScale = pData->m_Value.m_Float;
	pSun->m_GlowOverlay.m_flHDRColorScale = pData->m_Value.m_Float;
}

#endif // C_SUN_H
