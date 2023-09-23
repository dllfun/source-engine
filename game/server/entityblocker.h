//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef ENTITYBLOCKER_H
#define ENTITYBLOCKER_H
#ifdef _WIN32
#pragma once
#endif

//==================================================================================================================
// Entity Blocker
//==================================================================================================================
class CEntityBlocker : public CBaseEntity
{
	DECLARE_CLASS( CEntityBlocker, CBaseEntity );

public:
	DECLARE_SERVERCLASS();
	static CEntityBlocker *Create( const Vector &origin, const Vector &mins, const Vector &maxs, CBaseEntity *pOwner = NULL, bool bBlockPhysics = false );

	void Spawn( void );
	bool TestCollision( const Ray_t &ray, unsigned int mask, trace_t& trace );

	BEGIN_INIT_SEND_TABLE(CEntityBlocker)
	BEGIN_SEND_TABLE(CEntityBlocker, DT_EntityBlocker, DT_BaseEntity)

	END_SEND_TABLE(DT_EntityBlocker)
	END_INIT_SEND_TABLE()
};

#endif // ENTITYBLOCKER_H
