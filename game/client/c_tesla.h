//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_TESLA_H
#define C_TESLA_H
#ifdef _WIN32
#pragma once
#endif


#include "c_baseentity.h"
#include "fx.h"
#include "utllinkedlist.h"


class C_Tesla : public C_BaseEntity
{
public:
	
	DECLARE_CLASS( C_Tesla, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	C_Tesla();

	virtual void ReceiveMessage( int classID, bf_read &msg );
	virtual void ClientThink();


public:

	CUtlLinkedList<CTeslaInfo,int> m_QueuedCommands;
	CNetworkString( m_SoundName,64);
	CNetworkString( m_iszSpriteName,256);

public:
	BEGIN_INIT_RECV_TABLE(C_Tesla)
	BEGIN_RECV_TABLE(C_Tesla, DT_Tesla, DT_BaseEntity)
		RecvPropString(RECVINFO(m_SoundName)),
		RecvPropString(RECVINFO(m_iszSpriteName))
	END_RECV_TABLE(DT_Tesla)
	END_INIT_RECV_TABLE()
};


#endif // C_TESLA_H
