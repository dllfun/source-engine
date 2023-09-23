//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef	__PLASMA__
#define __PLASMA__
#pragma once

#include "fire_smoke.h"

//==================================================
// CPlasma
//==================================================

//NOTENOTE: Mirrored in cl_dll/c_plasma.cpp
#define	bitsPLASMA_FREE		0x00000002

class CPlasma : public CBaseFire
{
public:
	DECLARE_CLASS( CPlasma, CBaseFire );

	CPlasma( void );
	virtual	~CPlasma( void );
	void	EnableSmoke( int state );

	void	Precache( void );

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

public:

	//Client-side
	CNetworkVar( int, m_nPlasmaModelIndex );
	CNetworkVar( int, m_nPlasmaModelIndex2 );
	CNetworkVar( int, m_nGlowModelIndex );

	//Server-side

	BEGIN_INIT_SEND_TABLE(CPlasma)
	BEGIN_SEND_TABLE(CPlasma, DT_Plasma, DT_BaseEntity)
		SendPropFloat(SENDINFO(m_flScale), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_flScaleTime), 0, SPROP_NOSCALE),
		SendPropInt(SENDINFO(m_nFlags), 8, SPROP_UNSIGNED),
		SendPropModelIndex(SENDINFO(m_nPlasmaModelIndex)),
		SendPropModelIndex(SENDINFO(m_nPlasmaModelIndex2)),
		SendPropModelIndex(SENDINFO(m_nGlowModelIndex)),
	END_SEND_TABLE(DT_Plasma)
	END_INIT_SEND_TABLE()
};

#endif	//__PLASMA__
