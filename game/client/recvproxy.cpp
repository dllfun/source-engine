//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: implements various common send proxies
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "recvproxy.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#include "cdll_client_int.h"
#include "proto_version.h"







void RecvProxy_IntToModelIndex16_BackCompatible( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	int modelIndex = pData->m_Value.m_Int;
	if ( modelIndex < -1 && engineClient->GetProtocolVersion() <= PROTOCOL_VERSION_20 )
	{
		Assert( modelIndex > -20000 );
		modelIndex = -2 - ( ( -2 - modelIndex ) << 1 );
	}
	*(int16*)pOut = modelIndex;
}

void RecvProxy_IntToModelIndex32_BackCompatible( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	int modelIndex = pData->m_Value.m_Int;
	if ( modelIndex < -1 && engineClient->GetProtocolVersion() <= PROTOCOL_VERSION_20 )
	{
		Assert( modelIndex > -20000 );
		modelIndex = -2 - ( ( -2 - modelIndex ) << 1 );
	}
	*(int32*)pOut = modelIndex;
}
















//-----------------------------------------------------------------------------
// Purpose: Decodes a time value
// Input  : *pStruct - ( C_BaseEntity * ) used to flag animtime is changine
//			*pVarData - 
//			*pIn - 
//			objectID - 
//-----------------------------------------------------------------------------
//static void RecvProxy_Time( const CRecvProxyData *pData, void *pStruct, void *pOut )
//{
//	float	t;
//	float	clock_base;
//	float	offset;
//
//	// Get msec offset
//	offset	= ( float )pData->m_Value.m_Int / 1000.0f;
//
//	// Get base
//	clock_base = floor(engineClient->GetLastTimeStamp() );
//
//	// Add together and clamp to msec precision
//	t = ClampToMsec( clock_base + offset );
//
//	// Store decoded value
//	*( float * )pOut = t;
//}





//-----------------------------------------------------------------------------
// Purpose: Okay, so we have to queue up the actual ehandle to entity lookup for the following reason:
//  If a player has an EHandle/CHandle to an object such as a weapon, since the player is in slot 1-31, then
//  if the weapon is created and given to the player in the same frame, then the weapon won't have been
//  created at the time we parse this EHandle index, since the player is ahead of every other entity in the
//  packet (except for the world).
// So instead, we remember which ehandles need to be set and we set them after all network data has
//  been received.  Sigh.
// Input  : *pData - 
//			*pStruct - 
//			*pOut - 
//-----------------------------------------------------------------------------
//#if !defined( NO_ENTITY_PREDICTION )
//static void RecvProxy_IntToPredictableId( const CRecvProxyData *pData, void *pStruct, void *pOut )
//{
//	CPredictableId *pId = (CPredictableId*)pOut;
//	Assert( pId );
//	pId->SetRaw( pData->m_Value.m_Int );
//}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pVarName - 
//			sizeofVar - 
// Output : RecvProp
//-----------------------------------------------------------------------------
//RecvProp RecvPropPredictableId(
//	const char *pVarName, 
//	int offset, 
//	int sizeofVar/*=SIZEOF_IGNORE*/ )
//{
//	return RecvPropInt( pVarName, offset, sizeofVar, 0, RecvProxy_IntToPredictableId );
//}
//#endif
