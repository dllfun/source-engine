//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef TE_PARTICLESYSTEM_H
#define TE_PARTICLESYSTEM_H
#ifdef _WIN32
#pragma once
#endif


#include "basetempentity.h"


class CTEParticleSystem : public CBaseTempEntity
{
public:
	DECLARE_CLASS( CTEParticleSystem, CBaseTempEntity );
	DECLARE_SERVERCLASS();
	CTEParticleSystem() {};
	CTEParticleSystem(const char *pName) : BaseClass(pName)
	{
		m_vecOrigin.GetForModify().Init();
	}

	CNetworkVector( m_vecOrigin );

	BEGIN_INIT_SEND_TABLE(CTEParticleSystem)
	BEGIN_SEND_TABLE(CTEParticleSystem, DT_TEParticleSystem, DT_BaseTempEntity)
#if defined( TF_DLL )
		SendPropFloat(SENDINFO_VECTORELEM(m_vecOrigin, 0), -1, SPROP_COORD_MP_INTEGRAL),
		SendPropFloat(SENDINFO_VECTORELEM(m_vecOrigin, 1), -1, SPROP_COORD_MP_INTEGRAL),
		SendPropFloat(SENDINFO_VECTORELEM(m_vecOrigin, 2), -1, SPROP_COORD_MP_INTEGRAL),
#else
		SendPropFloat(SENDINFO_VECTORELEM(m_vecOrigin, 0), -1, SPROP_COORD),
		SendPropFloat(SENDINFO_VECTORELEM(m_vecOrigin, 1), -1, SPROP_COORD),
		SendPropFloat(SENDINFO_VECTORELEM(m_vecOrigin, 2), -1, SPROP_COORD),
#endif
	END_SEND_TABLE(DT_TEParticleSystem)
	END_INIT_SEND_TABLE()
};


#endif // TE_PARTICLESYSTEM_H
