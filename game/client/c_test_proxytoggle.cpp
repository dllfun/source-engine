//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_baseentity.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_Test_ProxyToggle_Networkable;
static C_Test_ProxyToggle_Networkable *g_pTestObj = 0;


// ---------------------------------------------------------------------------------------- //
// C_Test_ProxyToggle_Networkable
// ---------------------------------------------------------------------------------------- //

class C_Test_ProxyToggle_Networkable : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_Test_ProxyToggle_Networkable, C_BaseEntity );
	DECLARE_CLIENTCLASS();

			C_Test_ProxyToggle_Networkable()
			{
				g_pTestObj = this;
			}

			~C_Test_ProxyToggle_Networkable()
			{
				g_pTestObj = 0;
			}

	int		m_WithProxy;

	BEGIN_RECV_TABLE_NOBASE(C_Test_ProxyToggle_Networkable, DT_ProxyToggle_ProxiedData)
		RecvPropInt(RECVINFO(m_WithProxy))
	END_RECV_TABLE(DT_ProxyToggle_ProxiedData)

	BEGIN_RECV_TABLE(C_Test_ProxyToggle_Networkable, DT_ProxyToggle, DT_BaseEntity)
		RecvPropDataTable("blah", 0, 0, REFERENCE_RECV_TABLE(DT_ProxyToggle_ProxiedData))
	END_RECV_TABLE(DT_ProxyToggle)
};


// ---------------------------------------------------------------------------------------- //
// Datatables.
// ---------------------------------------------------------------------------------------- //



IMPLEMENT_CLIENTCLASS( C_Test_ProxyToggle_Networkable, DT_ProxyToggle, CTest_ProxyToggle_Networkable )




// ---------------------------------------------------------------------------------------- //
// Console commands.
// ---------------------------------------------------------------------------------------- //

// The engine uses this to get the current value.
CON_COMMAND_F( Test_ProxyToggle_EnsureValue, "Test_ProxyToggle_EnsureValue", FCVAR_CHEAT )
{
	if ( args.ArgC() < 2 )
	{
		Error( "Test_ProxyToggle_EnsureValue: requires value parameter." );
	}
	else if ( !g_pTestObj )
	{
		Error( "Test_ProxyToggle_EnsureValue: object doesn't exist on the client." );
	}

	int wantedValue = atoi( args[ 1 ] );
	if ( g_pTestObj->m_WithProxy != wantedValue )
	{
		Error( "Test_ProxyToggle_EnsureValue: value (%d) doesn't match wanted value (%d).", g_pTestObj->m_WithProxy, wantedValue );
	}
}




