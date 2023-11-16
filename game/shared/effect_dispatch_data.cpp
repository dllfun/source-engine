//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "effect_dispatch_data.h"
#include "coordsize.h"

#ifdef CLIENT_DLL
#include "cliententitylist.h"
#endif

#include "qlimits.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"



#ifdef CLIENT_DLL

	#include "dt_recv.h"

	

	

#else

	#include "dt_send.h"

	

#endif

#ifdef CLIENT_DLL

IClientRenderable *CEffectData::GetRenderable() const
{
	return ClientEntityList().GetClientRenderableFromHandle( m_hEntity );
}

C_BaseEntity *CEffectData::GetEntity() const
{
	return ClientEntityList().GetBaseEntityFromHandle( m_hEntity );
}

int CEffectData::entindex() const
{
	C_BaseEntity *pEnt = ClientEntityList().GetBaseEntityFromHandle( m_hEntity );
	return pEnt ? pEnt->entindex() : -1;
}

#endif

#ifdef CLIENT_DLL

bool g_bSuppressParticleEffects = false;

bool SuppressingParticleEffects()
{
	return g_bSuppressParticleEffects;
}

void SuppressParticleEffects( bool bSuppress )
{
	g_bSuppressParticleEffects = bSuppress;
}

#endif
