//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "baseprojectile.h"


IMPLEMENT_NETWORKCLASS_ALIASED( BaseProjectile, DT_BaseProjectile )




//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CBaseProjectile::CBaseProjectile()
{
#ifdef GAME_DLL
	m_iDestroyableHitCount = 0;
#endif
}
