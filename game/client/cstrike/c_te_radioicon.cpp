//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_basetempentity.h"
#include "c_cs_player.h"
#include "hud_radar.h"
#include "radio_status.h"



//-----------------------------------------------------------------------------
// Purpose: Kills Player Attachments
//-----------------------------------------------------------------------------
class C_TERadioIcon : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TERadioIcon, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

					C_TERadioIcon( void );
	virtual			~C_TERadioIcon( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

public:
	int m_iAttachToClient;

public:
	BEGIN_INIT_RECV_TABLE(C_TERadioIcon)
	BEGIN_RECV_TABLE(C_TERadioIcon, DT_TERadioIcon, DT_BaseTempEntity)
		RecvPropInt(RECVINFO(m_iAttachToClient)),
	END_RECV_TABLE(DT_TERadioIcon)
	END_INIT_RECV_TABLE()
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TERadioIcon::C_TERadioIcon( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TERadioIcon::~C_TERadioIcon( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_TERadioIcon::PostDataUpdate( DataUpdateType_t updateType )
{
	//Flash them on the radar
	//this could be in a better place.
	C_CSPlayer *pPlayer = static_cast<C_CSPlayer*>( cl_entitylist->GetEnt(m_iAttachToClient) );

	if ( pPlayer && !pPlayer->IsDormant() )
	{
		// Create the flashy above player's head
		RadioManager()->UpdateRadioStatus( m_iAttachToClient, 1.5f );
	}
	
	Radar_FlashPlayer( m_iAttachToClient );
}

IMPLEMENT_CLIENTCLASS_EVENT(C_TERadioIcon, DT_TERadioIcon, CTERadioIcon)
LINK_ENTITY_TO_CLASS(TERadioIcon, C_TERadioIcon);
PRECACHE_REGISTER(TERadioIcon);