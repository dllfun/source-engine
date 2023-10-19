//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// -------------------------------------------------------------------------------- //
// An entity used to access overlays (and change their texture)
// -------------------------------------------------------------------------------- //

class CInfoOverlayAccessor : public CPointEntity
{
public:

	DECLARE_CLASS( CInfoOverlayAccessor, CPointEntity );

	int  	UpdateTransmitState();

	DECLARE_SERVERCLASS();
	DECLARE_SEND_TABLE_ACCESS(DT_InfoOverlayAccessor);
	DECLARE_DATADESC();

private:

	CNetworkVar( int, m_iOverlayID );

	BEGIN_INIT_SEND_TABLE(CInfoOverlayAccessor)
	BEGIN_SEND_TABLE(CInfoOverlayAccessor, DT_InfoOverlayAccessor, DT_PointEntity)
		SendPropInt(SENDINFO(m_iTextureFrameIndex), 8, SPROP_UNSIGNED),
		SendPropInt(SENDINFO(m_iOverlayID), 32, SPROP_UNSIGNED),
	END_SEND_TABLE(DT_InfoOverlayAccessor)
	END_INIT_SEND_TABLE()
};
							  

// This table encodes the CBaseEntity data.
IMPLEMENT_SERVERCLASS(CInfoOverlayAccessor, DT_InfoOverlayAccessor)


LINK_ENTITY_TO_CLASS( info_overlay_accessor, CInfoOverlayAccessor );

BEGIN_DATADESC( CInfoOverlayAccessor )
	DEFINE_KEYFIELD( m_iOverlayID,	FIELD_INTEGER, "OverlayID" ),
END_DATADESC()


int CInfoOverlayAccessor::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}
