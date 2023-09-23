//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_SMOKEGRENADE_H
#define WEAPON_SMOKEGRENADE_H
#ifdef _WIN32
#pragma once
#endif


#include "weapon_basecsgrenade.h"


#ifdef CLIENT_DLL
	
	#define CSmokeGrenade C_SmokeGrenade

#endif


//-----------------------------------------------------------------------------
// Smoke grenades
//-----------------------------------------------------------------------------
class CSmokeGrenade : public CBaseCSGrenade
{
public:
	DECLARE_CLASS( CSmokeGrenade, CBaseCSGrenade );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CSmokeGrenade() {}

	virtual CSWeaponID GetWeaponID( void ) const { return WEAPON_SMOKEGRENADE; }

#ifdef CLIENT_DLL

#else
	DECLARE_DATADESC();

	void EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer );

#endif

	CSmokeGrenade( const CSmokeGrenade & ) {}

#ifndef CLIENT_DLL
	BEGIN_INIT_SEND_TABLE(CSmokeGrenade)
	BEGIN_NETWORK_TABLE(CSmokeGrenade, DT_SmokeGrenade, DT_BaseCSGrenade)

	END_NETWORK_TABLE(DT_SmokeGrenade)
	END_INIT_SEND_TABLE()
#endif

#ifdef CLIENT_DLL
	BEGIN_INIT_RECV_TABLE(CSmokeGrenade)
	BEGIN_NETWORK_TABLE(CSmokeGrenade, DT_SmokeGrenade, DT_BaseCSGrenade)

	END_NETWORK_TABLE(DT_SmokeGrenade)
	END_INIT_RECV_TABLE()
#endif
};


#endif // WEAPON_SMOKEGRENADE_H
