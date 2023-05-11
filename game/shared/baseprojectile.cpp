//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "baseprojectile.h"


IMPLEMENT_NETWORKCLASS_ALIASED( BaseProjectile, DT_BaseProjectile )

#if defined( CLIENT_DLL )
BEGIN_NETWORK_TABLE( CBaseProjectile, DT_BaseProjectile, DT_BaseAnimating)
END_NETWORK_TABLE(DT_BaseProjectile)
#endif


//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CBaseProjectile::CBaseProjectile()
{
#ifdef GAME_DLL
	m_iDestroyableHitCount = 0;
#endif
}
