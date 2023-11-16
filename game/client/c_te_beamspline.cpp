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
#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define MAX_SPLINE_POINTS 16
//-----------------------------------------------------------------------------
// Purpose: BeamSpline TE
//-----------------------------------------------------------------------------
class C_TEBeamSpline : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEBeamSpline, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

					C_TEBeamSpline( void );
	virtual			~C_TEBeamSpline( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

public:
	CNetworkArray( Vector,			m_vecPoints, MAX_SPLINE_POINTS );
	CNetworkVar( int,				m_nPoints);

public:
	BEGIN_INIT_RECV_TABLE(C_TEBeamSpline)
	BEGIN_RECV_TABLE(C_TEBeamSpline, DT_TEBeamSpline, DT_BaseTempEntity)
		RecvPropInt(RECVINFO(m_nPoints)),
		RecvPropInternalArray(RECVINFO_INTERNALARRAY(m_vecPoints), RecvPropVector(RECVINFO_ARRAY3(m_vecPoints)))
	END_RECV_TABLE(DT_TEBeamSpline)
	END_INIT_RECV_TABLE()
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEBeamSpline::C_TEBeamSpline( void )
{
	int i;
	for ( i = 0; i < MAX_SPLINE_POINTS; i++ )
	{
		m_vecPoints[ i ].Init();
	}
	m_nPoints = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEBeamSpline::~C_TEBeamSpline( void )
{
}

void TE_BeamSpline( IRecipientFilter& filter, float delay,
	int points, Vector* rgPoints )
{
	DevMsg( 1, "Beam spline with %i points invoked\n", points );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_TEBeamSpline::PostDataUpdate( DataUpdateType_t updateType )
{
	VPROF( "C_TEBeamSpline::PostDataUpdate" );

	DevMsg( 1, "Beam spline with %i points received\n", m_nPoints );
}

// Expose the TE to the engine.
IMPLEMENT_CLIENTCLASS_EVENT(C_TEBeamSpline, DT_TEBeamSpline, CTEBeamSpline );
LINK_ENTITY_TO_CLASS(TEBeamSpline, C_TEBeamSpline);
PRECACHE_REGISTER(TEBeamSpline);

