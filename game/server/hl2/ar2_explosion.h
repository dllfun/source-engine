//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef AR2_EXPLOSION_H
#define AR2_EXPLOSION_H


#include "baseparticleentity.h"


class AR2Explosion : public CBaseParticleEntity
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( AR2Explosion, CBaseParticleEntity );
	DECLARE_SERVERCLASS();

	static AR2Explosion* CreateAR2Explosion(const Vector &pos);

	inline void SetMaterialName(const char *szMaterialName);

private:

	CNetworkString( m_szMaterialName, 255 );

public:
	BEGIN_INIT_SEND_TABLE(AR2Explosion)
	BEGIN_SEND_TABLE(AR2Explosion, DT_AR2Explosion, DT_BaseParticleEntity)
		SendPropString(SENDINFO(m_szMaterialName)),
	END_SEND_TABLE()
	END_INIT_SEND_TABLE()
};


void AR2Explosion::SetMaterialName(const char *szMaterialName)
{
	if (szMaterialName)
	{
		Q_strncpy(m_szMaterialName.GetForModify(), szMaterialName, sizeof(m_szMaterialName));
	}
}


#endif
