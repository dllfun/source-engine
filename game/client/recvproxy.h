//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: implements various common send proxies
//
// $NoKeywords: $
//=============================================================================//

#ifndef RECVPROXY_H
#define RECVPROXY_H


#include "dt_recv.h"

class CRecvProxyData;


// This converts the int stored in pData to an EHANDLE in pOut.
void RecvProxy_IntToEHandle( const CRecvProxyData *pData, void *pStruct, void *pOut );

void RecvProxy_IntToMoveParent( const CRecvProxyData *pData, void *pStruct, void *pOut );
void RecvProxy_IntToColor32( const CRecvProxyData *pData, void *pStruct, void *pOut );
void RecvProxy_IntSubOne( const CRecvProxyData *pData, void *pStruct, void *pOut );
void RecvProxy_ShortSubOne( const CRecvProxyData *pData, void *pStruct, void *pOut );
void RecvProxy_InterpolationAmountChanged( const CRecvProxyData *pData, void *pStruct, void *pOut );
void RecvProxy_IntToModelIndex16_BackCompatible( const CRecvProxyData *pData, void *pStruct, void *pOut );
void RecvProxy_IntToModelIndex32_BackCompatible( const CRecvProxyData *pData, void *pStruct, void *pOut );

class RecvPropTime : public RecvPropFloat {
public:
	RecvPropTime() {}
	RecvPropTime(
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE);
	virtual	~RecvPropTime() {}
	RecvPropTime& operator=(const RecvPropTime& srcSendProp);
	operator RecvProp* () {
		RecvPropTime* pRecvProp = new RecvPropTime;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

//#if !defined( NO_ENTITY_PREDICTION )
//RecvProp RecvPropPredictableId(
//	const char *pVarName, 
//	int offset, 
//	int sizeofVar=SIZEOF_IGNORE );
//#endif

class RecvPropEHandle : public RecvPropInt {
public:
	RecvPropEHandle() {}
	RecvPropEHandle(
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,
		RecvVarProxyFn proxyFn = RecvProxy_IntToEHandle);
	virtual	~RecvPropEHandle() {}
	RecvPropEHandle& operator=(const RecvPropEHandle& srcSendProp);
	operator RecvProp* () {
		RecvPropEHandle* pRecvProp = new RecvPropEHandle;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

class RecvPropBool : public RecvPropInt {
public:
	RecvPropBool() {}
	RecvPropBool(
		const char* pVarName,
		int offset,
		int sizeofVar);
	virtual	~RecvPropBool() {}
	RecvPropBool& operator=(const RecvPropBool& srcSendProp);
	operator RecvProp* () {
		RecvPropBool* pRecvProp = new RecvPropBool;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

class RecvPropIntWithMinusOneFlag : RecvPropInt {
public:
	RecvPropIntWithMinusOneFlag() {}
	RecvPropIntWithMinusOneFlag(
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,
		RecvVarProxyFn proxyFn = RecvProxy_IntSubOne);
	virtual	~RecvPropIntWithMinusOneFlag() {}
	RecvPropIntWithMinusOneFlag& operator=(const RecvPropIntWithMinusOneFlag& srcSendProp);
	operator RecvProp* () {
		RecvPropIntWithMinusOneFlag* pRecvProp = new RecvPropIntWithMinusOneFlag;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

 

#endif // RECVPROXY_H

