//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "basehlcombatweapon_shared.h"

#ifndef C_BASEHLCOMBATWEAPON_H
#define C_BASEHLCOMBATWEAPON_H
#ifdef _WIN32
#pragma once
#endif

class C_HLMachineGun : public C_BaseHLCombatWeapon
{
public:
	DECLARE_CLASS( C_HLMachineGun, C_BaseHLCombatWeapon );
	DECLARE_CLIENTCLASS();

public:
	BEGIN_INIT_RECV_TABLE(C_HLMachineGun)
	BEGIN_RECV_TABLE(C_HLMachineGun, DT_HLMachineGun, DT_BaseHLCombatWeapon)
	END_RECV_TABLE()
	END_INIT_RECV_TABLE()
};

class C_HLSelectFireMachineGun : public C_HLMachineGun
{
public:
	DECLARE_CLASS( C_HLSelectFireMachineGun, C_HLMachineGun );
	DECLARE_CLIENTCLASS();

public:
	BEGIN_INIT_RECV_TABLE(C_HLSelectFireMachineGun)
	BEGIN_RECV_TABLE(C_HLSelectFireMachineGun, DT_HLSelectFireMachineGun, DT_HLMachineGun)
	END_RECV_TABLE()
	END_INIT_RECV_TABLE()
};

class C_BaseHLBludgeonWeapon : public C_BaseHLCombatWeapon
{
public:
	DECLARE_CLASS( C_BaseHLBludgeonWeapon, C_BaseHLCombatWeapon );
	DECLARE_CLIENTCLASS();

public:
	BEGIN_INIT_RECV_TABLE(C_BaseHLBludgeonWeapon)
	BEGIN_RECV_TABLE(C_BaseHLBludgeonWeapon, DT_BaseHLBludgeonWeapon, DT_BaseHLCombatWeapon)
	END_RECV_TABLE()
	END_INIT_RECV_TABLE()
};

#endif // C_BASEHLCOMBATWEAPON_H
