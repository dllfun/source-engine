//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: implements various common send proxies
//
// $NoKeywords: $
//=============================================================================//

#ifndef SENDPROXY_H
#define SENDPROXY_H


#include "dt_send.h"


class DVariant;

void SendProxy_Color32ToInt( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID );
void SendProxy_EHandleToInt( const SendProp *pProp, const void *pStruct, const void *pVarData, DVariant *pOut, int iElement, int objectID );
void SendProxy_IntAddOne( const SendProp *pProp, const void *pStruct, const void *pVarData, DVariant *pOut, int iElement, int objectID );
void SendProxy_ShortAddOne( const SendProp *pProp, const void *pStruct, const void *pVarData, DVariant *pOut, int iElement, int objectID );

class SendPropBool : public SendPropInt {
public:
	SendPropBool() {}
	SendPropBool(
		const char* pVarName,
		int offset,
		int sizeofVar);
	virtual ~SendPropBool() {}
	SendPropBool& operator=(const SendPropBool& srcSendProp);
	operator SendProp* () {
		SendPropBool* pSendProp = new SendPropBool;
		*pSendProp = *this;
		return pSendProp;
	}
}; 

class SendPropEHandle : public SendPropInt {
public:
	SendPropEHandle() {}
	SendPropEHandle(
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,
		int flags = 0,
		SendVarProxyFn proxyFn = SendProxy_EHandleToInt);
	virtual ~SendPropEHandle() {}
	SendPropEHandle& operator=(const SendPropEHandle& srcSendProp);
	operator SendProp* () {
		SendPropEHandle* pSendProp = new SendPropEHandle;
		*pSendProp = *this;
		return pSendProp;
	}
};

class SendPropTime : public SendPropFloat {
public:
	SendPropTime() {}
	SendPropTime(
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE);
	virtual ~SendPropTime() {}
	SendPropTime& operator=(const SendPropTime& srcSendProp);
	operator SendProp* () {
		SendPropTime* pSendProp = new SendPropTime;
		*pSendProp = *this;
		return pSendProp;
	}
};

//#if !defined( NO_ENTITY_PREDICTION )
//SendProp SendPropPredictableId(
//	const char *pVarName,
//	int offset,
//	int sizeofVar=SIZEOF_IGNORE	);
//#endif

class SendPropIntWithMinusOneFlag : public SendPropInt {
public:
	SendPropIntWithMinusOneFlag() {}
	SendPropIntWithMinusOneFlag(
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,
		int bits = -1,
		SendVarProxyFn proxyFn = SendProxy_IntAddOne);
	virtual ~SendPropIntWithMinusOneFlag() {}
	SendPropIntWithMinusOneFlag& operator=(const SendPropIntWithMinusOneFlag& srcSendProp);
	operator SendProp* () {
		SendPropIntWithMinusOneFlag* pSendProp = new SendPropIntWithMinusOneFlag;
		*pSendProp = *this;
		return pSendProp;
	}
};

class SendPropStringT : public SendPropString {
public:
	// Send a string_t as a string property.
	SendPropStringT() {}
	SendPropStringT(const char* pVarName, int offset, int sizeofVar);
	virtual ~SendPropStringT() {}
	SendPropStringT& operator=(const SendPropStringT& srcSendProp);
	operator SendProp* () {
		SendPropStringT* pSendProp = new SendPropStringT;
		*pSendProp = *this;
		return pSendProp;
	}
};

//-----------------------------------------------------------------------------
// Purpose: Proxy that only sends data to team members
//-----------------------------------------------------------------------------
void* SendProxy_OnlyToTeam( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID );


#endif // SENDPROXY_H
