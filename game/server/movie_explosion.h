//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef MOVIE_EXPLOSION_H
#define MOVIE_EXPLOSION_H


#include "baseparticleentity.h"


class MovieExplosion : public CBaseParticleEntity
{
public:
	DECLARE_CLASS( MovieExplosion, CBaseParticleEntity );
	DECLARE_SERVERCLASS();

	static MovieExplosion* CreateMovieExplosion(const Vector &pos);

	BEGIN_SEND_TABLE(MovieExplosion, DT_MovieExplosion, DT_BaseParticleEntity)

	END_SEND_TABLE(DT_MovieExplosion)
};


#endif


