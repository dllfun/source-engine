//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "c_weapon__stubs.h"
#include "basehlcombatweapon_shared.h"
#include "c_basehlcombatweapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

STUB_WEAPON_CLASS( cycler_weapon, WeaponCycler, C_BaseCombatWeapon ,DT_BaseCombatWeapon);

STUB_WEAPON_CLASS( weapon_binoculars, WeaponBinoculars, C_BaseHLCombatWeapon, DT_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_bugbait, WeaponBugBait, C_BaseHLCombatWeapon, DT_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_flaregun, Flaregun, C_BaseHLCombatWeapon, DT_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_annabelle, WeaponAnnabelle, C_BaseHLCombatWeapon, DT_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_gauss, WeaponGaussGun, C_BaseHLCombatWeapon, DT_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_cubemap, WeaponCubemap, C_BaseCombatWeapon, DT_BaseCombatWeapon);
STUB_WEAPON_CLASS( weapon_alyxgun, WeaponAlyxGun, C_HLSelectFireMachineGun, DT_HLSelectFireMachineGun);
STUB_WEAPON_CLASS( weapon_citizenpackage, WeaponCitizenPackage, C_BaseHLCombatWeapon, DT_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_citizensuitcase, WeaponCitizenSuitcase, C_WeaponCitizenPackage, DT_WeaponCitizenPackage);

#ifndef HL2MP
STUB_WEAPON_CLASS( weapon_ar2, WeaponAR2, C_HLMachineGun, DT_HLMachineGun);
STUB_WEAPON_CLASS( weapon_frag, WeaponFrag, C_BaseHLCombatWeapon, DT_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_rpg, WeaponRPG, C_BaseHLCombatWeapon, DT_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_pistol, WeaponPistol, C_BaseHLCombatWeapon, DT_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_shotgun, WeaponShotgun, C_BaseHLCombatWeapon, DT_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_smg1, WeaponSMG1, C_HLSelectFireMachineGun, DT_HLSelectFireMachineGun);
STUB_WEAPON_CLASS( weapon_357, Weapon357, C_BaseHLCombatWeapon, DT_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_crossbow, WeaponCrossbow, C_BaseHLCombatWeapon, DT_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_slam, Weapon_SLAM, C_BaseHLCombatWeapon, DT_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_crowbar, WeaponCrowbar, C_BaseHLBludgeonWeapon, DT_BaseHLBludgeonWeapon);
#ifdef HL2_EPISODIC
STUB_WEAPON_CLASS( weapon_hopwire, WeaponHopwire, C_BaseHLCombatWeapon, DT_BaseHLCombatWeapon);
//STUB_WEAPON_CLASS( weapon_proto1, WeaponProto1, C_BaseHLCombatWeapon );
#endif
#ifdef HL2_LOSTCOAST
STUB_WEAPON_CLASS( weapon_oldmanharpoon, WeaponOldManHarpoon, C_WeaponCitizenPackage, DT_WeaponCitizenPackage);
#endif
#endif


