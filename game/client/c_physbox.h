//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//



// Client-side CBasePlayer

#ifndef C_PHYSBOX_H
#define C_PHYSBOX_H
#pragma once


#include "c_baseentity.h"


class C_PhysBox : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_PhysBox, C_BaseEntity );
	DECLARE_CLIENTCLASS();

					C_PhysBox();
	virtual			~C_PhysBox();
	virtual ShadowType_t ShadowCastType();
	
public:
	CNetworkVar( float,			m_mass);	// TEST..

public:
	BEGIN_INIT_RECV_TABLE(C_PhysBox)
	BEGIN_RECV_TABLE(C_PhysBox, DT_PhysBox, DT_BaseEntity)
		RecvPropFloat(RECVINFO(m_mass), 0), // Test..
	END_RECV_TABLE(DT_PhysBox)
	END_INIT_RECV_TABLE()
};


#endif



