//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#include "cbase.h"
#include "modelentities.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CFuncReflectiveGlass : public CFuncBrush
{
	DECLARE_DATADESC();
	DECLARE_CLASS( CFuncReflectiveGlass, CFuncBrush );
	DECLARE_SERVERCLASS();
	DECLARE_SEND_TABLE_ACCESS(DT_FuncReflectiveGlass);
};

// automatically hooks in the system's callbacks
BEGIN_DATADESC( CFuncReflectiveGlass )
END_DATADESC()

LINK_ENTITY_TO_CLASS( func_reflective_glass, CFuncReflectiveGlass );

IMPLEMENT_SERVERCLASS_ST( CFuncReflectiveGlass, DT_FuncReflectiveGlass, DT_BaseEntity)
END_SEND_TABLE(DT_FuncReflectiveGlass)
