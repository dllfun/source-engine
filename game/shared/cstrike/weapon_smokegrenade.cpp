//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbase.h"
#include "gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "weapon_smokegrenade.h"


#ifdef CLIENT_DLL
	
#else

	#include "cs_player.h"
	#include "items.h"
	#include "smokegrenade_projectile.h"

#endif

#if defined( CLIENT_DLL )
#undef CSmokeGrenade
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( SmokeGrenade, DT_SmokeGrenade )

#if defined( CLIENT_DLL )
#define CSmokeGrenade C_SmokeGrenade
#endif

BEGIN_PREDICTION_DATA( CSmokeGrenade )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_smokegrenade, CSmokeGrenade );
PRECACHE_WEAPON_REGISTER( weapon_smokegrenade );


#ifndef CLIENT_DLL

	BEGIN_DATADESC( CSmokeGrenade )
	END_DATADESC()

	void CSmokeGrenade::EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer )
	{
		CSmokeGrenadeProjectile::Create( vecSrc, vecAngles, vecVel, angImpulse, pPlayer );
	}

#endif

