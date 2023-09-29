//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "tier0/icommandline.h"
#include "dt_stack.h"
#include "client.h"
#include "host.h"
#include "utllinkedlist.h"
#include "server.h"
#include "server_class.h"
#include "eiface.h"
#include "demo.h"
#include "sv_packedentities.h"
#include "cdll_engine_int.h"
#include "dt_common_eng.h";

#ifndef DEDICATED
#include "renamed_recvtable_compat.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//extern CUtlLinkedList< CClientSendTable*, unsigned short > g_ClientSendTables;
//extern CUtlLinkedList< CRecvDecoder *, unsigned short > g_RecvDecoders;

// If the table's ID is -1, writes its info into the buffer and increments curID.
void DataTable_MaybeCreateReceiveTable( CUtlVector< SendTable * >& visited, SendTable *pTable, bool bNeedDecoder )
{
	// Already sent?
	if ( visited.Find( pTable ) != visited.InvalidIndex() )
		return;

	visited.AddToTail( pTable );

	g_ClientDLL->GetRecvTableManager()->DataTable_SetupReceiveTableFromSendTable( pTable, bNeedDecoder );
}


void DataTable_MaybeCreateReceiveTable_R( CUtlVector< SendTable * >& visited, SendTable *pTable )
{
	DataTable_MaybeCreateReceiveTable( visited, pTable, false );

	// Make sure we send child send tables..
	for(int i=0; i < pTable->m_nProps; i++)
	{
		SendProp *pProp = &pTable->m_pProps[i];

		if( pProp->m_Type == DPT_DataTable )
		{
			DataTable_MaybeCreateReceiveTable_R( visited, pProp->GetDataTable() );
		}
	}
}

void DataTable_CreateClientTablesFromServerTables()
{
	if ( !serverGameDLL )
	{
		Sys_Error( "DataTable_CreateClientTablesFromServerTables:  No serverGameDLL loaded!" );
	}

	CUtlVector< SendTable* > visited;
	SendTable* pSendTable = serverGameDLL->GetSendTableManager()->GetSendTableHead();
	SendTable* pCur;
	for (pCur = pSendTable; pCur; pCur = pCur->m_pNext)
	{
		if (pCur->IsLeaf()) {
			DataTable_MaybeCreateReceiveTable(visited, pCur, true);
		}
		else {
			DataTable_MaybeCreateReceiveTable(visited, pCur, true);
		}
	}


	//ServerClass *pClasses = serverGameDLL->GetServerClassManager()->GetServerClassHead();
	//ServerClass *pCur;

	//CUtlVector< SendTable * > visited;

	//// First, we send all the leaf classes. These are the ones that will need decoders
	//// on the client.
	//for ( pCur=pClasses; pCur; pCur=pCur->GetNext() )
	//{
	//	if (!pCur->GetDataTable()) {
	//		continue;
	//	}
	//	DataTable_MaybeCreateReceiveTable( visited, pCur->GetDataTable(), true);
	//}

	//// Now, we send their base classes. These don't need decoders on the client
	//// because we will never send these SendTables by themselves.
	//for ( pCur=pClasses; pCur; pCur=pCur->GetNext() )
	//{
	//	if (!pCur->GetDataTable()) {
	//		continue;
	//	}
	//	DataTable_MaybeCreateReceiveTable_R( visited, pCur->GetDataTable() );
	//}
}

void DataTable_CreateClientClassInfosFromServerClasses( CBaseClientState *pState )
{
	if ( !serverGameDLL )
	{
		Sys_Error( "DataTable_CreateClientClassInfosFromServerClasses:  No serverGameDLL loaded!" );
	}

	ServerClass *pClasses = serverGameDLL->GetServerClassManager()->GetServerClassHead();

	// Count the number of classes.
	int nClasses = 0;
	for ( ServerClass *pCount=pClasses; pCount; pCount=pCount->GetNext() )
	{
		if (!pCount->GetDataTable()) {
			continue;
		}
		++nClasses;
	}

	// Remove old
	if ( pState->m_pServerClasses )
	{
		delete [] pState->m_pServerClasses;
	}

	Assert( nClasses > 0 );

	pState->m_nServerClasses = nClasses;
	pState->m_pServerClasses = new C_ServerClassInfo[ pState->m_nServerClasses ];
	if ( !pState->m_pServerClasses )
	{
		g_pHost->Host_EndGame(true, "CL_ParseClassInfo: can't allocate %d C_ServerClassInfos.\n", pState->m_nServerClasses);
		return;
	}

	// Now fill in the entries
	int curID = 0;
	for ( ServerClass *pClass=pClasses; pClass; pClass=pClass->GetNext() )
	{
		if (!pClass->GetDataTable()) {
			continue;
		}
		Assert( pClass->m_ClassID >= 0 && pClass->m_ClassID < nClasses );

		pClass->GetClassID() = curID++;

		pState->m_pServerClasses[ pClass->GetClassID()].m_ClassName = COM_StringCopy(pClass->GetNetworkName());
		pState->m_pServerClasses[ pClass->GetClassID()].m_DatatableName = COM_StringCopy(pClass->GetDataTable()->GetName());
	}
}

// If the table's ID is -1, writes its info into the buffer and increments curID.
void DataTable_MaybeWriteSendTableBuffer( SendTable *pTable, bf_write *pBuf, bool bNeedDecoder )
{
	// Already sent?
	if ( pTable->GetWriteFlag() )
		return;

	pTable->SetWriteFlag( true );

	pBuf->WriteOneBit( 1 ); // next SendTable follows
	pBuf->WriteOneBit( bNeedDecoder?1:0 );

	SendTable_WriteInfos(pTable, pBuf );
}

// Calls DataTable_MaybeWriteSendTable recursively.
void DataTable_MaybeWriteSendTableBuffer_R( SendTable *pTable, bf_write *pBuf )
{
	DataTable_MaybeWriteSendTableBuffer( pTable, pBuf, false );

	// Make sure we send child send tables..
	for(int i=0; i < pTable->m_nProps; i++)
	{
		SendProp *pProp = &pTable->m_pProps[i];

		if( pProp->m_Type == DPT_DataTable )
		{
			DataTable_MaybeWriteSendTableBuffer_R( pProp->GetDataTable(), pBuf );
		}
	}
}

void DataTable_ClearWriteFlags_R( SendTable *pTable )
{
	pTable->SetWriteFlag( false );

	for(int i=0; i < pTable->m_nProps; i++)
	{
		SendProp *pProp = &pTable->m_pProps[i];

		if( pProp->m_Type == DPT_DataTable )
		{
			DataTable_ClearWriteFlags_R( pProp->GetDataTable() );
		}
	}
}

void DataTable_ClearWriteFlags( ServerClass *pClasses )
{
	for ( ServerClass *pCur=pClasses; pCur; pCur=pCur->GetNext() )
	{
		if (!pCur->GetDataTable()) {
			continue;
		}
		DataTable_ClearWriteFlags_R( pCur->GetDataTable() );
	}
}

void DataTable_WriteSendTablesBuffer( ServerClass *pClasses, bf_write *pBuf )
{
	ServerClass *pCur;

	DataTable_ClearWriteFlags( pClasses );

	// First, we send all the leaf classes. These are the ones that will need decoders
	// on the client.
	for ( pCur=pClasses; pCur; pCur=pCur->GetNext() )
	{
		if (!pCur->GetDataTable()) {
			continue;
		}
		DataTable_MaybeWriteSendTableBuffer( pCur->GetDataTable(), pBuf, true);
	}

	// Now, we send their base classes. These don't need decoders on the client
	// because we will never send these SendTables by themselves.
	for ( pCur=pClasses; pCur; pCur=pCur->GetNext() )
	{
		if (!pCur->GetDataTable()) {
			continue;
		}
		DataTable_MaybeWriteSendTableBuffer_R( pCur->GetDataTable(), pBuf);
	}

	// Signal no more send tables
	pBuf->WriteOneBit( 0 );
}

void DataTable_WriteClassInfosBuffer(ServerClass *pClasses, bf_write *pBuf )
{
	int count = 0;

	ServerClass *pClass = pClasses;
	
	// first count total number of classes in list
	while ( pClass != NULL )
	{
		if (pClass->GetDataTable()) {
			count++;
		}
		pClass = pClass->GetNext();
	}

	// write number of classes
	pBuf->WriteShort( count );	

	pClass = pClasses; // go back to first class

	// write each class info
	while ( pClass != NULL )
	{
		if (pClass->GetDataTable()) {
			pBuf->WriteShort(pClass->GetClassID());
			pBuf->WriteString(pClass->GetNetworkName());
			pBuf->WriteString(pClass->GetDataTable()->GetName());
		}
		pClass=pClass->GetNext();
	}
}

bool DataTable_ParseClassInfosFromBuffer( CClientState *pState, bf_read *pBuf )
{
	if(pState->m_pServerClasses)
	{
		delete [] pState->m_pServerClasses;
	}

	pState->m_nServerClasses = pBuf->ReadShort();

	Assert( pState->m_nServerClasses );
	pState->m_pServerClasses = new C_ServerClassInfo[pState->m_nServerClasses];

	if ( !pState->m_pServerClasses )
	{
		g_pHost->Host_EndGame(true, "CL_ParseClassInfo: can't allocate %d C_ServerClassInfos.\n", pState->m_nServerClasses);
		return false;
	}
	
	for ( int i = 0; i < pState->m_nServerClasses; i++ )
	{
		int classID = pBuf->ReadShort();

		if( classID >= pState->m_nServerClasses )
		{
			g_pHost->Host_EndGame(true, "DataTable_ParseClassInfosFromBuffer: invalid class index (%d).\n", classID);
			return false;
		}

		pState->m_pServerClasses[classID].m_ClassName = pBuf->ReadAndAllocateString();
		pState->m_pServerClasses[classID].m_DatatableName = pBuf->ReadAndAllocateString();
	}

	return true;
}

bool SendTable_WriteInfos(SendTable* pSendTable, bf_write* pBuf)
{
	pBuf->WriteString(pSendTable->GetName());
	pBuf->WriteUBitLong(pSendTable->GetNumProps(), PROPINFOBITS_NUMPROPS);

	// Send each property.
	for (int iProp = 0; iProp < pSendTable->m_nProps; iProp++)
	{
		const SendProp* pProp = &pSendTable->m_pProps[iProp];

		pBuf->WriteUBitLong((unsigned int)pProp->m_Type, PROPINFOBITS_TYPE);
		pBuf->WriteString(pProp->GetName());
		// we now have some flags that aren't networked so strip them off
		unsigned int networkFlags = pProp->GetFlags() & ((1 << PROPINFOBITS_FLAGS) - 1);
		pBuf->WriteUBitLong(networkFlags, PROPINFOBITS_FLAGS);

		if (pProp->m_Type == DPT_DataTable)
		{
			// Just write the name and it will be able to reuse the table with a matching name.
			pBuf->WriteString(pProp->GetDataTable()->m_pNetTableName);
		}
		else
		{
			if (pProp->IsExcludeProp())
			{
				pBuf->WriteString(pProp->GetExcludeDTName());
			}
			else if (pProp->GetType() == DPT_Array)
			{
				pBuf->WriteUBitLong(pProp->GetNumElements(), PROPINFOBITS_NUMELEMENTS);
			}
			else
			{
				pBuf->WriteBitFloat(pProp->m_fLowValue);
				pBuf->WriteBitFloat(pProp->m_fHighValue);
				pBuf->WriteUBitLong(pProp->m_nBits, PROPINFOBITS_NUMBITS);
			}
		}
	}

	return !pBuf->IsOverflowed();
}

SendTable* RecvTable_ReadInfos(bf_read* pBuf, int nDemoProtocol)
{
	SendTable* pTable = new SendTable;

	pTable->m_pNetTableName = pBuf->ReadAndAllocateString();

	// Read the property list.
	pTable->m_nProps = pBuf->ReadUBitLong(PROPINFOBITS_NUMPROPS);
	pTable->m_pProps = pTable->m_nProps ? new SendProp[pTable->m_nProps] : NULL;

	for (int iProp = 0; iProp < pTable->m_nProps; iProp++)
	{
		SendProp* pProp = &pTable->m_pProps[iProp];

		pProp->m_Type = (SendPropType)pBuf->ReadUBitLong(PROPINFOBITS_TYPE);
		pProp->m_pVarName = pBuf->ReadAndAllocateString();

		int nFlagsBits = PROPINFOBITS_FLAGS;

		// HACK to playback old demos. SPROP_NUMFLAGBITS was 11, now 13
		// old nDemoProtocol was 2 
		if (nDemoProtocol == 2)
		{
			nFlagsBits = 11;
		}

		pProp->SetFlags(pBuf->ReadUBitLong(nFlagsBits));

		if (pProp->m_Type == DPT_DataTable)
		{
			pProp->m_pExcludeDTName = pBuf->ReadAndAllocateString();
		}
		else
		{
			if (pProp->IsExcludeProp())
			{
				pProp->m_pExcludeDTName = pBuf->ReadAndAllocateString();
			}
			else if (pProp->GetType() == DPT_Array)
			{
				pProp->SetNumElements(pBuf->ReadUBitLong(PROPINFOBITS_NUMELEMENTS));
			}
			else
			{
				pProp->m_fLowValue = pBuf->ReadBitFloat();
				pProp->m_fHighValue = pBuf->ReadBitFloat();
				pProp->m_nBits = pBuf->ReadUBitLong(PROPINFOBITS_NUMBITS);
			}
		}
	}

	return pTable;
}

void RecvTable_FreeSendTable(SendTable* pTable)
{
	for (int iProp = 0; iProp < pTable->m_nProps; iProp++)
	{
		SendProp* pProp = &pTable->m_pProps[iProp];

		delete[] pProp->m_pVarName;

		if (pProp->m_pExcludeDTName)
			delete[] pProp->m_pExcludeDTName;
	}

	if (pTable->m_pProps)
		delete[] pTable->m_pProps;

	delete pTable;
}

bool RecvTable_RecvClassInfos(bf_read* pBuf, bool bNeedsDecoder, int nDemoProtocol)
{
	SendTable* pSendTable = RecvTable_ReadInfos(pBuf, nDemoProtocol);

	if (!pSendTable)
		return false;

	bool ret = g_ClientDLL->GetRecvTableManager()->DataTable_SetupReceiveTableFromSendTable(pSendTable, bNeedsDecoder);

	RecvTable_FreeSendTable(pSendTable);

	return ret;
}

bool DataTable_LoadDataTablesFromBuffer( bf_read *pBuf, int nDemoProtocol )
{
	// Okay, read them out of the buffer since they weren't recorded into the main network stream during recording

	// Create all of the send tables locally
	// was DataTable_ParseClientTablesFromBuffer()
	while ( pBuf->ReadOneBit() != 0 )
	{
		bool bNeedsDecoder = pBuf->ReadOneBit() != 0;

		if ( !RecvTable_RecvClassInfos( pBuf, bNeedsDecoder, nDemoProtocol ) )
		{
			g_pHost->Host_Error( "DataTable_ParseClientTablesFromBuffer failed.\n" );
			return false;
		}
	}


	// Now create all of the server classes locally, too
	return DataTable_ParseClassInfosFromBuffer( &cl, pBuf );
}
