//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "func_ladder.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: A transient entity used to construct a true CFuncLadder
// FIXME:  THIS ENTITY IS OBSOLETE NOW, SHOULD BE REMOVED FROM HERE AND .FGD AT SOME POINT!!!
//-----------------------------------------------------------------------------
class CFuncLadderEndPoint : public CBaseEntity
{
public:
	DECLARE_CLASS( CFuncLadderEndPoint, CBaseEntity );
	DECLARE_SERVERCLASS();
	virtual void Activate();

private:
	bool		Validate();

public:
	BEGIN_INIT_SEND_TABLE(CFuncLadderEndPoint)
	BEGIN_SEND_TABLE(CFuncLadderEndPoint, DT_FuncLadderEndPoint, DT_BaseEntity)

	END_SEND_TABLE(DT_FuncLadderEndPoint)
	END_INIT_SEND_TABLE()
};

IMPLEMENT_SERVERCLASS(CFuncLadderEndPoint, DT_FuncLadderEndPoint)

LINK_ENTITY_TO_CLASS( func_ladderendpoint, CFuncLadderEndPoint );

void CFuncLadderEndPoint::Activate()
{
	BaseClass::Activate();

	if ( IsMarkedForDeletion() )
		return;

	Validate();
}

bool CFuncLadderEndPoint::Validate()
{
	// Find the the other end
	Vector startPos = GetAbsOrigin();
	
	CFuncLadderEndPoint *other = dynamic_cast< CFuncLadderEndPoint * >( GetNextTarget() );
	if ( !other )
	{
		DevMsg( 1, "func_ladderendpoint(%s) without matching target\n", STRING( GetEntityName() ) );
		return false;
	}

	Vector endPos = other->GetAbsOrigin();

	CFuncLadder *ladder = ( CFuncLadder * )engineServer->CreateEntityByName( "func_useableladder" );
	if ( ladder )
	{
		ladder->SetEndPoints( startPos, endPos );
		ladder->SetAbsOrigin( GetAbsOrigin() );
		ladder->SetParent( GetParent() );
		ladder->SetName( GetEntityName() );
		ladder->Spawn();
	}

	// Delete both endpoints
	UTIL_Remove( other );
	UTIL_Remove( this );

	return true;
}
