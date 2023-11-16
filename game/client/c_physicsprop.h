//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_PHYSICSPROP_H
#define C_PHYSICSPROP_H
#ifdef _WIN32
#pragma once
#endif

#include "c_breakableprop.h"
#include "c_baseentity.h"
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_PhysicsProp : public C_BreakableProp
{
	typedef C_BreakableProp BaseClass;
public:
	DECLARE_CLASS(C_PhysicsProp, C_BreakableProp)
	DECLARE_CLIENTCLASS();

	C_PhysicsProp();
	~C_PhysicsProp();

	virtual bool OnInternalDrawModel( ClientModelRenderInfo_t *pInfo );

protected:
	// Networked vars.
	CNetworkVar( bool, m_bAwake);
	bool m_bAwakeLastTime;

public:
	BEGIN_INIT_RECV_TABLE(C_PhysicsProp)
	BEGIN_RECV_TABLE(C_PhysicsProp, DT_PhysicsProp, DT_BreakableProp)
		RecvPropBool(RECVINFO(m_bAwake)),
	END_RECV_TABLE(DT_PhysicsProp)
	END_INIT_RECV_TABLE()
};

#endif // C_PHYSICSPROP_H 
