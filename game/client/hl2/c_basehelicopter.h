//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_BASEHELICOPTER_H
#define C_BASEHELICOPTER_H
#ifdef _WIN32
#pragma once
#endif


#include "c_ai_basenpc.h"


class C_BaseHelicopter : public C_AI_BaseNPC
{
public:
	DECLARE_CLASS( C_BaseHelicopter, C_AI_BaseNPC );
	DECLARE_CLIENTCLASS();

	C_BaseHelicopter();

	float StartupTime() const { return m_flStartupTime; }

private:
	C_BaseHelicopter( const C_BaseHelicopter &other ) {}
	float m_flStartupTime;

public:
	BEGIN_INIT_RECV_TABLE(C_BaseHelicopter)
	BEGIN_RECV_TABLE(C_BaseHelicopter, DT_BaseHelicopter, DT_AI_BaseNPC)
		RecvPropTime(RECVINFO(m_flStartupTime)),
	END_RECV_TABLE()
	END_INIT_RECV_TABLE()
};


#endif // C_BASEHELICOPTER_H
