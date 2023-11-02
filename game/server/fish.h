//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
// fish.h
// Simple fish behavior
// Author: Michael S. Booth, April 2005

#ifndef _FISH_H_
#define _FISH_H_

#include "baseanimating.h"
#include "GameEventListener.h"

class CFishPool;

void SendProxy_FishAngle(const SendProp* pProp, const void* pStruct, const void* pData, DVariant* pOut, int iElement, int objectID);
//----------------------------------------------------------------------------------------------
/**
 * Simple ambient fish
 */
class CFish : public CBaseAnimating
{
public:
	DECLARE_CLASS( CFish, CBaseAnimating );
	DECLARE_SERVERCLASS();
	DECLARE_SEND_TABLE_ACCESS(DT_CFish);
	DECLARE_DATADESC();

	CFish( void );
	virtual ~CFish();

	void Initialize( CFishPool *pool, unsigned int id );
	
	virtual void Spawn( void );

	virtual void Event_Killed( const CTakeDamageInfo &info );
	virtual void Touch( CBaseEntity *other );			///< in contact with "other"

	void Update( float deltaT );						///< invoked each server tick

	void FlockTo( CFish *other, float amount );			///< influence my motion to flock with other nearby fish
	float Avoid( void );
	void Panic( void );									///< panic for awhile

	void ResetVisible( void );							///< zero the visible vector
	void AddVisible( CFish *fish );						///< add this fish to our visible vector

private:
	friend void SendProxy_FishOriginX( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID );
	friend void SendProxy_FishOriginY( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID );

	CHandle<CFishPool> m_pool;							///< the pool we are in
	unsigned int m_id;									///< our unique ID

	CNetworkVar( float, m_x );							///< have to send position coordinates separately since Z is unused
	CNetworkVar( float, m_y );							///< have to send position coordinates separately since Z is unused
	CNetworkVar( float, m_z );							///< only sent once since fish always swim at the same depth

	CNetworkVar( float, m_angle );						///< only yaw changes
	float m_angleChange;
	Vector m_forward;
	Vector m_perp;

	CNetworkVector( m_poolOrigin );				///< used to efficiently network our relative position
	CNetworkVar( float, m_waterLevel );

	float m_speed;
	float m_desiredSpeed;

	float m_calmSpeed;									///< speed the fish moves when calm
	float m_panicSpeed;									///< speed the fish moves when panicked

	float m_avoidRange;									///< range to avoid obstacles

	CountdownTimer m_turnTimer;							///< every so often our turn preference changes
	bool m_turnClockwise;								///< if true this fish prefers to turn clockwise, else CCW
	
	CountdownTimer m_goTimer;							///< start the fish moving when timer elapses
	CountdownTimer m_moveTimer;							///< dont decay speed while we are moving
	CountdownTimer m_panicTimer;						///< if active, fish is panicked
	CountdownTimer m_disperseTimer;						///< initial non-flocking time

	CUtlVector< CFish * > m_visible;					///< vector of fish that we can see

	BEGIN_INIT_SEND_TABLE(CFish)
	BEGIN_SEND_TABLE(CFish, DT_CFish, DT_BaseAnimating)

		SendPropVector(SENDINFO(m_poolOrigin), -1, SPROP_COORD, 0.0f, HIGH_DEFAULT),	// only sent once

		SendPropFloat(SENDINFO(m_angle), 7, 0 /*SPROP_CHANGES_OFTEN*/, 0.0f, 360.0f, SendProxy_FishAngle),

		SendPropFloat(SENDINFO(m_x), 7, 0 /*SPROP_CHANGES_OFTEN*/, -255.0f, 255.0f),
		SendPropFloat(SENDINFO(m_y), 7, 0 /*SPROP_CHANGES_OFTEN*/, -255.0f, 255.0f),
		SendPropFloat(SENDINFO(m_z), -1, SPROP_COORD),								// only sent once

		SendPropModelIndex(SENDINFO(m_nModelIndex)),
		SendPropInt(SENDINFO(m_lifeState)),

		SendPropFloat(SENDINFO(m_waterLevel)),										// only sent once

	END_SEND_TABLE(DT_CFish)
	END_INIT_SEND_TABLE()
};


//----------------------------------------------------------------------------------------------
/**
 * This class defines a volume of water where a number of CFish swim
 */
class CFishPool : public CBaseEntity, public CGameEventListener
{
public:
	DECLARE_CLASS( CFishPool, CBaseEntity );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
	CFishPool( void );

	virtual void Spawn();

	virtual bool KeyValue( const char *szKeyName, const char *szValue );

	virtual void FireGameEvent( IGameEvent *event );

	void Update( void );					///< invoked each server tick

	float GetWaterLevel( void ) const;		///< return Z coordinate of water in world coords
	float GetMaxRange( void ) const;		///< return how far a fish is allowed to wander

private:
	int m_fishCount;						///< number of fish in the pool
	float m_maxRange;						///< how far a fish is allowed to wander
	float m_swimDepth;						///< the depth the fish swim below the water surface

	float m_waterLevel;						///< Z of water surface

	bool m_isDormant;

	CUtlVector< CHandle<CFish> > m_fishes;	///< vector of all fish in this pool

	CountdownTimer m_visTimer;				///< for throttling line of sight checks between all fish

public:
	BEGIN_INIT_SEND_TABLE(CFishPool)
	BEGIN_SEND_TABLE(CFishPool, DT_FishPool, DT_BaseEntity)

	END_SEND_TABLE(DT_FishPool)
	END_INIT_SEND_TABLE()
};


inline float CFishPool::GetMaxRange( void ) const
{
	return m_maxRange;
}


inline float CFishPool::GetWaterLevel( void ) const
{
	return m_waterLevel;
}


#endif // _FISH_H_

