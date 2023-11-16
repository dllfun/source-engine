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
#include "IEffects.h"
#include "tier1/KeyValues.h"
#include "toolframework_client.h"
#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Smoke TE
//-----------------------------------------------------------------------------
class C_TESmoke : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TESmoke, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

					C_TESmoke( void );
	virtual			~C_TESmoke( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

public:
	CNetworkVector(			m_vecOrigin);
	CNetworkVar( int,				m_nModelIndex);
	CNetworkVar( float,			m_fScale);
	CNetworkVar( int,				m_nFrameRate);

public:
	BEGIN_INIT_RECV_TABLE(C_TESmoke)
	BEGIN_RECV_TABLE(C_TESmoke, DT_TESmoke, DT_BaseTempEntity)
		RecvPropVector(RECVINFO(m_vecOrigin)),
		RecvPropInt(RECVINFO(m_nModelIndex)),
		RecvPropFloat(RECVINFO(m_fScale)),
		RecvPropInt(RECVINFO(m_nFrameRate)),
	END_RECV_TABLE(DT_TESmoke)
	END_INIT_RECV_TABLE()
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TESmoke::C_TESmoke( void )
{
	m_vecOrigin.Init();
	m_nModelIndex = 0;
	m_fScale = 0;
	m_nFrameRate = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TESmoke::~C_TESmoke( void )
{
}


//-----------------------------------------------------------------------------
// Recording
//-----------------------------------------------------------------------------
static inline void RecordSmoke( const Vector &start, float flScale, int nFrameRate )
{
	if ( !ToolsEnabled() )
		return;

	if ( clienttools->IsInRecordingMode() )
	{
		KeyValues *msg = new KeyValues( "TempEntity" );

 		msg->SetInt( "te", TE_SMOKE );
 		msg->SetString( "name", "TE_Smoke" );
		msg->SetFloat( "time", gpGlobals->GetCurTime() );
		msg->SetFloat( "originx", start.x );
		msg->SetFloat( "originy", start.y );
		msg->SetFloat( "originz", start.z );
		msg->SetFloat( "scale", flScale );
		msg->SetInt( "framerate", nFrameRate );

		ToolFramework_PostToolMessage( HTOOLHANDLE_INVALID, msg );
		msg->deleteThis();
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TESmoke::PostDataUpdate( DataUpdateType_t updateType )
{
	VPROF( "C_TESmoke::PostDataUpdate" );

	// The number passed down is 10 times smaller...
	g_pEffects->Smoke( m_vecOrigin, m_nModelIndex, m_fScale * 10.0f, m_nFrameRate );
	RecordSmoke( m_vecOrigin, m_fScale * 10.0f, m_nFrameRate );
}

void TE_Smoke( IRecipientFilter& filter, float delay,
	const Vector* pos, int modelindex, float scale, int framerate )
{
	// The number passed down is 10 times smaller...
	g_pEffects->Smoke( *pos, modelindex, scale * 10.0f, framerate );
	RecordSmoke( *pos, scale * 10.0f, framerate );
}

IMPLEMENT_CLIENTCLASS_EVENT(C_TESmoke, DT_TESmoke, CTESmoke)
LINK_ENTITY_TO_CLASS(TESmoke, C_TESmoke);
PRECACHE_REGISTER(TESmoke);