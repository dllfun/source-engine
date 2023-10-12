//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef DATATABLE_SEND_H
#define DATATABLE_SEND_H

#ifdef _WIN32
#pragma once
#endif

#include "dt_common.h"
#include "tier0/dbg.h"
#include "const.h"
#include "bitvec.h"
#include "tier1/UtlStringMap.h"
#include "bitbuf.h"
#include "utlmemory.h"
#include "utlmap.h"

// ------------------------------------------------------------------------ //
// Send proxies can be used to convert a variable into a networkable type 
// (a good example is converting an edict pointer into an integer index).

// These allow you to translate data. For example, if you had a user-entered 
// string number like "10" (call the variable pUserStr) and wanted to encode 
// it as an integer, you would use a SendPropInt32 and write a proxy that said:
// pOut->m_Int = atoi(pUserStr);

// pProp       : the SendProp that has the proxy
// pStructBase : the base structure (like CBaseEntity*).
// pData       : the address of the variable to proxy.
// pOut        : where to output the proxied value.
// iElement    : the element index if this data is part of an array (or 0 if not).
// objectID    : entity index for debugging purposes.

// Return false if you don't want the engine to register and send a delta to
// the clients for this property (regardless of whether it actually changed or not).
// ------------------------------------------------------------------------ //
typedef void (*SendVarProxyFn)(const SendProp* pProp, const void* pStructBase, const void* pData, DVariant* pOut, int iElement, int objectID);
typedef unsigned int CRC32_t;

// Return the pointer to the data for the datatable.
// If the proxy returns null, it's the same as if pRecipients->ClearAllRecipients() was called.
class CSendProxyRecipients;

typedef void* (*SendTableProxyFn)(
	const SendProp* pProp,
	const void* pStructBase,
	const void* pData,
	CSendProxyRecipients* pRecipients,
	int objectID);
extern char* COM_StringCopy(const char* in);

class CNonModifiedPointerProxy
{
public:
	CNonModifiedPointerProxy(SendTableProxyFn fn);

public:

	SendTableProxyFn m_Fn;
	CNonModifiedPointerProxy* m_pNext;
};


// This tells the engine that the send proxy will not modify the pointer
// - it only plays with the recipients. This must be set on proxies that work
// this way, otherwise the engine can't track which properties changed
// in NetworkStateChanged().
#define REGISTER_SEND_PROXY_NON_MODIFIED_POINTER( sendProxyFn ) static CNonModifiedPointerProxy __proxy_##sendProxyFn( sendProxyFn );


class CStandardSendProxiesV1
{
public:
	CStandardSendProxiesV1();

	SendVarProxyFn m_Int8ToInt32;
	SendVarProxyFn m_Int16ToInt32;
	SendVarProxyFn m_Int32ToInt32;

	SendVarProxyFn m_UInt8ToInt32;
	SendVarProxyFn m_UInt16ToInt32;
	SendVarProxyFn m_UInt32ToInt32;

	SendVarProxyFn m_FloatToFloat;
	SendVarProxyFn m_VectorToVector;

#ifdef SUPPORTS_INT64
	SendVarProxyFn m_Int64ToInt64;
	SendVarProxyFn m_UInt64ToInt64;
#endif
};

class CStandardSendProxies : public CStandardSendProxiesV1
{
public:
	CStandardSendProxies();

	SendTableProxyFn m_DataTableToDataTable;
	SendTableProxyFn m_SendLocalDataTable;
	CNonModifiedPointerProxy** m_ppNonModifiedPointerProxies;
};

extern CStandardSendProxies g_StandardSendProxies;


// Max # of datatable send proxies you can have in a tree.
#define MAX_DATATABLE_PROXIES	32


class CStandardSendProxies;


#define MAX_DELTABITS_SIZE 2048

// ------------------------------------------------------------------------ //
// Datatable send proxies are used to tell the engine where the datatable's 
// data is and to specify which clients should get the data. 
//
// pRecipients is the object that allows you to specify which clients will
// receive the data.
// ------------------------------------------------------------------------ //
class CSendProxyRecipients
{
public:
	void	SetAllRecipients();					// Note: recipients are all set by default when each proxy is called.
	void	ClearAllRecipients();

	void	SetRecipient(int iClient);		// Note: these are CLIENT indices, not entity indices (so the first player's index is 0).
	void	ClearRecipient(int iClient);

	// Clear all recipients and set only the specified one.
	void	SetOnly(int iClient);

public:
	// Make sure we have enough room for the max possible player count
	CBitVec< ABSOLUTE_PLAYER_LIMIT >	m_Bits;
};

inline void CSendProxyRecipients::SetAllRecipients()
{
	m_Bits.SetAll();
}

inline void CSendProxyRecipients::ClearAllRecipients()
{
	m_Bits.ClearAll();
}

inline void CSendProxyRecipients::SetRecipient(int iClient)
{
	m_Bits.Set(iClient);
}

inline void	CSendProxyRecipients::ClearRecipient(int iClient)
{
	m_Bits.Clear(iClient);
}

inline void CSendProxyRecipients::SetOnly(int iClient)
{
	m_Bits.ClearAll();
	m_Bits.Set(iClient);
}



// ------------------------------------------------------------------------ //
// ArrayLengthSendProxies are used when you want to specify an array's length
// dynamically.
// ------------------------------------------------------------------------ //
typedef int (*ArrayLengthSendProxyFn)(const void* pStruct, int objectID);



class RecvProp;
class SendTable;
class CSendTablePrecalc;


// -------------------------------------------------------------------------------------------------------------- //
// SendProp.
// -------------------------------------------------------------------------------------------------------------- //

// If SendProp::GetDataTableProxyIndex() returns this, then the proxy is one that always sends
// the data to all clients, so we don't need to store the results.
#define DATATABLE_PROXY_INDEX_NOPROXY	255
#define DATATABLE_PROXY_INDEX_INVALID	254

class SendProp
{
public:
	SendProp();
	virtual				~SendProp();
	SendProp(const SendProp& srcSendProp);
	SendProp& operator=(const SendProp& srcSendProp);

	void				Clear();

	int					GetOffset() const;
	void				SetOffset(int i);

	SendVarProxyFn		GetProxyFn() const;
	void				SetProxyFn(SendVarProxyFn f);

	SendTableProxyFn	GetDataTableProxyFn() const;
	void				SetDataTableProxyFn(SendTableProxyFn f);

	const char* GetDataTableName() const;
	void				SetDataTableName(const char* pTableName);

	SendTable* GetDataTable() const;
	void				SetDataTable(SendTable* pTable);

	char const* GetExcludeDTName() const;

	// If it's one of the numbered "000", "001", etc properties in an array, then
	// these can be used to get its array property name for debugging.
	const char* GetParentArrayPropName() const;
	void				SetParentArrayPropName(char* pArrayPropName);

	const char* GetName() const;

	bool				IsSigned() const;

	bool				IsExcludeProp() const;

	bool				IsInsideArray() const;	// Returns true if SPROP_INSIDEARRAY is set.
	void				SetInsideArray();

	// Arrays only.
	void				SetArrayProp(SendProp* pProp);
	SendProp* GetArrayProp() const;

	// Arrays only.
	void					SetArrayLengthProxy(ArrayLengthSendProxyFn fn);
	ArrayLengthSendProxyFn	GetArrayLengthProxy() const;

	int					GetNumElements() const;
	void				SetNumElements(int nElements);

	// Return the # of bits to encode an array length (must hold GetNumElements()).
	int					GetNumArrayLengthBits() const;

	int					GetElementStride() const;

	SendPropType		GetType() const;

	int					GetFlags() const;
	void				SetFlags(int flags);

	// Some property types bind more data to the SendProp in here.
	const void* GetExtraData() const;
	void				SetExtraData(const void* pData);

public:

	RecvProp* m_pMatchingRecvProp = NULL;	// This is temporary and only used while precalculating
	// data for the decoders.

	SendPropType	m_Type = DPT_Int;
	int				m_nBits = 0;
	float			m_fLowValue = 0;
	float			m_fHighValue = 0;

	SendProp* m_pArrayProp = NULL;					// If this is an array, this is the property that defines each array element.
	ArrayLengthSendProxyFn	m_ArrayLengthProxy = NULL;	// This callback returns the array length.

	int				m_nElements = 0;		// Number of elements in the array (or 1 if it's not an array).
	int				m_ElementStride = 0;	// Pointer distance between array elements.

	const char* m_pExcludeDTName = NULL;			// If this is an exclude prop, then this is the name of the datatable to exclude a prop from.
	const char* m_pParentArrayPropName = NULL;

	const char* m_pVarName = NULL;
	float			m_fHighLowMul = 0;

private:

	int					m_Flags = 0;				// SPROP_ flags.

	SendVarProxyFn		m_ProxyFn = NULL;				// NULL for DPT_DataTable.
	SendTableProxyFn	m_DataTableProxyFn = NULL;		// Valid for DPT_DataTable.

	const char* m_pDataTableName = NULL;
	SendTable* m_pDataTable = NULL;

	// SENDPROP_VECTORELEM makes this negative to start with so we can detect that and
	// set the SPROP_IS_VECTOR_ELEM flag.
	int					m_Offset = 0;

	// Extra data bound to this property.
	const void* m_pExtraData = NULL;
};


inline int SendProp::GetOffset() const
{
	return m_Offset;
}

inline void SendProp::SetOffset(int i)
{
	m_Offset = i;
}

inline SendVarProxyFn SendProp::GetProxyFn() const
{
	Assert(m_Type != DPT_DataTable);
	return m_ProxyFn;
}

inline void SendProp::SetProxyFn(SendVarProxyFn f)
{
	m_ProxyFn = f;
}

inline SendTableProxyFn SendProp::GetDataTableProxyFn() const
{
	Assert(m_Type == DPT_DataTable);
	return m_DataTableProxyFn;
}

inline void SendProp::SetDataTableProxyFn(SendTableProxyFn f)
{
	m_DataTableProxyFn = f;
}

inline const char* SendProp::GetDataTableName() const {
	return m_pDataTableName;
}

inline void SendProp::SetDataTableName(const char* pTableName)
{
	m_pDataTableName = pTableName;
}

inline SendTable* SendProp::GetDataTable() const
{
	return m_pDataTable;
}

inline char const* SendProp::GetExcludeDTName() const
{
	return m_pExcludeDTName;
}

inline const char* SendProp::GetParentArrayPropName() const
{
	return m_pParentArrayPropName;
}

inline void	SendProp::SetParentArrayPropName(char* pArrayPropName)
{
	Assert(!m_pParentArrayPropName);
	m_pParentArrayPropName = pArrayPropName;
}

inline const char* SendProp::GetName() const
{
	return m_pVarName;
}


inline bool SendProp::IsSigned() const
{
	return !(m_Flags & SPROP_UNSIGNED);
}

inline bool SendProp::IsExcludeProp() const
{
	return (m_Flags & SPROP_EXCLUDE) != 0;
}

inline bool	SendProp::IsInsideArray() const
{
	return (m_Flags & SPROP_INSIDEARRAY) != 0;
}

inline void SendProp::SetInsideArray()
{
	m_Flags |= SPROP_INSIDEARRAY;
}

inline void SendProp::SetArrayProp(SendProp* pProp)
{
	m_pArrayProp = pProp;
}

inline SendProp* SendProp::GetArrayProp() const
{
	return m_pArrayProp;
}

inline void SendProp::SetArrayLengthProxy(ArrayLengthSendProxyFn fn)
{
	m_ArrayLengthProxy = fn;
}

inline ArrayLengthSendProxyFn SendProp::GetArrayLengthProxy() const
{
	return m_ArrayLengthProxy;
}

inline int SendProp::GetNumElements() const
{
	return m_nElements;
}

inline void SendProp::SetNumElements(int nElements)
{
	m_nElements = nElements;
}

inline int SendProp::GetElementStride() const
{
	return m_ElementStride;
}

inline SendPropType SendProp::GetType() const
{
	return m_Type;
}

inline int SendProp::GetFlags() const
{
	return m_Flags;
}

inline void SendProp::SetFlags(int flags)
{
	// Make sure they're using something from the valid set of flags.
	Assert(!(flags & ~((1 << SPROP_NUMFLAGBITS) - 1)));
	m_Flags = flags;
}

inline const void* SendProp::GetExtraData() const
{
	return m_pExtraData;
}

// -------------------------------------------------------------------------------------------------------------- //
// SendTable.
// -------------------------------------------------------------------------------------------------------------- //
class SendTableManager;
class CDTISendTable;

class CFastLocalTransferPropInfo
{
public:
	unsigned short	m_iRecvOffset;
	unsigned short	m_iSendOffset;
	unsigned short	m_iProp;
};


class CFastLocalTransferInfo
{
public:
	CUtlVector<CFastLocalTransferPropInfo> m_FastInt32;
	CUtlVector<CFastLocalTransferPropInfo> m_FastInt16;
	CUtlVector<CFastLocalTransferPropInfo> m_FastInt8;
	CUtlVector<CFastLocalTransferPropInfo> m_FastVector;
	CUtlVector<CFastLocalTransferPropInfo> m_OtherProps;	// Props that must be copied slowly (proxies and all).
};

class SendTable
{
	friend class SendTableManager;
public:

	typedef SendProp PropType;

	SendTable();
	SendTable(SendProp** pProps, int nProps, const char* pNetTableName);
	~SendTable();

	void		Construct(SendProp** pProps, int nProps, const char* pNetTableName);

	SendTable& operator=(const SendTable& srcSendTable);

	virtual void Init() {};

	const char* GetName() const;

	int			GetNumProps() const;
	SendProp* GetProp(int i);

	// Used by the engine.
	//bool		IsInitialized() const;
	//void		SetInitialized(bool bInitialized);

	// Used by the engine while writing info into the signon.
	void		SetWriteFlag(bool bHasBeenWritten);
	bool		GetWriteFlag() const;

	bool		HasPropsEncodedAgainstTickCount() const;
	void		SetHasPropsEncodedAgainstTickcount(bool bState);

	bool		IsLeaf() {
		return m_bIsLeaf;
	}

	void		InitRefSendTable(SendTableManager* pSendTableNanager);

	bool		SendTable_InitTable();

	void		SendTable_TermTable();

	// Return the number of unique properties in the table.
	int			SendTable_GetNumFlatProps();

	// compares properties and writes delta properties
	int SendTable_WriteAllDeltaProps(
		const void* pFromData,
		const int	nFromDataBits,
		const void* pToData,
		const int nToDataBits,
		const int nObjectID,
		bf_write* pBufOut);

	// Write the properties listed in pCheckProps and nCheckProps.
	void SendTable_WritePropList(
		const void* pState,
		const int nBits,
		bf_write* pOut,
		const int objectID,
		const int* pCheckProps,
		const int nCheckProps
	);

	//
	// Writes the property indices that must be written to move from pFromState to pToState into pDeltaProps.
	// Returns the number of indices written to pDeltaProps.
	//
	int	SendTable_CalcDelta(
		const void* pFromState,
		const int nFromBits,

		const void* pToState,
		const int nToBits,

		int* pDeltaProps,
		int nMaxDeltaProps,

		const int objectID);


	// This function takes the list of property indices in startProps and the values from
	// SendProxies in pProxyResults, and fills in a new array in outProps with the properties
	// that the proxies want to allow for iClient's client.
	//
	// If pOldStateProxies is non-null, this function adds new properties into the output list
	// if a proxy has turned on from the previous state.
	int SendTable_CullPropsFromProxies(
		const int* pStartProps,
		int nStartProps,

		const int iClient,

		const CSendProxyRecipients* pOldStateProxies,
		const int nOldStateProxies,

		const CSendProxyRecipients* pNewStateProxies,
		const int nNewStateProxies,

		int* pOutProps,
		int nMaxOutProps
	);


	// Encode the properties that are referenced in the delta bits.
	// If pDeltaBits is NULL, then all the properties are encoded.
	bool SendTable_Encode(
		const void* pStruct,
		bf_write* pOut,
		int objectID = -1,
		CUtlMemory<CSendProxyRecipients>* pRecipients = NULL,	// If non-null, this is an array of CSendProxyRecipients.
		// The array must have room for pTable->GetNumDataTableProxies().
		bool bNonZeroOnly = false								// If this is true, then it will write all properties that have
		// nonzero values.
	);

	// do all kinds of checks on a packed entity bitbuffer
	bool SendTable_CheckIntegrity(const void* pData, const int nDataBits);
public:

	SendProp* m_pProps = NULL;
	int			m_nProps = 0;

	const char* m_pNetTableName = NULL;	// The name matched between client and server.

	// The engine hooks the SendTable here.
	//CSendTablePrecalc	*m_pPrecalc = NULL;


protected:
	bool		m_bInited = false;
	bool		m_bIsLeaf = true;
	bool        m_RefTableInited = false;
	//bool		m_bInitialized : 1;
	bool		m_bHasBeenWritten : 1;
	bool		m_bHasPropsEncodedAgainstCurrentTickCount : 1; // m_flSimulationTime and m_flAnimTime, e.g.
public:
	SendTable* m_pNext = NULL;


	int				GetNumChildren() const;
	SendTable*		GetChild(int i) const;


	// Returns true if the specified prop is in this node or any of its children.
	bool			IsPropInRecursiveProps(int i) const;

	// Each datatable property (without SPROP_PROXY_ALWAYS_YES set) gets a unique index here.
	// The engine stores arrays of CSendProxyRecipients with the results of the proxies and indexes the results
	// with this index.
	//
	// Returns DATATABLE_PROXY_INDEX_NOPROXY if the property has SPROP_PROXY_ALWAYS_YES set.
	unsigned short	GetDataTableProxyIndex() const;
	void			SetDataTableProxyIndex(unsigned short val);

	// Similar to m_DataTableProxyIndex, but doesn't use DATATABLE_PROXY_INDEX_INVALID,
	// so this can be used to index CDataTableStack::m_pProxies. 
	unsigned short	GetRecursiveProxyIndex() const;
	void			SetRecursiveProxyIndex(unsigned short val);


public:

	bool					m_bFlatPropInited = false;
	// Child datatables.
	CUtlVector<SendTable*>*	m_Children = NULL;

	// The datatable property that leads us to this CSendNode.
	// This indexes the CSendTablePrecalc or CRecvDecoder's m_DatatableProps list.
	// The root CSendNode sets this to -1.
	short					m_iFlatDatatableProp = -1;

	// The SendTable that this node represents.
	// ALL CSendNodes have this.
	//const SendTable* m_pTable;

	//
	// Properties in this table.
	//

	// m_iFirstRecursiveProp to m_nRecursiveProps defines the list of propertise
	// of this node and all its children.
	unsigned short	m_iFirstRecursiveProp = 0;
	unsigned short	m_nRecursiveProps = 0;


	// See GetDataTableProxyIndex().
	unsigned short	m_DataTableProxyIndex = 0;

	// See GetRecursiveProxyIndex().
	unsigned short	m_RecursiveProxyIndex = 0;

	// This function builds the flat property array given a SendTable.
	bool				SetupFlatPropertyArray();

	int					GetNumFlatProps() const;
	const SendProp* GetFlatProp(int i) const;

	int					GetNumFlatDatatableProps() const;
	const SendProp* GetFlatDatatableProp(int i) const;

	//SendTable* GetSendTable() const;
	//SendTable* GetRootNode();

	int			GetNumDataTableProxies() const;
	void		SetNumDataTableProxies(int count);


public:

	class CProxyPathEntry
	{
	public:
		unsigned short m_iDatatableProp;	// Lookup into CSendTablePrecalc or CRecvDecoder::m_DatatableProps.
		unsigned short m_iProxy;
	};
	class CProxyPath
	{
	public:
		unsigned short m_iFirstEntry;	// Index into m_ProxyPathEntries.
		unsigned short m_nEntries;
	};

	CUtlVector<CProxyPathEntry>* m_ProxyPathEntries = NULL;	// For each proxy index, this is all the DT proxies that generate it.
	CUtlVector<CProxyPath>* m_ProxyPaths = NULL;			// CProxyPathEntries lookup into this.

	// These are what CSendNodes reference.
	// These are actual data properties (ints, floats, etc).
	CUtlVector<const SendProp*>*	m_FlatProps = NULL;

	// Each datatable in a SendTable's tree gets a proxy index, and its properties reference that.
	CUtlVector<unsigned char>* m_FlatPropProxyIndices = NULL;

	// CSendNode::m_iDatatableProp indexes this.
	// These are the datatable properties (SendPropDataTable).
	CUtlVector<const SendProp*>*	m_FlatDatatableProps = NULL;

	// This is the property hierarchy, with the nodes indexing m_Props.
	//SendTable*				m_Root;

	// From whence we came.
	//SendTable* m_pSendTable;

	// For instrumentation.
	CDTISendTable* m_pDTITable = NULL;

	// This is precalculated in single player to allow faster direct copying of the entity data
	// from the server entity to the client entity.
	CFastLocalTransferInfo	m_FastLocalTransfer;

	// This tells how many data table properties there are without SPROP_PROXY_ALWAYS_YES.
	// Arrays allocated with this size can be indexed by CSendNode::GetDataTableProxyIndex().
	int						m_nDataTableProxies = 0;

	// Map prop offsets to indices for properties that can use it.
	CUtlMap<unsigned short, unsigned short>* m_PropOffsetToIndexMap = NULL;

};


inline const char* SendTable::GetName() const
{
	return m_pNetTableName;
}


inline int SendTable::GetNumProps() const
{
	return m_nProps;
}


inline SendProp* SendTable::GetProp(int i)
{
	Assert(i >= 0 && i < m_nProps);
	return &m_pProps[i];
}


//inline bool SendTable::IsInitialized() const
//{
//	return m_bInitialized;
//}
//
//
//inline void SendTable::SetInitialized(bool bInitialized)
//{
//	m_bInitialized = bInitialized;
//}


inline bool SendTable::GetWriteFlag() const
{
	return m_bHasBeenWritten;
}


inline void SendTable::SetWriteFlag(bool bHasBeenWritten)
{
	m_bHasBeenWritten = bHasBeenWritten;
}

inline bool SendTable::HasPropsEncodedAgainstTickCount() const
{
	return m_bHasPropsEncodedAgainstCurrentTickCount;
}

inline void SendTable::SetHasPropsEncodedAgainstTickcount(bool bState)
{
	m_bHasPropsEncodedAgainstCurrentTickCount = bState;
}

inline int SendTable::GetNumChildren() const
{
	return m_Children->Count();
}

inline SendTable* SendTable::GetChild(int i) const
{
	return (*m_Children)[i];
}


inline bool SendTable::IsPropInRecursiveProps(int i) const
{
	int index = i - (int)m_iFirstRecursiveProp;
	return index >= 0 && index < m_nRecursiveProps;
}

inline unsigned short SendTable::GetDataTableProxyIndex() const
{
	Assert(m_DataTableProxyIndex != DATATABLE_PROXY_INDEX_INVALID);	// Make sure it's been set before.
	return m_DataTableProxyIndex;
}

inline void SendTable::SetDataTableProxyIndex(unsigned short val)
{
	m_DataTableProxyIndex = val;
}

inline unsigned short SendTable::GetRecursiveProxyIndex() const
{
	return m_RecursiveProxyIndex;
}

inline void SendTable::SetRecursiveProxyIndex(unsigned short val)
{
	m_RecursiveProxyIndex = val;
}

inline int SendTable::GetNumFlatProps() const
{
	return m_FlatProps->Count();
}

inline const SendProp* SendTable::GetFlatProp(int i) const
{
	return (*m_FlatProps)[i];
}

inline int SendTable::GetNumFlatDatatableProps() const
{
	return m_FlatDatatableProps->Count();
}

inline const SendProp* SendTable::GetFlatDatatableProp(int i) const
{
	return (*m_FlatDatatableProps)[i];
}

//inline SendTable* SendTable::GetSendTable() const
//{
//	return m_pSendTable;
//}
//
//inline SendTable* SendTable::GetRootNode()
//{
//	return m_Root;
//}

inline int SendTable::GetNumDataTableProxies() const
{
	return m_nDataTableProxies;
}


inline void SendTable::SetNumDataTableProxies(int count)
{
	m_nDataTableProxies = count;
}

class SendTableManager {
public:
	SendTableManager() {

	}
	~SendTableManager() {
		m_SendTableMap.Clear();
	}
	int		GetSendTablesCount() {
		return m_SendTableMap.GetNumStrings();
	}

	SendTable* FindSendTable(const char* pName) {
		if (m_SendTableMap.Defined(pName)) {
			return m_SendTableMap[pName];
		}
		return NULL;
	}

	SendTable* GetSendTableHead() {
		return m_pSendTableHead;
	}

	SendTable* RegisteSendTable(SendTable* pSendTable);

	void	ClearAllSendTables() {
		m_Inited = false;
		m_SendTableCRC = 0;
		m_pSendTableHead = NULL;
		m_SendTableMap.PurgeAndDeleteElements();
	}
	// ------------------------------------------------------------------------ //
// SendTable functions.
// ------------------------------------------------------------------------ //

// Precalculate data that enables the SendTable to be used to encode data.
	bool		SendTable_Init();//SendTable** pTables, int nTables
	void		SendTable_PrintStats(void);
	void		SendTable_Term();
	CRC32_t SendTable_ComputeCRC();
	CRC32_t		SendTable_GetCRC();
	int			SendTable_GetNum();
	SendTable* SendTabe_GetTable(int index);

private:
	bool		m_Inited = false;
	CRC32_t		m_SendTableCRC = 0;
	SendTable* m_pSendTableHead = NULL;
	//CUtlVector< SendTable* > g_SendTables;
	CUtlStringMap< SendTable* >	m_SendTableMap;
};

SendTableManager* GetSendTableManager();
// ------------------------------------------------------------------------------------------------------ //
// Use BEGIN_SEND_TABLE if you want to declare a SendTable and have it inherit all the properties from
// its base class. There are two requirements for this to work:

// 1. Its base class must have a static SendTable pointer member variable called m_pClassSendTable which
//    points to its send table. The DECLARE_SERVERCLASS and IMPLEMENT_SERVERCLASS macros do this automatically.

// 2. Your class must typedef its base class as BaseClass. So it would look like this:
//    class Derived : public CBaseEntity
//    {
//    typedef CBaseEntity BaseClass;
//    };

// If you don't want to interit a base class's properties, use BEGIN_SEND_TABLE_NOBASE.
// ------------------------------------------------------------------------------------------------------ //

//#define BEGIN_SEND_TABLE_NOBASE(className, tableName) \
//	template <typename T> int ServerClassInit(T*); \
//	namespace tableName { \
//		struct ignored; \
//	} \
//	template <> int ServerClassInit<tableName::ignored>(tableName::ignored *); \
//	namespace tableName { \
//		SendTable g_SendTable;\
//		int g_SendTableInit = ServerClassInit((tableName::ignored *)NULL); \
//	} \
//	template <> int ServerClassInit<tableName::ignored>(tableName::ignored *) \
//	{ \
//		typedef className currentSendDTClass; \
//		static const char *g_pSendTableName = #tableName; \
//		SendTable &sendTable = tableName::g_SendTable; \
//		static SendProp g_SendProps[] = { \
//			SendPropInt("should_never_see_this", 0, sizeof(int)),		// It adds a dummy property at the start so you can define "empty" SendTables.
//
//
//
//#define END_SEND_TABLE() \
//		};\
//		sendTable.Construct(g_SendProps+1, sizeof(g_SendProps) / sizeof(SendProp) - 1, g_pSendTableName);\
//		return 1; \
//	} 

#define DECLARE_SEND_TABLE_ACCESS(tableName)



//#define BEGIN_SEND_TABLE_NOBASE(className, tableName) \
//	class SendTable_##tableName : public SendTable{\
//	public:\
//	\
//		SendTable_##tableName(){ \
//			typedef className currentSendDTClass; \
//			static bool registed = false;\
//			if( !registed ){\
//				registed = true;\
//				static const char *pTableName = #tableName; \
//				static SendProp g_SendProps[] = { \
//						SendPropInt("should_never_see_this", 0, sizeof(int)),
//
//
//#define END_SEND_TABLE(tableName) \
//				};\
//				Construct(g_SendProps+1, sizeof(g_SendProps) / sizeof(SendProp) - 1, pTableName);\
//				GetSendTableManager()->RegisteSendTable(this);\
//			}\
//		}\
//	\
//	};\
//	friend class SendTable_##tableName;\
//	SendTable_##tableName g_SendTable_##tableName;
	//SendTable* g_pSendTable_##tableName = &g_SendTable_##tableName;

#define BEGIN_INIT_SEND_TABLE(className) \
		static void InitSendTable(){\
			static bool inited = false; \
			if( inited ){\
				return;\
			}\
			inited = true;

#define BEGIN_SEND_TABLE_NOBASE(className, tableName) \
			{\
				typedef className currentSendDTClass;\
				const char *pTableName = #tableName; \
				SendProp* pSendProps[] = { \
								SendPropInt("should_never_see_this", 0, sizeof(int)),

#define END_SEND_TABLE(tableName) \
										};\
				SendTable pSendTable;		\
				pSendTable.Construct(pSendProps+1, sizeof(pSendProps) / sizeof(SendProp*) - 1, pTableName);\
				GetSendTableManager()->RegisteSendTable(&pSendTable);\
				for(int i=0;i<sizeof(pSendProps) / sizeof(SendProp*);i++){\
					delete pSendProps[i];\
				}\
			}

#define BEGIN_SEND_TABLE(className, tableName, baseTableName) \
	BEGIN_SEND_TABLE_NOBASE(className, tableName) \
							SendPropDataTable("baseclass", 0, #baseTableName, SendProxy_DataTableToDataTable),

#define END_INIT_SEND_TABLE() \
		}

#define INIT_REFERENCE_SEND_TABLE(className) \
		className::InitSendTable();

// Normal offset of is invalid on non-array-types, this is dubious as hell. The rest of the codebase converted to the
// legit offsetof from the C headers, so we'll use the old impl here to avoid exposing temptation to others
#define _hacky_dtsend_offsetof(s,m)	((size_t)&(((s *)0)->m))

// These can simplify creating the variables.
// Note: currentSendDTClass::MakeANetworkVar_##varName equates to currentSendDTClass. It's
// there as a check to make sure all networked variables use the CNetworkXXXX macros in network_var.h.
#define SENDINFO(varName)					#varName, _hacky_dtsend_offsetof(currentSendDTClass, varName), sizeof(((currentSendDTClass*)0)->varName)
#define SENDINFO_ARRAY(varName)				#varName, _hacky_dtsend_offsetof(currentSendDTClass, varName), sizeof(((currentSendDTClass*)0)->varName[0])
#define SENDINFO_INTERNALARRAY(varName)		sizeof(((currentSendDTClass*)0)->varName)/sizeof(((currentSendDTClass*)0)->varName[0]), sizeof(((currentSendDTClass*)0)->varName[0]), #varName
#define SENDINFO_ARRAY3(varName)			#varName, _hacky_dtsend_offsetof(currentSendDTClass, varName), sizeof(((currentSendDTClass*)0)->varName[0]), sizeof(((currentSendDTClass*)0)->varName)/sizeof(((currentSendDTClass*)0)->varName[0])
#define SENDINFO_ARRAYELEM(varName, i)		#varName "[" #i "]", _hacky_dtsend_offsetof(currentSendDTClass, varName[i]), sizeof(((currentSendDTClass*)0)->varName[0])
#define SENDINFO_NETWORKARRAYELEM(varName, i)#varName "[" #i "]", _hacky_dtsend_offsetof(currentSendDTClass, varName.m_Value[i]), sizeof(((currentSendDTClass*)0)->varName.m_Value[0])

// NOTE: Be VERY careful to specify any other vector elems for the same vector IN ORDER and 
// right after each other, otherwise it might miss the Y or Z component in SP.
//
// Note: this macro specifies a negative offset so the engine can detect it and setup m_pNext
#define SENDINFO_VECTORELEM(varName, i)		#varName "[" #i "]", -(int)_hacky_dtsend_offsetof(currentSendDTClass::MakeANetworkVar_##varName, varName.m_Value[i]), sizeof(((currentSendDTClass*)0)->varName.m_Value[0])

#define SENDINFO_STRUCTELEM(varName)		#varName, _hacky_dtsend_offsetof(currentSendDTClass, varName), sizeof(((currentSendDTClass*)0)->varName.m_Value)
#define SENDINFO_STRUCTARRAYELEM(varName, i)#varName "[" #i "]", _hacky_dtsend_offsetof(currentSendDTClass, varName.m_Value[i]), sizeof(((currentSendDTClass*)0)->varName.m_Value[0])

// Use this when you're not using a CNetworkVar to represent the data you're sending.
#define SENDINFO_NOCHECK(varName)						#varName, _hacky_dtsend_offsetof(currentSendDTClass, varName), sizeof(((currentSendDTClass*)0)->varName)
#define SENDINFO_STRING_NOCHECK(varName)				#varName, _hacky_dtsend_offsetof(currentSendDTClass, varName)
#define SENDINFO_DT(varName)							#varName, _hacky_dtsend_offsetof(currentSendDTClass, varName)
#define SENDINFO_DT_NAME(varName, remoteVarName)		#remoteVarName, _hacky_dtsend_offsetof(currentSendDTClass, varName)
#define SENDINFO_NAME(varName,remoteVarName)			#remoteVarName, _hacky_dtsend_offsetof(currentSendDTClass, varName), sizeof(((currentSendDTClass*)0)->varName)

// ------------------------------------------------------------------------ //
// Built-in proxy types.
// See the definition of SendVarProxyFn for information about these.
// ------------------------------------------------------------------------ //
void SendProxy_QAngles(const SendProp* pProp, const void* pStruct, const void* pData, DVariant* pOut, int iElement, int objectID);
void SendProxy_AngleToFloat(const SendProp* pProp, const void* pStruct, const void* pData, DVariant* pOut, int iElement, int objectID);
void SendProxy_FloatToFloat(const SendProp* pProp, const void* pStruct, const void* pData, DVariant* pOut, int iElement, int objectID);
void SendProxy_VectorToVector(const SendProp* pProp, const void* pStruct, const void* pData, DVariant* pOut, int iElement, int objectID);
void SendProxy_VectorXYToVectorXY(const SendProp* pProp, const void* pStruct, const void* pData, DVariant* pOut, int iElement, int objectID);
#if 0 // We can't ship this since it changes the size of DTVariant to be 20 bytes instead of 16 and that breaks MODs!!!
void SendProxy_QuaternionToQuaternion(const SendProp* pProp, const void* pStruct, const void* pData, DVariant* pOut, int iElement, int objectID);
#endif

void SendProxy_Int8ToInt32(const SendProp* pProp, const void* pStruct, const void* pData, DVariant* pOut, int iElement, int objectID);
void SendProxy_Int16ToInt32(const SendProp* pProp, const void* pStruct, const void* pData, DVariant* pOut, int iElement, int objectID);
void SendProxy_Int32ToInt32(const SendProp* pProp, const void* pStruct, const void* pData, DVariant* pOut, int iElement, int objectID);
#ifdef SUPPORTS_INT64
void SendProxy_Int64ToInt64(const SendProp* pProp, const void* pStruct, const void* pData, DVariant* pOut, int iElement, int objectID);
#endif
void SendProxy_StringToString(const SendProp* pProp, const void* pStruct, const void* pData, DVariant* pOut, int iElement, int objectID);

// pData is the address of a data table.
void* SendProxy_DataTableToDataTable(const SendProp* pProp, const void* pStructBase, const void* pData, CSendProxyRecipients* pRecipients, int objectID);

// pData is the address of a pointer to a data table.
void* SendProxy_DataTablePtrToDataTable(const SendProp* pProp, const void* pStructBase, const void* pData, CSendProxyRecipients* pRecipients, int objectID);

// Used on player entities - only sends the data to the local player (objectID-1).
void* SendProxy_SendLocalDataTable(const SendProp* pProp, const void* pStruct, const void* pVarData, CSendProxyRecipients* pRecipients, int objectID);


// ------------------------------------------------------------------------ //
// Use these functions to setup your data tables.
// ------------------------------------------------------------------------ //
class SendPropFloat : public SendProp {
public:
	SendPropFloat() {}
	SendPropFloat(
		const char* pVarName,		// Variable name.
		int offset,					// Offset into container structure.
		int sizeofVar = SIZEOF_IGNORE,
		int nBits = 32,				// Number of bits to use when encoding.
		int flags = 0,
		float fLowValue = 0.0f,			// For floating point, low and high values.
		float fHighValue = HIGH_DEFAULT,	// High value. If HIGH_DEFAULT, it's (1<<nBits).
		SendVarProxyFn varProxy = SendProxy_FloatToFloat
	);
	virtual ~SendPropFloat() {}
	SendPropFloat& operator=(const SendPropFloat& srcSendProp);
	operator SendProp* () {
		SendPropFloat* pSendProp = new SendPropFloat;
		*pSendProp = *this;
		return pSendProp;
	}
};

class SendPropVector : public SendProp {
public:
	SendPropVector() {}
	SendPropVector(
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,
		int nBits = 32,					// Number of bits (for each floating-point component) to use when encoding.
		int flags = SPROP_NOSCALE,
		float fLowValue = 0.0f,			// For floating point, low and high values.
		float fHighValue = HIGH_DEFAULT,	// High value. If HIGH_DEFAULT, it's (1<<nBits).
		SendVarProxyFn varProxy = SendProxy_VectorToVector
	);
	virtual ~SendPropVector() {}
	SendPropVector& operator=(const SendPropVector& srcSendProp);
	operator SendProp* () {
		SendPropVector* pSendProp = new SendPropVector;
		*pSendProp = *this;
		return pSendProp;
	}
};

class  SendPropVectorXY : public SendProp {
public:
	SendPropVectorXY() {}
	SendPropVectorXY(
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,
		int nBits = 32,					// Number of bits (for each floating-point component) to use when encoding.
		int flags = SPROP_NOSCALE,
		float fLowValue = 0.0f,			// For floating point, low and high values.
		float fHighValue = HIGH_DEFAULT,	// High value. If HIGH_DEFAULT, it's (1<<nBits).
		SendVarProxyFn varProxy = SendProxy_VectorXYToVectorXY
	);
	virtual ~SendPropVectorXY() {}
	SendPropVectorXY& operator=(const SendPropVectorXY& srcSendProp);
	operator SendProp* () {
		SendPropVectorXY* pSendProp = new SendPropVectorXY;
		*pSendProp = *this;
		return pSendProp;
	}
};

#if 0 // We can't ship this since it changes the size of DTVariant to be 20 bytes instead of 16 and that breaks MODs!!!
SendProp SendPropQuaternion(
	const char* pVarName,
	int offset,
	int sizeofVar = SIZEOF_IGNORE,
	int nBits = 32,					// Number of bits (for each floating-point component) to use when encoding.
	int flags = SPROP_NOSCALE,
	float fLowValue = 0.0f,			// For floating point, low and high values.
	float fHighValue = HIGH_DEFAULT,	// High value. If HIGH_DEFAULT, it's (1<<nBits).
	SendVarProxyFn varProxy = SendProxy_QuaternionToQuaternion
);
#endif

class SendPropAngle : public SendProp {
public:
	SendPropAngle() {}
	SendPropAngle(
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,
		int nBits = 32,
		int flags = 0,
		SendVarProxyFn varProxy = SendProxy_AngleToFloat
	);
	virtual ~SendPropAngle() {}
	SendPropAngle& operator=(const SendPropAngle& srcSendProp);
	operator SendProp* () {
		SendPropAngle* pSendProp = new SendPropAngle;
		*pSendProp = *this;
		return pSendProp;
	}
};

class SendPropQAngles : public SendProp {
public:
	SendPropQAngles() {}
	SendPropQAngles(
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,
		int nBits = 32,
		int flags = 0,
		SendVarProxyFn varProxy = SendProxy_QAngles
	);
	virtual ~SendPropQAngles() {}
	SendPropQAngles& operator=(const SendPropQAngles& srcSendProp);
	operator SendProp* () {
		SendPropQAngles* pSendProp = new SendPropQAngles;
		*pSendProp = *this;
		return pSendProp;
	}
};

class SendPropInt : public SendProp {
public:
	SendPropInt() {}
	SendPropInt(
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,	// Handled by SENDINFO macro.
		int nBits = -1,					// Set to -1 to automatically pick (max) number of bits based on size of element.
		int flags = 0,
		SendVarProxyFn varProxy = 0
	);
	virtual ~SendPropInt() {}
	SendPropInt& operator=(const SendPropInt& srcSendProp);
	operator SendProp* () {
		SendPropInt* pSendProp = new SendPropInt;
		*pSendProp = *this;
		return pSendProp;
	}
};

class SendPropModelIndex : public SendPropInt {
public:
	SendPropModelIndex() {}
	SendPropModelIndex(const char* pVarName, int offset, int sizeofVar = SIZEOF_IGNORE)
		:SendPropInt(pVarName, offset, sizeofVar, SP_MODEL_INDEX_BITS, 0)
	{
		
	}
	virtual ~SendPropModelIndex() {}
	SendPropModelIndex& operator=(const SendPropModelIndex& srcSendProp) {
		SendProp::operator=(srcSendProp);
		return *this;
	}
	operator SendProp* () {
		SendPropModelIndex* pSendProp = new SendPropModelIndex;
		*pSendProp = *this;
		return pSendProp;
	}
};

class SendPropString : public SendProp {
public:
	SendPropString() {}
	SendPropString(
		const char* pVarName,
		int offset,
		int bufferLen,
		int flags = 0,
		SendVarProxyFn varProxy = SendProxy_StringToString);
	virtual ~SendPropString() {}
	SendPropString& operator=(const SendPropString& srcSendProp);
	operator SendProp* () {
		SendPropString* pSendProp = new SendPropString;
		*pSendProp = *this;
		return pSendProp;
	}
};

class SendPropDataTable : public SendProp {
public:
	SendPropDataTable() {}
	// The data table encoder looks at DVariant::m_pData.
	SendPropDataTable(
		const char* pVarName,
		int offset,
		const char* pTableName,
		SendTableProxyFn varProxy = SendProxy_DataTableToDataTable
	);
	virtual ~SendPropDataTable() {}
	SendPropDataTable& operator=(const SendPropDataTable& srcSendProp);
	operator SendProp* () {
		SendPropDataTable* pSendProp = new SendPropDataTable;
		*pSendProp = *this;
		return pSendProp;
	}
};

class SendPropArray3 : public SendProp {
public:
	SendPropArray3() {}
	SendPropArray3(
		const char* pVarName,
		int offset,
		int sizeofVar,
		int elements,
		SendProp&& pArrayProp,
		SendTableProxyFn varProxy = SendProxy_DataTableToDataTable
	);
	virtual ~SendPropArray3() {}
	SendPropArray3& operator=(const SendPropArray3& srcSendProp);
	operator SendProp* () {
		SendPropArray3* pSendProp = new SendPropArray3;
		*pSendProp = *this;
		return pSendProp;
	}
};

class SendPropInternalArray : public SendProp {
public:
	// Use the macro to let it automatically generate a table name. You shouldn't 
// ever need to reference the table name. If you want to exclude this array, then
// reference the name of the variable in varTemplate.
	SendPropInternalArray() {}
	SendPropInternalArray(
		const int elementCount,
		const int elementStride,
		const char* pName,
		SendProp&& pArrayProp,
		ArrayLengthSendProxyFn proxy = 0
	);
	virtual ~SendPropInternalArray() {}
	SendPropInternalArray& operator=(const SendPropInternalArray& srcSendProp);
	operator SendProp* () {
		SendPropInternalArray* pSendProp = new SendPropInternalArray;
		*pSendProp = *this;
		return pSendProp;
	}
};

// Use this and pass the array name and it will figure out the count and stride automatically.
#define SendPropArray( varTemplate, arrayName )			\
	SendPropVariableLengthArray(						\
		0,												\
		varTemplate,									\
		arrayName )

//
// Use this when you want to send a variable-length array of data but there is no physical array you can point it at.
// You need to provide:
// 1. A proxy function that returns the current length of the array.
// 2. The maximum length the array will ever be.
// 2. A SendProp describing what the elements are comprised of.
// 3. In the SendProp, you'll want to specify a proxy function so you can go grab the data from wherever it is.
// 4. A property name that matches the definition on the client.
//
#define SendPropVirtualArray( arrayLengthSendProxy, maxArrayLength, varTemplate, propertyName )	\
	SendPropInternalArray(								\
		maxArrayLength,									\
		0,												\
		#propertyName,									\
		varTemplate,										\
		arrayLengthSendProxy							\
		)


#define SendPropVariableLengthArray( arrayLengthSendProxy, varTemplate, arrayName )	\
	SendPropInternalArray(								\
		sizeof(((currentSendDTClass*)0)->arrayName) / PROPSIZEOF(currentSendDTClass, arrayName[0]), \
		PROPSIZEOF(currentSendDTClass, arrayName[0]),	\
		#arrayName,										\
		varTemplate,										\
		arrayLengthSendProxy							\
		)

// Use this one to specify the element count and stride manually.
#define SendPropArray2( arrayLengthSendProxy, varTemplate, elementCount, elementStride, arrayName )		\
	SendPropInternalArray( elementCount, elementStride, #arrayName, varTemplate, arrayLengthSendProxy )


class SendPropExclude : public SendProp {
public:
	// Use these to create properties that exclude other properties. This is useful if you want to use most of 
// a base class's datatable data, but you want to override some of its variables.
	SendPropExclude() {}
	SendPropExclude(
		const char* pDataTableName,	// Data table name (given to BEGIN_SEND_TABLE and BEGIN_RECV_TABLE).
		const char* pPropName		// Name of the property to exclude.
	);
	virtual ~SendPropExclude() {}
	SendPropExclude& operator=(const SendPropExclude& srcSendProp);
	operator SendProp* () {
		SendPropExclude* pSendProp = new SendPropExclude;
		*pSendProp = *this;
		return pSendProp;
	}
};

#endif // DATATABLE_SEND_H
