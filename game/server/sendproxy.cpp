//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: implements various common send proxies
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "sendproxy.h"
#include "basetypes.h"
#include "baseentity.h"
#include "team.h"
#include "player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Proxy that only sends data to team members
// Input  : *pStruct - 
//			*pData - 
//			*pOut - 
//			objectID - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
void* SendProxy_OnlyToTeam( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID )
{
	CBaseEntity *pEntity = (CBaseEntity*)pStruct;
	if ( pEntity )
	{
		CTeam *pTeam = pEntity->GetTeam();
		if ( pTeam )
		{
			pRecipients->ClearAllRecipients();
			for ( int i=0; i < pTeam->GetNumPlayers(); i++ )
				pRecipients->SetRecipient( pTeam->GetPlayer( i )->GetClientIndex() );
		
			return (void*)pVarData;
		}
	}

	return NULL;
}
REGISTER_SEND_PROXY_NON_MODIFIED_POINTER( SendProxy_OnlyToTeam );

#define TIME_BITS 24

// This table encodes edict data.
//static void SendProxy_Time( const SendProp *pProp, const void *pStruct, const void *pVarData, DVariant *pOut, int iElement, int objectID )
//{
//	float clock_base = floor( gpGlobals->GetCurTime() );
//	float t = *( float * )pVarData;
//	float dt = t - clock_base;
//	int addt = Floor2Int( 1000.0f * dt + 0.5f );
//	// TIME_BITS bits gives us TIME_BITS-1 bits plus sign bit
//	int maxoffset = 1 << ( TIME_BITS - 1);
//
//	addt = clamp( addt, -maxoffset, maxoffset );
//
//	pOut->m_Int = addt;
//}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pVarName - 
//			sizeofVar - 
//			flags - 
//			pId - 
// Output : SendProp
//-----------------------------------------------------------------------------




//#if !defined( NO_ENTITY_PREDICTION )

//#define PREDICTABLE_ID_BITS 31

//-----------------------------------------------------------------------------
// Purpose: Converts a predictable Id to an integer
// Input  : *pStruct - 
//			*pVarData - 
//			*pOut - 
//			iElement - 
//			objectID - 
//-----------------------------------------------------------------------------
//static void SendProxy_PredictableIdToInt( const SendProp *pProp, const void *pStruct, const void *pVarData, DVariant *pOut, int iElement, int objectID )
//{
//	CPredictableId* pId = ( CPredictableId * )pVarData;
//	if ( pId )
//	{
//		pOut->m_Int = pId->GetRaw();
//	}
//	else
//	{
//		pOut->m_Int = 0;
//	}
//}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pVarName - 
//			sizeofVar - 
//			flags - 
//			pId - 
// Output : SendProp
//-----------------------------------------------------------------------------
//SendProp SendPropPredictableId(
//	const char *pVarName,
//	int offset,
//	int sizeofVar )
//{
//	return SendPropInt( pVarName, offset, sizeofVar, PREDICTABLE_ID_BITS, SPROP_UNSIGNED, SendProxy_PredictableIdToInt );
//}

//#endif
