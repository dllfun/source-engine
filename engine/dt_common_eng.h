//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef DT_COMMON_ENG_H
#define DT_COMMON_ENG_H
#ifdef _WIN32
#pragma once
#endif

#include "bitbuf.h"

class CBaseClientState;
class ServerClass;

// For shortcutting when server and client have the same game .dll
//  data
void DataTable_CreateClientTablesFromServerTables();
void DataTable_CreateClientClassInfosFromServerClasses( CBaseClientState *pState );

void DataTable_ClearWriteFlags( ServerClass *pClasses );
bool DataTable_LoadDataTablesFromBuffer( bf_read *pBuf, int nDemoProtocol );

void DataTable_WriteSendTablesBuffer( ServerClass *pClasses, bf_write *pBuf );
void DataTable_WriteClassInfosBuffer(ServerClass *pClasses, bf_write *pBuf );

// In order to receive a table, you must send it from the server and receive its info
	// on the client so the client knows how to unpack it.
bool SendTable_WriteInfos(SendTable* pSendTable, bf_write* pBuf);
// After calling RecvTable_Init to provide the list of RecvTables, call this
	// as the server sends its SendTables over. When bNeedsDecoder is specified,
	// it will precalculate the necessary data to actually decode this type of
	// SendTable from the server. nDemoProtocol = 0 means current version.
bool RecvTable_RecvClassInfos(bf_read* pBuf, bool bNeedsDecoder, int nDemoProtocol = 0);
#endif // DT_COMMON_ENG_H
