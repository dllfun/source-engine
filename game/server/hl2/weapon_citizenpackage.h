//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================
#ifndef WEAPON_CITIZENPACKAGE_H
#define WEAPON_CITIZENPACKAGE_H
#ifdef _WIN32
#pragma once
#endif

#include "basehlcombatweapon.h"

//=============================================================================
//
// Weapon - Citizen Package Class
//
class CWeaponCitizenPackage : public CBaseHLCombatWeapon
{
	DECLARE_CLASS( CWeaponCitizenPackage, CBaseHLCombatWeapon );
public:

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();	
	DECLARE_ACTTABLE();

	void ItemPostFrame( void );
	void Drop( const Vector &vecVelocity );

public:
	BEGIN_INIT_SEND_TABLE(CWeaponCitizenPackage)
	BEGIN_SEND_TABLE(CWeaponCitizenPackage, DT_WeaponCitizenPackage, DT_BaseHLCombatWeapon)
	END_SEND_TABLE()
	END_INIT_SEND_TABLE()
};

#endif // WEAPON_CITIZENPACKAGE_H