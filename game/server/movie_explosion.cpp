//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "movie_explosion.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define MOVIEEXPLOSION_ENTITYNAME	"env_movieexplosion"


IMPLEMENT_SERVERCLASS(MovieExplosion, DT_MovieExplosion, DT_BaseParticleEntity)
BEGIN_SEND_TABLE(MovieExplosion, DT_MovieExplosion, DT_BaseParticleEntity)
END_SEND_TABLE(DT_MovieExplosion)

LINK_ENTITY_TO_CLASS(env_movieexplosion, MovieExplosion);


MovieExplosion* MovieExplosion::CreateMovieExplosion(const Vector &pos)
{
	CBaseEntity *pEnt = CreateEntityByName(MOVIEEXPLOSION_ENTITYNAME);
	if(pEnt)
	{
		MovieExplosion *pEffect = dynamic_cast<MovieExplosion*>(pEnt);
		if(pEffect && pEffect->edict())
		{
			pEffect->SetLocalOrigin( pos );
			pEffect->Activate();
			return pEffect;
		}
		else
		{
			UTIL_Remove(pEnt);
		}
	}

	return NULL;
}


