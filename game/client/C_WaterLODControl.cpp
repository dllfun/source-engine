//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Water LOD control entity.
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "iviewrender.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//------------------------------------------------------------------------------
// FIXME: This really should inherit from something	more lightweight
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Purpose : Water LOD control entity
//------------------------------------------------------------------------------
class C_WaterLODControl : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_WaterLODControl, C_BaseEntity );

	DECLARE_CLIENTCLASS();

	void OnDataChanged(DataUpdateType_t updateType);
	bool ShouldDraw();

private:
	CNetworkVar( float, m_flCheapWaterStartDistance);
	CNetworkVar( float, m_flCheapWaterEndDistance);

public:
	BEGIN_INIT_RECV_TABLE(C_WaterLODControl)
	BEGIN_RECV_TABLE(C_WaterLODControl, DT_WaterLODControl, DT_BaseEntity)
		RecvPropFloat(RECVINFO(m_flCheapWaterStartDistance)),
		RecvPropFloat(RECVINFO(m_flCheapWaterEndDistance)),
	END_RECV_TABLE(DT_WaterLODControl)
	END_INIT_RECV_TABLE()
};

IMPLEMENT_CLIENTCLASS(C_WaterLODControl, DT_WaterLODControl, CWaterLODControl)



//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void C_WaterLODControl::OnDataChanged(DataUpdateType_t updateType)
{
	g_pView->SetCheapWaterStartDistance( m_flCheapWaterStartDistance );
	g_pView->SetCheapWaterEndDistance( m_flCheapWaterEndDistance );
}

//------------------------------------------------------------------------------
// We don't draw...
//------------------------------------------------------------------------------
bool C_WaterLODControl::ShouldDraw()
{
	return false;
}

