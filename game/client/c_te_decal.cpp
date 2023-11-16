//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//===========================================================================//
#include "cbase.h"
#include "c_basetempentity.h"
#include "iefx.h"
#include "engine/IStaticPropMgr.h"
#include "tier1/KeyValues.h"
#include "tier0/vprof.h"
#include "toolframework_client.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Decal TE
//-----------------------------------------------------------------------------
class C_TEDecal : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEDecal, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

					C_TEDecal( void );
	virtual			~C_TEDecal( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

	virtual void	Precache( void );

public:
	CNetworkVector(			m_vecOrigin);
	CNetworkVector(			m_vecStart);
	CNetworkVar( int,				m_nEntity);
	CNetworkVar( int,				m_nHitbox);
	CNetworkVar( int,				m_nIndex);

public:
	BEGIN_INIT_RECV_TABLE(C_TEDecal)
	BEGIN_RECV_TABLE(C_TEDecal, DT_TEDecal, DT_BaseTempEntity)
		RecvPropVector(RECVINFO(m_vecOrigin)),
		RecvPropVector(RECVINFO(m_vecStart)),
		RecvPropInt(RECVINFO(m_nEntity)),
		RecvPropInt(RECVINFO(m_nHitbox)),
		RecvPropInt(RECVINFO(m_nIndex)),
	END_RECV_TABLE(DT_TEDecal)
	END_INIT_RECV_TABLE()
};


//-----------------------------------------------------------------------------
// Networking
//-----------------------------------------------------------------------------
IMPLEMENT_CLIENTCLASS_EVENT(C_TEDecal, DT_TEDecal, CTEDecal)
LINK_ENTITY_TO_CLASS(TEDecal, C_TEDecal);
PRECACHE_REGISTER(TEDecal);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEDecal::C_TEDecal( void )
{
	m_vecOrigin.Init();
	m_vecStart.Init();
	m_nEntity = 0;
	m_nIndex = 0;
	m_nHitbox = 0;

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEDecal::~C_TEDecal( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TEDecal::Precache( void )
{											 
}

//-----------------------------------------------------------------------------
// Recording 
//-----------------------------------------------------------------------------
static inline void RecordDecal( const Vector &pos, const Vector &start, 
	int entity, int hitbox, int index )
{
	if ( !ToolsEnabled() )
		return;

	if ( clienttools->IsInRecordingMode() )
	{
		// FIXME: Can't record on entities yet
		if ( entity != 0 )
			return;

		KeyValues *msg = new KeyValues( "TempEntity" );

 		msg->SetInt( "te", TE_DECAL );
 		msg->SetString( "name", "TE_Decal" );
		msg->SetFloat( "time", gpGlobals->GetCurTime() );
		msg->SetFloat( "originx", pos.x );
		msg->SetFloat( "originy", pos.y );
		msg->SetFloat( "originz", pos.z );
		msg->SetFloat( "startx", start.x );
		msg->SetFloat( "starty", start.y );
		msg->SetFloat( "startz", start.z );
		msg->SetInt( "hitbox", hitbox );
		msg->SetString( "decalname", effects->Draw_DecalNameFromIndex( index ) );

		ToolFramework_PostToolMessage( HTOOLHANDLE_INVALID, msg );
		
		msg->deleteThis();
	}
}


//-----------------------------------------------------------------------------
// Tempent 
//-----------------------------------------------------------------------------
void TE_Decal( IRecipientFilter& filter, float delay,
	const Vector* pos, const Vector* start, int entity, int hitbox, int index )
{
	RecordDecal( *pos, *start, entity, hitbox, index );

	trace_t tr;

	// Special case for world entity with hitbox:
	if ( (entity == 0) && (hitbox != 0) )
	{
		Ray_t ray;
		ray.Init( *start, *pos );
		staticpropmgr->AddDecalToStaticProp( *start, *pos, hitbox - 1, index, false, tr );
	}
	else
	{
		// Only decal the world + brush models
		// Here we deal with decals on entities.
		C_BaseEntity* ent;
		if ( ( ent = cl_entitylist->GetEnt( entity ) ) == NULL )
			return;

		ent->AddDecal( *start, *pos, *pos, hitbox, 
			index, false, tr );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TEDecal::PostDataUpdate( DataUpdateType_t updateType )
{
	VPROF( "C_TEDecal::PostDataUpdate" );

	CBroadcastRecipientFilter filter;
	TE_Decal( filter, 0.0f, &m_vecOrigin.m_Value, &m_vecStart.m_Value, m_nEntity, m_nHitbox, m_nIndex );
}


//-----------------------------------------------------------------------------
// Playback
//-----------------------------------------------------------------------------
void TE_Decal( IRecipientFilter& filter, float delay, KeyValues *pKeyValues )
{
	Vector vecOrigin, vecStart;
	vecOrigin.x = pKeyValues->GetFloat( "originx" );
	vecOrigin.y = pKeyValues->GetFloat( "originy" );
	vecOrigin.z = pKeyValues->GetFloat( "originz" );
	vecStart.x = pKeyValues->GetFloat( "startx" );
	vecStart.y = pKeyValues->GetFloat( "starty" );
	vecStart.z = pKeyValues->GetFloat( "startz" );
	int nHitbox = pKeyValues->GetInt( "hitbox" );
	const char *pDecalName = pKeyValues->GetString( "decalname" );

	TE_Decal( filter, 0.0f, &vecOrigin, &vecStart, 0, nHitbox, effects->Draw_DecalIndexFromName( (char*)pDecalName ) );
}
