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
void RecvProxy_SunHDRColorScale(const CRecvProxyData* pData, void* pStruct, void* pOut);

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
	
	color32				m_clrOverlay;
	int					m_nSize;
	int					m_nOverlaySize;
	Vector				m_vDirection;
	bool				m_bOn;

	int					m_nMaterial;
	int					m_nOverlayMaterial;

	BEGIN_RECV_TABLE_NOBASE(C_Sun, DT_Sun, CSun)
		RecvPropInt(RECVINFO(m_clrRender), 0, RecvProxy_IntToColor32),
		RecvPropInt(RECVINFO(m_clrOverlay), 0, RecvProxy_IntToColor32),
		RecvPropVector(RECVINFO(m_vDirection)),
		RecvPropInt(RECVINFO(m_bOn)),
		RecvPropInt(RECVINFO(m_nSize)),
		RecvPropInt(RECVINFO(m_nOverlaySize)),
		RecvPropInt(RECVINFO(m_nMaterial)),
		RecvPropInt(RECVINFO(m_nOverlayMaterial)),
		RecvPropFloat("HDRColorScale", 0, SIZEOF_IGNORE, 0, RecvProxy_SunHDRColorScale),

	END_RECV_TABLE(DT_Sun)
};


#endif // C_SUN_H
