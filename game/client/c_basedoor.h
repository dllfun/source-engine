//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#if !defined( C_BASEDOOR_H )
#define C_BASEDOOR_H
#ifdef _WIN32
#pragma once
#endif

#include "c_baseentity.h"

#if defined( CLIENT_DLL )
#define CBaseDoor C_BaseDoor
#endif

class C_BaseDoor : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_BaseDoor, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	C_BaseDoor( void );
	~C_BaseDoor( void );

public:
	float		m_flWaveHeight;

	BEGIN_RECV_TABLE(C_BaseDoor, DT_BaseDoor, DT_BaseEntity)
		RecvPropFloat(RECVINFO(m_flWaveHeight)),
	END_RECV_TABLE(DT_BaseDoor)
};

#endif // C_BASEDOOR_H