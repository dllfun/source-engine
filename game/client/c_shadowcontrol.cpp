//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shadow control entity.
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//------------------------------------------------------------------------------
// FIXME: This really should inherit from something	more lightweight
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Purpose : Shadow control entity
//------------------------------------------------------------------------------
class C_ShadowControl : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_ShadowControl, C_BaseEntity );

	DECLARE_CLIENTCLASS();

	void OnDataChanged(DataUpdateType_t updateType);
	bool ShouldDraw();

private:
	CNetworkVector( m_shadowDirection);
	CNetworkColor32( m_shadowColor);
	CNetworkVar( float, m_flShadowMaxDist);
	CNetworkVar( bool, m_bDisableShadows);

public:
	BEGIN_INIT_RECV_TABLE(C_ShadowControl)
	BEGIN_RECV_TABLE(C_ShadowControl, DT_ShadowControl, DT_BaseEntity)
		RecvPropVector(RECVINFO(m_shadowDirection)),
		RecvPropColor32(RECVINFO(m_shadowColor)),
		RecvPropFloat(RECVINFO(m_flShadowMaxDist)),
		RecvPropBool(RECVINFO(m_bDisableShadows)),
	END_RECV_TABLE(DT_ShadowControl)
	END_INIT_RECV_TABLE()
};

IMPLEMENT_CLIENTCLASS(C_ShadowControl, DT_ShadowControl, CShadowControl)



//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void C_ShadowControl::OnDataChanged(DataUpdateType_t updateType)
{
	// Set the color, direction, distance...
	g_pClientShadowMgr->SetShadowDirection( m_shadowDirection );
	g_pClientShadowMgr->SetShadowColor( m_shadowColor.GetR(), m_shadowColor.GetG(), m_shadowColor.GetB());
	g_pClientShadowMgr->SetShadowDistance( m_flShadowMaxDist );
	g_pClientShadowMgr->SetShadowsDisabled( m_bDisableShadows );
}

//------------------------------------------------------------------------------
// We don't draw...
//------------------------------------------------------------------------------
bool C_ShadowControl::ShouldDraw()
{
	return false;
}

