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
	char m_SoundName[64];
	char m_iszSpriteName[256];

	BEGIN_RECV_TABLE(C_Tesla, DT_Tesla, DT_BaseEntity)
		RecvPropString(RECVINFO(m_SoundName)),
		RecvPropString(RECVINFO(m_iszSpriteName))
	END_RECV_TABLE(DT_Tesla)
};


#endif // C_TESLA_H
