//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for HL1.
//
// $NoKeywords: $
//=============================================================================//

#ifndef DT_UTLVECTOR_SEND_H
#define DT_UTLVECTOR_SEND_H
#pragma once


#include "dt_send.h"
#include "dt_utlvector_common.h"


#define SENDINFO_UTLVECTOR( varName )	#varName, \
										offsetof(currentSendDTClass, varName), \
										sizeof(((currentSendDTClass*)0)->varName[0]), \
										GetEnsureCapacityTemplate( ((currentSendDTClass*)0)->varName )



#define SendPropUtlVectorDataTable( varName, nMaxElements, dataTableName ) \
	SendPropUtlVector( \
		SENDINFO_UTLVECTOR( varName ), \
		nMaxElements, \
		SendPropDataTable( NULL, 0, REFERENCE_SEND_TABLE( dataTableName ) ) \
		)

//
// Set it up to transmit a CUtlVector of basic types or of structures.
//
// pArrayProp doesn't need a name, offset, or size. You can pass 0 for all those.
// Example usage:
//
//	SendPropUtlVectorDataTable( m_StructArray, 11, DT_TestStruct )
//
//	SendPropUtlVector( 
//		SENDINFO_UTLVECTOR( m_FloatArray ),
//		16,	// max elements
//		SendPropFloat( NULL, 0, 0, 0, SPROP_NOSCALE )
//		)
//
class SendPropUtlVector : public SendProp {
public:
	SendPropUtlVector(){}
	SendPropUtlVector(
		char* pVarName,		// Use SENDINFO_UTLVECTOR to generate these first 4 parameters.
		int offset,
		int sizeofVar,
		EnsureCapacityFn ensureFn,

		int nMaxElements,			// Max # of elements in the array. Keep this as low as possible.
		SendProp&& pArrayProp,		// Describe the data inside of each element in the array.
		SendTableProxyFn varProxy = SendProxy_DataTableToDataTable	// This can be overridden to control who the array is sent to.
	);
	virtual ~SendPropUtlVector() {}
	SendPropUtlVector& operator=(const SendPropUtlVector& srcSendProp) {
		SendProp::operator=(srcSendProp);
		return *this;
	}
	operator SendProp* () {
		SendPropUtlVector* pSendProp = new SendPropUtlVector;
		*pSendProp = *this;
		return pSendProp;
	}
};

// This gets associated with SendProps inside a utlvector and stores extra data needed to make it work.
class CSendPropExtra_UtlVector
{
public:
	CSendPropExtra_UtlVector() :
		m_DataTableProxyFn(NULL),
		m_ProxyFn(NULL),
		m_EnsureCapacityFn(NULL),
		m_ElementStride(0),
		m_Offset(0),
		m_nMaxElements(0)
	{
	}

	SendTableProxyFn m_DataTableProxyFn;	// If it's a datatable, then this is the proxy they specified.
	SendVarProxyFn m_ProxyFn;				// If it's a non-datatable, then this is the proxy they specified.
	EnsureCapacityFn m_EnsureCapacityFn;
	int m_ElementStride;					// Distance between each element in the array.
	int m_Offset;							// # bytes from the parent structure to its utlvector.
	int m_nMaxElements;						// For debugging...
};

#endif	// DT_UTLVECTOR_SEND_H
