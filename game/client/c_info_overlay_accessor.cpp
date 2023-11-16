//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "materialsystem/imesh.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// -------------------------------------------------------------------------------- //
// An entity used to access overlays (and change their texture)
// -------------------------------------------------------------------------------- //
class C_InfoOverlayAccessor : public C_BaseEntity
{
public:

	DECLARE_CLASS( C_InfoOverlayAccessor, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	C_InfoOverlayAccessor();

	virtual void OnDataChanged( DataUpdateType_t updateType );

private:

	CNetworkVar( int,		m_iOverlayID);

public:
	BEGIN_INIT_RECV_TABLE(C_InfoOverlayAccessor)
	BEGIN_RECV_TABLE(C_InfoOverlayAccessor, DT_InfoOverlayAccessor, DT_BaseEntity)
		RecvPropInt(RECVINFO(m_iTextureFrameIndex)),
		RecvPropInt(RECVINFO(m_iOverlayID)),
	END_RECV_TABLE(DT_InfoOverlayAccessor)
	END_INIT_RECV_TABLE()
};

// Expose it to the engine.
IMPLEMENT_CLIENTCLASS(C_InfoOverlayAccessor, DT_InfoOverlayAccessor, CInfoOverlayAccessor);




// -------------------------------------------------------------------------------- //
// Functions.
// -------------------------------------------------------------------------------- //

C_InfoOverlayAccessor::C_InfoOverlayAccessor()
{
}

void C_InfoOverlayAccessor::OnDataChanged( DataUpdateType_t updateType )
{
	if ( updateType == DATA_UPDATE_CREATED )
	{
		// Update overlay's bind proxy
		engineClient->SetOverlayBindProxy( m_iOverlayID, GetClientRenderable() );
	}
}
