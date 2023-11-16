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

template<typename T= color32>
void SendProxy_Color32ToInt( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID ,int aaa);
template<typename T>
void SendProxy_Color32ToInt(const SendProp* pProp, const void* pStruct, const void* pData, DVariant* pOut, int iElement, int objectID,int aaa)
{
	T* pIn = (T*)pData;//color32
	*((unsigned int*)&pOut->m_Int) = ((unsigned int)pIn->GetR() << 24) | ((unsigned int)pIn->GetG() << 16) | ((unsigned int)pIn->GetB() << 8) | ((unsigned int)pIn->GetA());
}
template<typename T= CBaseHandle>
void SendProxy_EHandleToInt( const SendProp *pProp, const void *pStruct, const void *pVarData, DVariant *pOut, int iElement, int objectID ,int aaa);
template<typename T>
void SendProxy_EHandleToInt(const SendProp* pProp, const void* pStruct, const void* pVarData, DVariant* pOut, int iElement, int objectID,int aaa)
{
	T* pHandle = (T*)pVarData;//CBaseHandle

	if (pHandle && pHandle->Get())
	{
		int iSerialNum = pHandle->GetSerialNumber() & (1 << NUM_NETWORKED_EHANDLE_SERIAL_NUMBER_BITS) - 1;
		pOut->m_Int = pHandle->GetEntryIndex() | (iSerialNum << MAX_EDICT_BITS);
	}
	else
	{
		pOut->m_Int = INVALID_NETWORKED_EHANDLE_VALUE;
	}
}
template<typename T= int>
void SendProxy_IntAddOne( const SendProp *pProp, const void *pStruct, const void *pVarData, DVariant *pOut, int iElement, int objectID ,int aaa);
template<typename T>
void SendProxy_IntAddOne(const SendProp* pProp, const void* pStruct, const void* pVarData, DVariant* pOut, int iElement, int objectID,int aaa)
{
	T* pInt = (T*)pVarData;//int

	pOut->m_Int = (*pInt) + 1;
}
template<typename T= short>
void SendProxy_ShortAddOne( const SendProp *pProp, const void *pStruct, const void *pVarData, DVariant *pOut, int iElement, int objectID ,int aaa);
template<typename T>
void SendProxy_ShortAddOne(const SendProp* pProp, const void* pStruct, const void* pVarData, DVariant* pOut, int iElement, int objectID,int aaa)
{
	T* pInt = (T*)pVarData;//short

	pOut->m_Int = (*pInt) + 1;
}

class SendPropBool : public SendPropInt {
public:
	SendPropBool() {}
	template<typename T>
	SendPropBool(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar);
	virtual ~SendPropBool() {}
	SendPropBool& operator=(const SendPropBool& srcSendProp) {
		SendProp::operator=(srcSendProp);
		return *this;
	}
	operator SendProp* () {
		SendPropBool* pSendProp = new SendPropBool;
		*pSendProp = *this;
		return pSendProp;
	}
}; 

template<typename T>
SendPropBool::SendPropBool(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar)
	: SendPropInt(pType, pVarName, offset, sizeofVar, 1, SPROP_UNSIGNED)
{
	Assert(sizeofVar == sizeof(bool));

}

class SendPropEHandle : public SendPropInt {
public:
	SendPropEHandle() {}

	template<typename T>
	SendPropEHandle(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,
		int flags = 0,
		SendVarProxyFn proxyFn = SendProxy_EHandleToInt<T>);
	virtual ~SendPropEHandle() {}
	SendPropEHandle& operator=(const SendPropEHandle& srcSendProp) {
		SendProp::operator=(srcSendProp);
		return *this;
	}
	operator SendProp* () {
		SendPropEHandle* pSendProp = new SendPropEHandle;
		*pSendProp = *this;
		return pSendProp;
	}
};

template<typename T>
SendPropEHandle::SendPropEHandle(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar,
	int flags,
	SendVarProxyFn proxyFn)
	:SendPropInt((int*)0, pVarName, offset, sizeofVar, NUM_NETWORKED_EHANDLE_BITS, SPROP_UNSIGNED | flags, proxyFn)
{

}

class SendPropTime : public SendPropFloat {
public:
	SendPropTime() {}

	template<typename T>
	SendPropTime(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE);
	virtual ~SendPropTime() {}
	SendPropTime& operator=(const SendPropTime& srcSendProp) {
		SendProp::operator=(srcSendProp);
		return *this;
	}
	operator SendProp* () {
		SendPropTime* pSendProp = new SendPropTime;
		*pSendProp = *this;
		return pSendProp;
	}
};

template<typename T>
SendPropTime::SendPropTime(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar)
	:SendPropFloat(pType, pVarName, offset, sizeofVar, -1, SPROP_NOSCALE)
{
	//	return SendPropInt( pVarName, offset, sizeofVar, TIME_BITS, 0, SendProxy_Time );
		// FIXME:  Re-enable above when it doesn't cause lots of deltas

}

//#if !defined( NO_ENTITY_PREDICTION )
//SendProp SendPropPredictableId(
//	const char *pVarName,
//	int offset,
//	int sizeofVar=SIZEOF_IGNORE	);
//#endif

class SendPropIntWithMinusOneFlag : public SendPropInt {
public:
	SendPropIntWithMinusOneFlag() {}
	template<typename T>
	SendPropIntWithMinusOneFlag(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,
		int bits = -1,
		SendVarProxyFn proxyFn = SendProxy_IntAddOne<T>);
	virtual ~SendPropIntWithMinusOneFlag() {}
	SendPropIntWithMinusOneFlag& operator=(const SendPropIntWithMinusOneFlag& srcSendProp) {
		SendProp::operator=(srcSendProp);
		return *this;
	}
	operator SendProp* () {
		SendPropIntWithMinusOneFlag* pSendProp = new SendPropIntWithMinusOneFlag;
		*pSendProp = *this;
		return pSendProp;
	}
};

template<typename T>
SendPropIntWithMinusOneFlag::SendPropIntWithMinusOneFlag(T* pType, const char* pVarName, int offset, int sizeofVar, int nBits, SendVarProxyFn proxyFn)
	:SendPropInt(pType, pVarName, offset, sizeofVar, nBits, SPROP_UNSIGNED, proxyFn)
{

}

class SendPropStringT : public SendPropString {
public:
	// Send a string_t as a string property.
	SendPropStringT() {}

	template<typename T>
	SendPropStringT(T* pType, const char* pVarName, int offset, int sizeofVar, int flags = 0);
	virtual ~SendPropStringT() {}
	SendPropStringT& operator=(const SendPropStringT& srcSendProp) {
		SendProp::operator=(srcSendProp);
		return *this;
	}
	operator SendProp* () {
		SendPropStringT* pSendProp = new SendPropStringT;
		*pSendProp = *this;
		return pSendProp;
	}
};

template<typename T= string_t>
void SendProxy_StringT_To_String(const SendProp* pProp, const void* pStruct, const void* pVarData, DVariant* pOut, int iElement, int objectID,int aaa);
template<typename T>
void SendProxy_StringT_To_String(const SendProp* pProp, const void* pStruct, const void* pVarData, DVariant* pOut, int iElement, int objectID,int aaa)
{
	T& str = *((T*)pVarData);//string_t
	pOut->m_pString = (char*)STRING(str);
}
//void SendProxy_String_tToString(const SendProp* pProp, const void* pStruct, const void* pData, DVariant* pOut, int iElement, int objectID)
//{
//	string_t* pString = (string_t*)pData;
//	pOut->m_pString = (char*)STRING(*pString);
//}

template<typename T>
SendPropStringT::SendPropStringT(T* pType, const char* pVarName, int offset, int sizeofVar, int flags)
	:SendPropString(pType, pVarName, offset, DT_MAX_STRING_BUFFERSIZE, flags, SendProxy_StringT_To_String<T>)
{
	// Make sure it's the right type.
	Assert(sizeofVar == sizeof(string_t));

}

class SendPropColor32 : public SendPropInt {
public:
	SendPropColor32() {}

	template<typename T>
	SendPropColor32(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,	// Handled by SENDINFO macro.
		int nBits = -1,					// Set to -1 to automatically pick (max) number of bits based on size of element.
		int flags = 0,
		SendVarProxyFn varProxy = 0);
	virtual ~SendPropColor32() {}
	SendPropColor32& operator=(const SendPropColor32& srcSendProp) {
		SendProp::operator=(srcSendProp);
		return *this;
	}
	operator SendProp* () {
		SendPropColor32* pSendProp = new SendPropColor32;
		*pSendProp = *this;
		return pSendProp;
	}
};

template<typename T>
SendPropColor32::SendPropColor32(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar,
	int nBits,
	int flags,
	SendVarProxyFn varProxy)
	: SendPropInt((int*)0, pVarName, offset, sizeofVar, nBits, flags, SendProxy_Color32ToInt<T>)
{
	//Assert(sizeofVar == sizeof(int));

}

class SendPropIntWithShortAddOne : public SendPropIntWithMinusOneFlag {
public:
	SendPropIntWithShortAddOne() {}
	template<typename T>
	SendPropIntWithShortAddOne(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,
		int bits = -1,
		SendVarProxyFn proxyFn = SendProxy_ShortAddOne<T>);
	virtual ~SendPropIntWithShortAddOne() {}
	SendPropIntWithShortAddOne& operator=(const SendPropIntWithShortAddOne& srcSendProp) {
		SendProp::operator=(srcSendProp);
		return *this;
	}
	operator SendProp* () {
		SendPropIntWithShortAddOne* pSendProp = new SendPropIntWithShortAddOne;
		*pSendProp = *this;
		return pSendProp;
	}
};

template<typename T>
SendPropIntWithShortAddOne::SendPropIntWithShortAddOne(T* pType, const char* pVarName, int offset, int sizeofVar, int nBits, SendVarProxyFn proxyFn)
	:SendPropIntWithMinusOneFlag(pType, pVarName, offset, sizeofVar, nBits, proxyFn)
{

}

//-----------------------------------------------------------------------------
// Purpose: Proxy that only sends data to team members
//-----------------------------------------------------------------------------
void* SendProxy_OnlyToTeam( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID );


#endif // SENDPROXY_H
