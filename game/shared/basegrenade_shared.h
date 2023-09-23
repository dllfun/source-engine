//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef BASEGRENADE_SHARED_H
#define BASEGRENADE_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#include "baseprojectile.h"

#if defined( CLIENT_DLL )

#define CBaseGrenade C_BaseGrenade

#include "c_basecombatcharacter.h"

#else

#include "basecombatcharacter.h"
#include "player_pickup.h"

#endif

#define BASEGRENADE_EXPLOSION_VOLUME	1024

class CTakeDamageInfo;

#if !defined( CLIENT_DLL )
class CBaseGrenade : public CBaseProjectile, public CDefaultPlayerPickupVPhysics
#else
class CBaseGrenade : public CBaseProjectile
#endif
{
	DECLARE_CLASS( CBaseGrenade, CBaseProjectile );
public:

	CBaseGrenade(void);
	~CBaseGrenade(void);

	DECLARE_PREDICTABLE();
	DECLARE_NETWORKCLASS();


#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();
	DECLARE_SEND_TABLE_ACCESS(DT_BaseGrenade);
#endif

	virtual void		Precache( void );

	virtual void		Explode( trace_t *pTrace, int bitsDamageType );
	void				Smoke( void );

	void				BounceTouch( CBaseEntity *pOther );
	void				SlideTouch( CBaseEntity *pOther );
	void				ExplodeTouch( CBaseEntity *pOther );
	void				DangerSoundThink( void );
	void				PreDetonate( void );
	virtual void		Detonate( void );
	void				DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void				TumbleThink( void );

	virtual Vector		GetBlastForce() { return vec3_origin; }

	virtual void		BounceSound( void );
	virtual int			BloodColor( void ) { return DONT_BLEED; }
	virtual void		Event_Killed( const CTakeDamageInfo &info );

	virtual float		GetShakeAmplitude( void ) { return 25.0; }
	virtual float		GetShakeRadius( void ) { return 750.0; }

	// Damage accessors.
	virtual float GetDamage()
	{
		return m_flDamage;
	}
	virtual float GetDamageRadius()
	{
		return m_DmgRadius;
	}

	virtual void SetDamage(float flDamage)
	{
		m_flDamage = flDamage;
	}

	virtual void SetDamageRadius(float flDamageRadius)
	{
		m_DmgRadius = flDamageRadius;
	}

	// Bounce sound accessors.
	void SetBounceSound( const char *pszBounceSound ) 
	{
		m_iszBounceSound = MAKE_STRING( pszBounceSound );
	}

	CBaseCombatCharacter *GetThrower( void );
	void				  SetThrower( CBaseCombatCharacter *pThrower );
	CBaseEntity *GetOriginalThrower() { return m_hOriginalThrower; }

#if !defined( CLIENT_DLL )
	// Allow +USE pickup
	int ObjectCaps() 
	{ 
		return (BaseClass::ObjectCaps() | FCAP_IMPULSE_USE | FCAP_USE_IN_RADIUS);
	}

	void				Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
#endif

public:
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_vecVelocity );
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_fFlags );
	
	bool				m_bHasWarnedAI;				// whether or not this grenade has issued its DANGER sound to the world sound list yet.
	CNetworkVar( bool, m_bIsLive );					// Is this grenade live, or can it be picked up?
	CNetworkVar( float, m_DmgRadius );				// How far do I do damage?
	CNetworkVar( float, m_flNextAttack );
	float				m_flDetonateTime;			// Time at which to detonate.
	float				m_flWarnAITime;				// Time at which to warn the AI

protected:

	CNetworkVar( float, m_flDamage );		// Damage to inflict.
	string_t m_iszBounceSound;	// The sound to make on bouncing.  If not NULL, overrides the BounceSound() function.

private:
	CNetworkHandle( CBaseEntity, m_hThrower );					// Who threw this grenade
	EHANDLE			m_hOriginalThrower;							// Who was the original thrower of this grenade

	CBaseGrenade( const CBaseGrenade & ); // not defined, not accessible

#if !defined( CLIENT_DLL )
	BEGIN_INIT_SEND_TABLE(CBaseGrenade)
	BEGIN_NETWORK_TABLE(CBaseGrenade, DT_BaseGrenade, DT_BaseProjectile)
		SendPropFloat(SENDINFO(m_flDamage), 10, SPROP_ROUNDDOWN, 0.0, 256.0f),
		SendPropFloat(SENDINFO(m_DmgRadius), 10, SPROP_ROUNDDOWN, 0.0, 1024.0f),
		SendPropInt(SENDINFO(m_bIsLive), 1, SPROP_UNSIGNED),
		//	SendPropTime( SENDINFO( m_flDetonateTime ) ),
		SendPropEHandle(SENDINFO(m_hThrower)),

		SendPropVector(SENDINFO(m_vecVelocity), 0, SPROP_NOSCALE),
		// HACK: Use same flag bits as player for now
		SendPropInt(SENDINFO(m_fFlags), PLAYER_FLAG_BITS, SPROP_UNSIGNED, SendProxy_CropFlagsToPlayerFlagBitsLength),
	END_NETWORK_TABLE(DT_BaseGrenade)
	END_INIT_SEND_TABLE()
#endif

#if defined( CLIENT_DLL )
	BEGIN_INIT_RECV_TABLE(CBaseGrenade)
	BEGIN_NETWORK_TABLE(CBaseGrenade, DT_BaseGrenade, DT_BaseProjectile)
		RecvPropFloat(RECVINFO(m_flDamage)),
		RecvPropFloat(RECVINFO(m_DmgRadius)),
		RecvPropInt(RECVINFO(m_bIsLive)),
		//	RecvPropTime( RECVINFO( m_flDetonateTime ) ),
		RecvPropEHandle(RECVINFO(m_hThrower)),

		// Need velocity from grenades to make animation system work correctly when running
		RecvPropVector(RECVINFO(m_vecVelocity), 0, RecvProxy_LocalVelocity),

		RecvPropInt(RECVINFO(m_fFlags)),
	END_NETWORK_TABLE(DT_BaseGrenade)
	END_INIT_RECV_TABLE()
#endif
};

#endif // BASEGRENADE_SHARED_H
