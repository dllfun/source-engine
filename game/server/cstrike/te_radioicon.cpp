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
#include "basetempentity.h"

//-----------------------------------------------------------------------------
// Purpose: Dispatches blood stream tempentity
//-----------------------------------------------------------------------------
class CTERadioIcon : public CBaseTempEntity
{
public:
	DECLARE_CLASS( CTERadioIcon, CBaseTempEntity );
					CTERadioIcon() {};
					CTERadioIcon( const char *name );
	virtual			~CTERadioIcon( void );

	void Precache( void );
	
	DECLARE_SERVERCLASS();

public:

	CNetworkVar( int, m_iAttachToClient );

	BEGIN_INIT_SEND_TABLE(CTERadioIcon)
	BEGIN_SEND_TABLE(CTERadioIcon, DT_TERadioIcon, DT_BaseTempEntity)
		SendPropInt(SENDINFO(m_iAttachToClient), 8, SPROP_UNSIGNED),
	END_SEND_TABLE(DT_TERadioIcon)
	END_INIT_SEND_TABLE()
};

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *name - 
//-----------------------------------------------------------------------------
CTERadioIcon::CTERadioIcon( const char *name ) :
	CBaseTempEntity( name )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTERadioIcon::~CTERadioIcon( void )
{
}

void CTERadioIcon::Precache( void )
{
	CBaseEntity::PrecacheModel("sprites/radio.vmt");
}

IMPLEMENT_SERVERCLASS(CTERadioIcon, DT_TERadioIcon, DT_BaseTempEntity)
LINK_ENTITY_TO_CLASS(TERadioIcon, CTERadioIcon);
PRECACHE_REGISTER(TERadioIcon);

// Singleton to fire StickyBolt objects
static CTERadioIcon g_TERadioIcon( "RadioIcon" );

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : filter - 
//			delay - 
//			pPlayer - 
//-----------------------------------------------------------------------------
void TE_RadioIcon( IRecipientFilter& filter, float delay, CBaseEntity *pPlayer )
{
	g_TERadioIcon.m_iAttachToClient = pPlayer->NetworkProp()->entindex();
	
	// Send it over the wire
	g_TERadioIcon.Create( filter, delay );
}
