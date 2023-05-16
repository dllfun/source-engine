//========= Copyright Valve Corporation, All rights reserved. ============//
//
// An entity that allows level designer control over the fog parameters.
//
//=============================================================================

#include "cbase.h"
#include "c_env_fog_controller.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( FogController, DT_FogController )



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_FogController::C_FogController()
{
	// Make sure that old maps without fog fields don't get wacked out fog values.
	m_fog.enable = false;
	m_fog.maxdensity = 1.0f;
}
