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

//-----------------------------------------------------------------------------
// Purpose: Dispatches a beam ring between two entities
//-----------------------------------------------------------------------------
#if !defined( TE_BASEBEAM_H )
#define TE_BASEBEAM_H
#ifdef _WIN32
#pragma once
#endif

#include "basetempentity.h"

abstract_class CTEBaseBeam : public CBaseTempEntity
{
public:

	DECLARE_CLASS( CTEBaseBeam, CBaseTempEntity );
	DECLARE_SERVERCLASS();


public:
					CTEBaseBeam() {};
					CTEBaseBeam( const char *name );
	virtual			~CTEBaseBeam( void );

	virtual void	Test(const Vector& current_origin, const QAngle& current_angles) {};
	
public:
	CNetworkVar( int, m_nModelIndex );
	CNetworkVar( int, m_nHaloIndex );
	CNetworkVar( int, m_nStartFrame );
	CNetworkVar( int, m_nFrameRate );
	CNetworkVar( float, m_fLife );
	CNetworkVar( float, m_fWidth );
	CNetworkVar( float, m_fEndWidth );
	CNetworkVar( int, m_nFadeLength );
	CNetworkVar( float, m_fAmplitude );
	CNetworkVar( int, r );
	CNetworkVar( int, g );
	CNetworkVar( int, b );
	CNetworkVar( int, a );
	CNetworkVar( int, m_nSpeed );
	CNetworkVar( int, m_nFlags );

	BEGIN_SEND_TABLE_NOBASE(CTEBaseBeam, DT_BaseBeam)
		SendPropModelIndex(SENDINFO(m_nModelIndex)),
		SendPropModelIndex(SENDINFO(m_nHaloIndex)),
		SendPropInt(SENDINFO(m_nStartFrame), 8, SPROP_UNSIGNED),
		SendPropInt(SENDINFO(m_nFrameRate), 8, SPROP_UNSIGNED),
		SendPropFloat(SENDINFO(m_fLife), 8, 0, 0.0, 25.6),
		SendPropFloat(SENDINFO(m_fWidth), 10, 0, 0.0, 128.0),
		SendPropFloat(SENDINFO(m_fEndWidth), 10, 0, 0.0, 128.0),
		SendPropInt(SENDINFO(m_nFadeLength), 8, SPROP_UNSIGNED),
		SendPropFloat(SENDINFO(m_fAmplitude), 8, 0, 0.0, 64.0),
		SendPropInt(SENDINFO(m_nSpeed), 8, SPROP_UNSIGNED),
		SendPropInt(SENDINFO(r), 8, SPROP_UNSIGNED),
		SendPropInt(SENDINFO(g), 8, SPROP_UNSIGNED),
		SendPropInt(SENDINFO(b), 8, SPROP_UNSIGNED),
		SendPropInt(SENDINFO(a), 8, SPROP_UNSIGNED),
		SendPropInt(SENDINFO(m_nFlags), 32, SPROP_UNSIGNED),
	END_SEND_TABLE(DT_BaseBeam)
};

EXTERN_SEND_TABLE(DT_BaseBeam);

#endif // TE_BASEBEAM_H