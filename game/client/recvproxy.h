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
template<typename T= CBaseHandle>
void RecvProxy_IntToEHandle( const CRecvProxyData *pData, void *pStruct, void *pOut );

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

template<typename T>
void RecvProxy_IntToEHandle(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	T* pEHandle = (T*)pOut;//CBaseHandle

	if (pData->m_Value.m_Int == INVALID_NETWORKED_EHANDLE_VALUE)
	{
		pEHandle->Term();// = INVALID_EHANDLE_INDEX;
	}
	else
	{
		int iEntity = pData->m_Value.m_Int & ((1 << MAX_EDICT_BITS) - 1);
		int iSerialNum = pData->m_Value.m_Int >> MAX_EDICT_BITS;

		pEHandle->Init(iEntity, iSerialNum);
	}
}

template<typename T= CHandle<C_BaseEntity>>
void RecvProxy_IntToMoveParent( const CRecvProxyData *pData, void *pStruct, void *pOut );

//-----------------------------------------------------------------------------
// Moveparent receive proxies
//-----------------------------------------------------------------------------
template<typename T>
void RecvProxy_IntToMoveParent(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	T* pHandle = (T*)pOut;//CHandle<C_BaseEntity>
	RecvProxy_IntToEHandle<T>(pData, pStruct, (CBaseHandle*)pHandle);
}

template<typename T= color32>
void RecvProxy_IntToColor32( const CRecvProxyData *pData, void *pStruct, void *pOut );
template<typename T>
void RecvProxy_IntToColor32(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	T* pOutColor = (T*)pOut;//color32
	unsigned int inColor = *((unsigned int*)&pData->m_Value.m_Int);

	pOutColor->SetR((unsigned char)(inColor >> 24));
	pOutColor->SetG((unsigned char)((inColor >> 16) & 0xFF));
	pOutColor->SetB((unsigned char)((inColor >> 8) & 0xFF));
	pOutColor->SetA((unsigned char)(inColor & 0xFF));
}

template<typename T= int>
void RecvProxy_IntSubOne( const CRecvProxyData *pData, void *pStruct, void *pOut );
template<typename T>
void RecvProxy_IntSubOne(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	T* pInt = (T*)pOut;//int

	*pInt = pData->m_Value.m_Int - 1;
}

template<typename T= short>
void RecvProxy_ShortSubOne( const CRecvProxyData *pData, void *pStruct, void *pOut );
template<typename T>
void RecvProxy_ShortSubOne(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	T* pInt = (T*)pOut;//short

	*pInt = pData->m_Value.m_Int - 1;
}

void RecvProxy_IntToModelIndex16_BackCompatible( const CRecvProxyData *pData, void *pStruct, void *pOut );
void RecvProxy_IntToModelIndex32_BackCompatible( const CRecvProxyData *pData, void *pStruct, void *pOut );

class RecvPropTime : public RecvPropFloat {
public:
	RecvPropTime() {}

	template<typename T= float>
	RecvPropTime(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE);
	virtual	~RecvPropTime() {}
	RecvPropTime& operator=(const RecvPropTime& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropTime* pRecvProp = new RecvPropTime;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pVarName - 
//			sizeofVar - 
// Output : RecvProp
//-----------------------------------------------------------------------------
template<typename T>
RecvPropTime::RecvPropTime(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar/*=SIZEOF_IGNORE*/)
	:RecvPropFloat(pType, pVarName, offset, sizeofVar)
{
	//	return RecvPropInt( pVarName, offset, sizeofVar, 0, RecvProxy_Time );

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

	template<typename T = int>
	RecvPropEHandle(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,
		RecvVarProxyFn proxyFn = RecvProxy_IntToEHandle<T>);
	virtual	~RecvPropEHandle() {}
	RecvPropEHandle& operator=(const RecvPropEHandle& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropEHandle* pRecvProp = new RecvPropEHandle;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

template<typename T>
RecvPropEHandle::RecvPropEHandle(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar,
	RecvVarProxyFn proxyFn)
	:RecvPropInt((int*)0, pVarName, offset, sizeofVar, 0, proxyFn)
{

}

class RecvPropBool : public RecvPropInt {
public:
	RecvPropBool() {}

	template<typename T = int>
	RecvPropBool(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar);
	virtual	~RecvPropBool() {}
	RecvPropBool& operator=(const RecvPropBool& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropBool* pRecvProp = new RecvPropBool;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

template<typename T>
RecvPropBool::RecvPropBool(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar)
	:RecvPropInt(pType, pVarName, offset, sizeofVar)
{
	Assert(sizeofVar == sizeof(bool));
}

class RecvPropIntWithMinusOneFlag : public RecvPropInt {
public:
	RecvPropIntWithMinusOneFlag() {}

	template<typename T = int>
	RecvPropIntWithMinusOneFlag(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,
		RecvVarProxyFn proxyFn = RecvProxy_IntSubOne<T>);
	virtual	~RecvPropIntWithMinusOneFlag() {}
	RecvPropIntWithMinusOneFlag& operator=(const RecvPropIntWithMinusOneFlag& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropIntWithMinusOneFlag* pRecvProp = new RecvPropIntWithMinusOneFlag;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

template<typename T>
RecvPropIntWithMinusOneFlag::RecvPropIntWithMinusOneFlag(
	T* pType,
	const char* pVarName, 
	int offset, 
	int sizeofVar, 
	RecvVarProxyFn proxyFn)
	:RecvPropInt(pType, pVarName, offset, sizeofVar, 0, proxyFn)
{

}
 
class RecvPropColor32 : public RecvPropInt {
public:
	RecvPropColor32() {}

	template<typename T = color32>
	RecvPropColor32(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,	// Handled by RECVINFO macro, but set to SIZEOF_IGNORE if you don't want to bother.
		int flags = 0,
		RecvVarProxyFn varProxy = RecvProxy_IntToColor32<T>
	);
	virtual	~RecvPropColor32() {}
	RecvPropColor32& operator=(const RecvPropColor32& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropColor32* pRecvProp = new RecvPropColor32;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

template<typename T>
RecvPropColor32::RecvPropColor32(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar,
	int flags,
	RecvVarProxyFn varProxy
):RecvPropInt((int*)0, pVarName, offset, sizeofVar, flags, varProxy)
{
	
}

class RecvPropIntWithShortSubOne : public RecvPropIntWithMinusOneFlag {
public:
	RecvPropIntWithShortSubOne() {}

	template<typename T = int>
	RecvPropIntWithShortSubOne(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,
		RecvVarProxyFn proxyFn = RecvProxy_ShortSubOne<T>);
	virtual	~RecvPropIntWithShortSubOne() {}
	RecvPropIntWithShortSubOne& operator=(const RecvPropIntWithShortSubOne& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropIntWithShortSubOne* pRecvProp = new RecvPropIntWithShortSubOne;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

template<typename T>
RecvPropIntWithShortSubOne::RecvPropIntWithShortSubOne(
	T* pType,
	const char* pVarName, 
	int offset, 
	int sizeofVar, 
	RecvVarProxyFn proxyFn
):RecvPropIntWithMinusOneFlag(pType, pVarName, offset, sizeofVar, proxyFn)
{

}

class RecvPropIntToMoveParent : public RecvPropInt {
public:
	RecvPropIntToMoveParent() {}

	template<typename T = int>
	RecvPropIntToMoveParent(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,	// Handled by RECVINFO macro, but set to SIZEOF_IGNORE if you don't want to bother.
		int flags = 0,
		RecvVarProxyFn varProxy = RecvProxy_IntToMoveParent<T>
	);
	virtual	~RecvPropIntToMoveParent() {}
	RecvPropIntToMoveParent& operator=(const RecvPropIntToMoveParent& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropIntToMoveParent* pRecvProp = new RecvPropIntToMoveParent;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

template<typename T>
RecvPropIntToMoveParent::RecvPropIntToMoveParent(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar,
	int flags,
	RecvVarProxyFn varProxy
):RecvPropInt((int*)0, pVarName, offset, sizeofVar, flags, varProxy)
{

}


#endif // RECVPROXY_H

