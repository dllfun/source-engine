//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "baseprojectile.h"

#ifdef CLIENT_DLL
#undef CBaseProjectile
#endif // CLIENT_DLL

IMPLEMENT_NETWORKCLASS_ALIASED( BaseProjectile, DT_BaseProjectile )

#ifdef CLIENT_DLL
#define CBaseProjectile C_BaseProjectile
#endif // CLIENT_DLL


//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CBaseProjectile::CBaseProjectile()
{
#ifdef GAME_DLL
	m_iDestroyableHitCount = 0;
#endif
}
