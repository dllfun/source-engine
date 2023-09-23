//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The worldspawn entity. This spawns first when each level begins.
//
// $NoKeywords: $
//=============================================================================//

#ifndef WORLD_H
#define WORLD_H
#ifdef _WIN32
#pragma once
#endif


class CWorld : public CBaseEntity
{
public:
	DECLARE_CLASS( CWorld, CBaseEntity );

	CWorld();
	~CWorld();

	DECLARE_SERVERCLASS();
	DECLARE_SEND_TABLE_ACCESS(DT_WORLD);

	virtual int RequiredEdictIndex( void ) { return 0; }   // the world always needs to be in slot 0
	
	static void RegisterSharedActivities( void );
	static void RegisterSharedEvents( void );
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual bool KeyValue( const char *szKeyName, const char *szValue );
	virtual void DecalTrace( trace_t *pTrace, char const *decalName );
	virtual void VPhysicsCollision( int index, gamevcollisionevent_t *pEvent ) {}
	virtual void VPhysicsFriction( IPhysicsObject *pObject, float energy, int surfaceProps, int surfacePropsHit ) {}

	inline void GetWorldBounds( Vector &vecMins, Vector &vecMaxs )
	{
		VectorCopy( m_WorldMins, vecMins );
		VectorCopy( m_WorldMaxs, vecMaxs );
	}

	inline float GetWaveHeight() const
	{
		return (float)m_flWaveHeight;
	}

	bool GetDisplayTitle() const;
	bool GetStartDark() const;

	void SetDisplayTitle( bool display );
	void SetStartDark( bool startdark );

	bool IsColdWorld( void );

private:
	DECLARE_DATADESC();

	string_t m_iszChapterTitle;

	CNetworkVar( float, m_flWaveHeight );
	CNetworkVector( m_WorldMins );
	CNetworkVector( m_WorldMaxs );
	CNetworkVar( float, m_flMaxOccludeeArea );
	CNetworkVar( float, m_flMinOccluderArea );
	CNetworkVar( float, m_flMinPropScreenSpaceWidth );
	CNetworkVar( float, m_flMaxPropScreenSpaceWidth );
	CNetworkVar( string_t, m_iszDetailSpriteMaterial );

	// start flags
	CNetworkVar( bool, m_bStartDark );
	CNetworkVar( bool, m_bColdWorld );
	bool m_bDisplayTitle;

	BEGIN_INIT_SEND_TABLE(CWorld)
	BEGIN_SEND_TABLE(CWorld, DT_WORLD, DT_BaseEntity)
		SendPropFloat(SENDINFO(m_flWaveHeight), 8, SPROP_ROUNDUP, 0.0f, 8.0f),
		SendPropVector(SENDINFO(m_WorldMins), -1, SPROP_COORD),
		SendPropVector(SENDINFO(m_WorldMaxs), -1, SPROP_COORD),
		SendPropInt(SENDINFO(m_bStartDark), 1, SPROP_UNSIGNED),
		SendPropFloat(SENDINFO(m_flMaxOccludeeArea), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_flMinOccluderArea), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_flMaxPropScreenSpaceWidth), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_flMinPropScreenSpaceWidth), 0, SPROP_NOSCALE),
		SendPropStringT(SENDINFO(m_iszDetailSpriteMaterial)),
		SendPropInt(SENDINFO(m_bColdWorld), 1, SPROP_UNSIGNED),
	END_SEND_TABLE(DT_WORLD)
	END_INIT_SEND_TABLE()
};


CWorld* GetWorldEntity();
extern const char *GetDefaultLightstyleString( int styleIndex );


#endif // WORLD_H
