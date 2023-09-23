//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_HEGRENADE_H
#define WEAPON_HEGRENADE_H
#ifdef _WIN32
#pragma once
#endif


#include "weapon_basecsgrenade.h"


#ifdef CLIENT_DLL
	
	#define CHEGrenade C_HEGrenade

#endif

//-----------------------------------------------------------------------------
// Fragmentation grenades
//-----------------------------------------------------------------------------
class CHEGrenade : public CBaseCSGrenade
{
public:
	DECLARE_CLASS( CHEGrenade, CBaseCSGrenade );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CHEGrenade() {}

	virtual CSWeaponID GetWeaponID( void ) const		{ return WEAPON_HEGRENADE; }

#ifdef CLIENT_DLL

#else
	DECLARE_DATADESC();

	virtual void EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer );
	
#endif

	CHEGrenade( const CHEGrenade & ) {}

#ifndef CLIENT_DLL
	BEGIN_INIT_SEND_TABLE(CHEGrenade)
	BEGIN_NETWORK_TABLE(CHEGrenade, DT_HEGrenade, DT_BaseCSGrenade)

	END_NETWORK_TABLE(DT_HEGrenade)
	END_INIT_SEND_TABLE()
#endif

#ifdef CLIENT_DLL
	BEGIN_INIT_RECV_TABLE(CHEGrenade)
	BEGIN_NETWORK_TABLE(CHEGrenade, DT_HEGrenade, DT_BaseCSGrenade)

	END_NETWORK_TABLE(DT_HEGrenade)
	END_INIT_RECV_TABLE()
#endif
};


#endif // WEAPON_HEGRENADE_H
