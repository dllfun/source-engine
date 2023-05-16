//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef BASEPROJECTILE_H
#define BASEPROJECTILE_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

#ifdef GAME_DLL
#include "baseanimating.h"
#else
#include "c_baseanimating.h"
#endif

#ifdef CLIENT_DLL
#define CBaseProjectile C_BaseProjectile
#endif // CLIENT_DLL

//=============================================================================
//
// Base Projectile.
//
//=============================================================================
class CBaseProjectile : public CBaseAnimating
{
public:
	DECLARE_CLASS( CBaseProjectile, CBaseAnimating );
	DECLARE_NETWORKCLASS();

	CBaseProjectile();

#ifdef GAME_DLL
	virtual int GetDestroyableHitCount( void ) const { return m_iDestroyableHitCount; }
	void IncrementDestroyableHitCount( void ) { ++m_iDestroyableHitCount; }
#endif // GAME_DLL

	virtual bool IsDestroyable( void ) { return false; }
	virtual void Destroy( bool bBlinkOut = true, bool bBreakRocket = false ) {}

protected:
#ifdef GAME_DLL
	int m_iDestroyableHitCount;
#endif // GAME_DLL

#if !defined( CLIENT_DLL )
	BEGIN_NETWORK_TABLE(CBaseProjectile, DT_BaseProjectile, DT_BaseAnimating)
	END_NETWORK_TABLE(DT_BaseProjectile)
#endif

#if defined( CLIENT_DLL )
	BEGIN_NETWORK_TABLE(CBaseProjectile, DT_BaseProjectile, DT_BaseAnimating)
	END_NETWORK_TABLE(DT_BaseProjectile)
#endif
};

#endif // BASEPROJECTILE_H
