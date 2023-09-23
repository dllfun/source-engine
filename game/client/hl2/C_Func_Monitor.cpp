//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_FuncMonitor : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_FuncMonitor, C_BaseEntity );
	DECLARE_CLIENTCLASS();

// C_BaseEntity.
public:
	virtual bool	ShouldDraw();

public:
	BEGIN_INIT_RECV_TABLE(C_FuncMonitor)
	BEGIN_RECV_TABLE(C_FuncMonitor, DT_FuncMonitor, DT_BaseEntity)

	END_RECV_TABLE(DT_FuncMonitor)
	END_INIT_RECV_TABLE()
};

IMPLEMENT_CLIENTCLASS( C_FuncMonitor, DT_FuncMonitor, CFuncMonitor )


bool C_FuncMonitor::ShouldDraw()
{
	return true;
}
