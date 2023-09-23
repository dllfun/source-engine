//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_basetempentity.h"
#include "c_te_legacytempents.h"
#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Kills Player Attachments
//-----------------------------------------------------------------------------
class C_TEKillPlayerAttachments : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEKillPlayerAttachments, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

					C_TEKillPlayerAttachments( void );
	virtual			~C_TEKillPlayerAttachments( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

public:
	int				m_nPlayer;

public:
	BEGIN_INIT_RECV_TABLE(C_TEKillPlayerAttachments)
	BEGIN_RECV_TABLE(C_TEKillPlayerAttachments, DT_TEKillPlayerAttachments, DT_BaseTempEntity)
		RecvPropInt(RECVINFO(m_nPlayer)),
	END_RECV_TABLE(DT_TEKillPlayerAttachments)
	END_INIT_RECV_TABLE()
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEKillPlayerAttachments::C_TEKillPlayerAttachments( void )
{
	m_nPlayer = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEKillPlayerAttachments::~C_TEKillPlayerAttachments( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_TEKillPlayerAttachments::PostDataUpdate( DataUpdateType_t updateType )
{
	VPROF( "C_TEKillPlayerAttachments::PostDataUpdate" );

	tempents->KillAttachedTents( m_nPlayer );
}

void TE_KillPlayerAttachments( IRecipientFilter& filter, float delay,
	int player )
{
	tempents->KillAttachedTents( player );
}

IMPLEMENT_CLIENTCLASS_EVENT(C_TEKillPlayerAttachments, DT_TEKillPlayerAttachments, CTEKillPlayerAttachments)
LINK_ENTITY_TO_CLASS(TEKillPlayerAttachments, C_TEKillPlayerAttachments);
PRECACHE_REGISTER(TEKillPlayerAttachments);