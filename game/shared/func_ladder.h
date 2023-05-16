//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef FUNC_LADDER_H
#define FUNC_LADDER_H
#ifdef _WIN32
#pragma once
#endif

#if defined( CLIENT_DLL )
#define CFuncLadder C_FuncLadder
#define CInfoLadderDismount C_InfoLadderDismount
#endif

class CInfoLadderDismount : public CBaseEntity
{
public:
	DECLARE_CLASS( CInfoLadderDismount, CBaseEntity );
	DECLARE_NETWORKCLASS();

	virtual void DrawDebugGeometryOverlays();

#ifndef CLIENT_DLL
	BEGIN_NETWORK_TABLE(CInfoLadderDismount, DT_InfoLadderDismount, DT_BaseEntity)
	END_NETWORK_TABLE(DT_InfoLadderDismount)
#endif

#ifdef CLIENT_DLL
	BEGIN_NETWORK_TABLE(CInfoLadderDismount, DT_InfoLadderDismount, DT_BaseEntity)
	END_NETWORK_TABLE(DT_InfoLadderDismount)
#endif
};

typedef CHandle< CInfoLadderDismount > CInfoLadderDismountHandle;

// Spawnflags
#define SF_LADDER_DONTGETON			1			// Set for ladders that are acting as automount points, but not really ladders

//-----------------------------------------------------------------------------
// Purpose: A player-climbable ladder
//-----------------------------------------------------------------------------
class CFuncLadder : public CBaseEntity
{
public:

	DECLARE_CLASS( CFuncLadder, CBaseEntity );
	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();
#ifndef CLIENT_DLL
	DECLARE_SEND_TABLE_ACCESS(DT_FuncLadder);
#endif

	CFuncLadder();
	~CFuncLadder();

	virtual void Spawn();

	virtual void DrawDebugGeometryOverlays(void);

	int					GetDismountCount() const;
	CInfoLadderDismount	*GetDismount( int index );

	void	GetTopPosition( Vector& org );
	void	GetBottomPosition( Vector& org );
	void	ComputeLadderDir( Vector& bottomToTopVec );

	void	SetEndPoints( const Vector& p1, const Vector& p2 );

	void	InputEnable( inputdata_t &inputdata );
	void	InputDisable( inputdata_t &inputdata );

	bool	IsEnabled() const;

	void	PlayerGotOn( CBasePlayer *pPlayer );
	void	PlayerGotOff( CBasePlayer *pPlayer );

	virtual void Activate();

	bool	DontGetOnLadder( void ) const;

	static int GetLadderCount();
	static CFuncLadder *GetLadder( int index );
	static CUtlVector< CFuncLadder * >	s_Ladders;
public:

	void FindNearbyDismountPoints( const Vector& origin, float radius, CUtlVector< CInfoLadderDismountHandle >& list );
	const char *GetSurfacePropName();

private:


	void	SearchForDismountPoints();

	// Movement vector from "bottom" to "top" of ladder
	CNetworkVector( m_vecLadderDir );

	// Dismount points near top/bottom of ladder, precomputed
	CUtlVector< CInfoLadderDismountHandle > m_Dismounts;

	// Endpoints for checking for mount/dismount
	CNetworkVector( m_vecPlayerMountPositionTop );
	CNetworkVector( m_vecPlayerMountPositionBottom );

	bool		m_bDisabled;
	CNetworkVar( bool,	m_bFakeLadder );

#if defined( GAME_DLL )
	string_t	m_surfacePropName;
	//-----------------------------------------------------
	//	Outputs
	//-----------------------------------------------------
	COutputEvent	m_OnPlayerGotOnLadder;
	COutputEvent	m_OnPlayerGotOffLadder;

	virtual int UpdateTransmitState();
#endif

#if !defined( CLIENT_DLL )
	BEGIN_NETWORK_TABLE(CFuncLadder, DT_FuncLadder, DT_BaseEntity)
		SendPropVector(SENDINFO(m_vecPlayerMountPositionTop), SPROP_COORD),
		SendPropVector(SENDINFO(m_vecPlayerMountPositionBottom), SPROP_COORD),
		SendPropVector(SENDINFO(m_vecLadderDir), SPROP_COORD),
		SendPropBool(SENDINFO(m_bFakeLadder)),
		//	SendPropStringT( SENDINFO(m_surfacePropName) ),
	END_NETWORK_TABLE(DT_FuncLadder)
#endif

#if defined( CLIENT_DLL )
	BEGIN_NETWORK_TABLE(CFuncLadder, DT_FuncLadder, DT_BaseEntity)
		RecvPropVector(RECVINFO(m_vecPlayerMountPositionTop)),
		RecvPropVector(RECVINFO(m_vecPlayerMountPositionBottom)),
		RecvPropVector(RECVINFO(m_vecLadderDir)),
		RecvPropBool(RECVINFO(m_bFakeLadder)),
	END_NETWORK_TABLE(DT_FuncLadder)
#endif
};

inline bool CFuncLadder::IsEnabled() const
{
	return !m_bDisabled;
}

const char *FuncLadder_GetSurfaceprops(CBaseEntity *pLadderEntity);

#endif // FUNC_LADDER_H
