//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef FUNC_BREAKABLESURF_H
#define FUNC_BREAKABLESURF_H
#ifdef _WIN32
#pragma once
#endif


#define MAX_NUM_PANELS 16	//Must match client

#include "func_break.h"

//#############################################################################
//  > CWindowPane
//
//  A piece that falls out of the window
//#############################################################################
class CWindowPane : public CBaseAnimating
{
public:
	DECLARE_CLASS( CWindowPane, CBaseAnimating );

	static CWindowPane* CreateWindowPane(  const Vector &vecOrigin, const QAngle &vecAngles );

	void			Spawn( void );
	void			Precache( void );
	void			PaneTouch( CBaseEntity *pOther );
	void			Die( void );
	DECLARE_DATADESC();
};

//#############################################################################
//  > CBreakableSurface
//
//  A breakable surface
//#############################################################################
class CBreakableSurface : public CBreakable
{
	DECLARE_CLASS( CBreakableSurface, CBreakable );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
	DECLARE_SEND_TABLE_ACCESS(DT_BreakableSurface);

public:
	CNetworkVar( int, m_nNumWide );
	CNetworkVar( int, m_nNumHigh );
	CNetworkVar( float, m_flPanelWidth );
	CNetworkVar( float, m_flPanelHeight );
	CNetworkVector( m_vNormal );
	CNetworkVector( m_vCorner );
	CNetworkVar( bool, m_bIsBroken );
	CNetworkVar( ShatterSurface_t, m_nSurfaceType );
	int					m_nNumBrokenPanes;
	float				m_flSupport[MAX_NUM_PANELS][MAX_NUM_PANELS]; //UNDONE: allocate dynamically?

	int					m_nFragility;
	Vector				m_vLLVertex;
	Vector				m_vULVertex;
	Vector				m_vLRVertex;
	Vector				m_vURVertex;
	int					m_nQuadError;

	void			SurfaceTouch( CBaseEntity *pOther );
	void			PanePos(const Vector &vPos, float *flWidth, float *flHeight);

	bool			IsBroken(int nWidth, int nHeight);
	void			SetSupport(int w, int h, float support);

	float			GetSupport(int nWidth, int nHeight);
	float			RecalcSupport(int nWidth, int nHeight);

	void			BreakPane(int nWidth, int nHeight);
	void			DropPane(int nWidth, int nHeight);
	bool			ShatterPane(int nWidth, int nHeight, const Vector &force, const Vector &vForcePos);
	void			BreakAllPanes(void);

	void			CreateShards(const Vector &vBreakPos, const QAngle &vAngles,
								 const Vector &vForce,	  const Vector &vForcePos,
								 float flWidth,			  float flHeight,
								 int   nShardSize);

	void			Spawn(void);
	void			Precache(void);
	void			Die( CBaseEntity *pBreaker, const Vector &vAttackDir );
	void			BreakThink(void);
	void			Event_Killed( CBaseEntity *pInflictor, CBaseEntity *pAttacker, float flDamage, int bitsDamageType );
	void			TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator );
	int				OnTakeDamage( const CTakeDamageInfo &info );
	void			InputShatter( inputdata_t &inputdata );
	void			VPhysicsCollision( int index, gamevcollisionevent_t *pEvent );
private:
	// One bit per pane
	CNetworkArray( bool, m_RawPanelBitVec, MAX_NUM_PANELS * MAX_NUM_PANELS );

	BEGIN_SEND_TABLE(CBreakableSurface, DT_BreakableSurface, DT_BaseEntity)
		SendPropInt(SENDINFO(m_nNumWide), 8, SPROP_UNSIGNED),
		SendPropInt(SENDINFO(m_nNumHigh), 8, SPROP_UNSIGNED),
		SendPropFloat(SENDINFO(m_flPanelWidth), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_flPanelHeight), 0, SPROP_NOSCALE),
		SendPropVector(SENDINFO(m_vNormal), -1, SPROP_COORD),
		SendPropVector(SENDINFO(m_vCorner), -1, SPROP_COORD),
		SendPropInt(SENDINFO(m_bIsBroken), 1, SPROP_UNSIGNED),
		SendPropInt(SENDINFO(m_nSurfaceType), 2, SPROP_UNSIGNED),
		SendPropArray3(SENDINFO_ARRAY3(m_RawPanelBitVec), SendPropInt(SENDINFO_ARRAY(m_RawPanelBitVec), 1, SPROP_UNSIGNED)),
	END_SEND_TABLE(DT_BreakableSurface)
};

#endif // FUNC_BREAKABLESURF_H

