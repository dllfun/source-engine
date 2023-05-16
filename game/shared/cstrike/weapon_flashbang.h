//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_FLASHBANG_H
#define WEAPON_FLASHBANG_H
#ifdef _WIN32
#pragma once
#endif


#include "weapon_basecsgrenade.h"


#ifdef CLIENT_DLL
	#define CFlashbang C_Flashbang
#endif


//-----------------------------------------------------------------------------
// Fragmentation grenades
//-----------------------------------------------------------------------------
class CFlashbang : public CBaseCSGrenade
{
public:
	DECLARE_CLASS( CFlashbang, CBaseCSGrenade );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CFlashbang() {}

	virtual CSWeaponID GetWeaponID( void ) const		{ return WEAPON_FLASHBANG; }


#ifdef CLIENT_DLL

#else
	DECLARE_DATADESC();

		virtual void EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer );
#endif

	CFlashbang( const CFlashbang & ) {}

#ifndef CLIENT_DLL
	BEGIN_NETWORK_TABLE(CFlashbang, DT_Flashbang, DT_BaseCSGrenade)
	END_NETWORK_TABLE(DT_Flashbang)
#endif

#ifdef CLIENT_DLL
	BEGIN_NETWORK_TABLE(CFlashbang, DT_Flashbang, DT_BaseCSGrenade)
	END_NETWORK_TABLE(DT_Flashbang)
#endif
};


#endif // WEAPON_FLASHBANG_H
