//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_CSBASE_GUN_H
#define WEAPON_CSBASE_GUN_H
#ifdef _WIN32
#pragma once
#endif


#include "weapon_csbase.h"


// This is the base class for pistols and rifles.
#if defined( CLIENT_DLL )

	#define CWeaponCSBaseGun C_WeaponCSBaseGun

#else
#endif


class CWeaponCSBaseGun : public CWeaponCSBase
{
public:
	
	DECLARE_CLASS( CWeaponCSBaseGun, CWeaponCSBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponCSBaseGun();

	virtual void PrimaryAttack();
	virtual void Spawn();
	virtual bool Deploy();
	virtual bool Reload();
	virtual void WeaponIdle();

	// Derived classes call this to fire a bullet.
	bool CSBaseGunFire( float flCycleTime, CSWeaponMode weaponMode );

	// Usually plays the shot sound. Guns with silencers can play different sounds.
	virtual void DoFireEffects();
	virtual void ItemPostFrame();

protected: 
	float m_zoomFullyActiveTime;

private:

	CWeaponCSBaseGun( const CWeaponCSBaseGun & );

public:
#if !defined(CLIENT_DLL)
	BEGIN_INIT_SEND_TABLE(CWeaponCSBaseGun)
	BEGIN_NETWORK_TABLE(CWeaponCSBaseGun, DT_WeaponCSBaseGun, DT_WeaponCSBase)

	END_NETWORK_TABLE(DT_WeaponCSBaseGun)
	END_INIT_SEND_TABLE()
#endif

#if defined(CLIENT_DLL)
	BEGIN_INIT_RECV_TABLE(CWeaponCSBaseGun)
	BEGIN_NETWORK_TABLE(CWeaponCSBaseGun, DT_WeaponCSBaseGun, DT_WeaponCSBase)

	END_NETWORK_TABLE(DT_WeaponCSBaseGun)
	END_INIT_RECV_TABLE()
#endif
};


#endif // WEAPON_CSBASE_GUN_H
