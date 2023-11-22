//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef DATATABLE_RECV_H
#define DATATABLE_RECV_H

#ifdef _WIN32
#pragma once
#endif

#include "dt_common.h"
#include "tier0/dbg.h"
#include "tier1/UtlStringMap.h"
#include "bitbuf.h"
//#include "dt.h"
#include "renamed_recvtable_compat.h"
#include "dt_send.h"


#define ADDRESSPROXY_NONE	-1


class RecvTable;
class RecvProp;


// This is passed into RecvProxy functions.
class CRecvProxyData
{
public:
	const RecvProp	*m_pRecvProp;		// The property it's receiving.

	DVariant		m_Value;			// The value given to you to store.

	int				m_iElement;			// Which array element you're getting.

	int				m_ObjectID;			// The object being referred to.
};


//-----------------------------------------------------------------------------
// pStruct = the base structure of the datatable this variable is in (like C_BaseEntity)
// pOut    = the variable that this this proxy represents (like C_BaseEntity::m_SomeValue).
//
// Convert the network-standard-type value in m_Value into your own format in pStruct/pOut.
//-----------------------------------------------------------------------------
typedef void (*RecvVarProxyFn)( const CRecvProxyData *pData, void *pStruct, void *pOut );

// ------------------------------------------------------------------------ //
// ArrayLengthRecvProxies are optionally used to get the length of the 
// incoming array when it changes.
// ------------------------------------------------------------------------ //
typedef void (*ArrayLengthRecvProxyFn)( void *pStruct, int objectID, int currentArrayLength );


// NOTE: DataTable receive proxies work differently than the other proxies.
// pData points at the object + the recv table's offset.
// pOut should be set to the location of the object to unpack the data table into.
// If the parent object just contains the child object, the default proxy just does *pOut = pData.
// If the parent object points at the child object, you need to dereference the pointer here.
// NOTE: don't ever return null from a DataTable receive proxy function. Bad things will happen.
typedef void (*DataTableRecvVarProxyFn)(const RecvProp *pProp, void **pOut, void *pData, int objectID);


// This is used to fork over the standard proxy functions to the engine so it can
// make some optimizations.
class CStandardRecvProxies
{
public:
	CStandardRecvProxies();

	RecvVarProxyFn m_Int32ToInt8;
	RecvVarProxyFn m_Int32ToInt16;
	RecvVarProxyFn m_Int32ToInt32;
	RecvVarProxyFn m_FloatToFloat;
	RecvVarProxyFn m_VectorToVector;
#ifdef SUPPORTS_INT64
	RecvVarProxyFn m_Int64ToInt64;
#endif
};
extern CStandardRecvProxies g_StandardRecvProxies;


class CRecvDecoder;


class RecvProp
{
// This info comes from the receive data table.
public:
							RecvProp();
	virtual					~RecvProp();
	RecvProp(const RecvProp& srcRecvProp);
	RecvProp&				operator=(const RecvProp& srcRecvProp);

	void					InitArray( int nElements, int elementStride );

	int						GetNumElements() const;
	void					SetNumElements( int nElements );

	int						GetElementStride() const;
	void					SetElementStride( int stride );

	int						GetFlags() const;

	const char*				GetName() const;
	SendPropType			GetType() const;

	const char*				GetDataTableName();
	void					SetDataTableName(const char* pTableName);

	RecvTable*				GetDataTable() const;
	void					SetDataTable( RecvTable *pTable );

	RecvVarProxyFn			GetProxyFn() const;
	void					SetProxyFn( RecvVarProxyFn fn );

	DataTableRecvVarProxyFn	GetDataTableProxyFn() const;
	void					SetDataTableProxyFn( DataTableRecvVarProxyFn fn );

	int						GetOffset() const;
	void					SetOffset( int o );

	// Arrays only.
	RecvProp*				GetArrayProp() const;
	void					SetArrayProp( RecvProp *pProp );

	// Arrays only.
	void					SetArrayLengthProxy( ArrayLengthRecvProxyFn proxy );
	ArrayLengthRecvProxyFn	GetArrayLengthProxy() const;

	bool					IsInsideArray() const;
	void					SetInsideArray();

	// Some property types bind more data to the prop in here.
	const void*			GetExtraData() const;
	void				SetExtraData( const void *pData );

	// If it's one of the numbered "000", "001", etc properties in an array, then
	// these can be used to get its array property name for debugging.
	const char*			GetParentArrayPropName();
	void				SetParentArrayPropName( const char *pArrayPropName );

public:

	const char              *m_pVarName = NULL;
	SendPropType			m_RecvType = DPT_Int;
	int						m_Flags = 0;
	int						m_StringBufferSize = 0;


protected:

	bool					m_bInsideArray = false;		// Set to true by the engine if this property sits inside an array.

	// Extra data that certain special property types bind to the property here.
	const void				*m_pExtraData = NULL;

	// If this is an array (DPT_Array).
	RecvProp				*m_pArrayProp = NULL;
	ArrayLengthRecvProxyFn	m_ArrayLengthProxy = NULL;
	
	RecvVarProxyFn			m_ProxyFn = NULL;
	DataTableRecvVarProxyFn	m_DataTableProxyFn = NULL;	// For RDT_DataTable.

	const char*				m_pDataTableName = NULL;
	RecvTable				*m_pDataTable = NULL;		// For RDT_DataTable.
	int						m_Offset = 0;
	
	int						m_ElementStride = 0;
	int						m_nElements = 0;

	// If it's one of the numbered "000", "001", etc properties in an array, then
	// these can be used to get its array property name for debugging.
	const char				*m_pParentArrayPropName = NULL;
};

class RecvTableManager;

class RecvTable
{
public:

	typedef RecvProp	PropType;

				RecvTable();
				RecvTable( RecvProp **pProps, int nProps, const char *pNetTableName );
				~RecvTable();

	void		Construct( RecvProp **pProps, int nProps, const char *pNetTableName );

	RecvTable& operator=(const RecvTable& srcRecvTable);

	int			GetNumProps();
	RecvProp*	GetProp( int i );

	const char*	GetName();

	// Used by the engine while initializing array props.
	void		SetInitialized( bool bInitialized );
	bool		IsInitialized() const;

	// Used by the engine.
	void		SetInMainList( bool bInList );
	bool		IsInMainList() const;

	bool		IsLeaf() {
		return m_bIsLeaf;
	}

	void		InitRefRecvTable(RecvTableManager* pRecvTableNanager);

	void		RecvTable_InitTable(RecvTableManager* pRecvTableNanager);

	void		RecvTable_TermTable(RecvTableManager* pRecvTableNanager);

	// objectID gets passed into proxies and can be used to track data on particular objects.
// NOTE: this function can ONLY decode a buffer outputted from RecvTable_MergeDeltas
//       or RecvTable_CopyEncoding because if the way it follows the exclude prop bits.
	bool		RecvTable_Decode(
		void* pStruct,
		bf_read* pIn,
		int objectID,
		bool updateDTI = true
	);

	// This acts like a RecvTable_Decode() call where all properties are written and all their values are zero.
	void RecvTable_DecodeZeros(void* pStruct, int objectID);

	// This writes all pNewState properties into pOut.
	//
	// If pOldState is non-null and contains any properties that aren't in pNewState, then
	// those properties are written to the output as well. Returns # of changed props
	int RecvTable_MergeDeltas(
		bf_read* pOldState,		// this can be null
		bf_read* pNewState,

		bf_write* pOut,

		int objectID = -1,

		int* pChangedProps = NULL,

		bool updateDTI = false // enclude merge from newState in the DTI reports
	);


	// Just copies the bits from the bf_read into the bf_write (this function is used
	// when you don't know the length of the encoded data).
	void RecvTable_CopyEncoding(
		bf_read* pIn,
		bf_write* pOut,
		int objectID
	);

public:

	// Properties described in a table.
	RecvProp		*m_pProps = NULL;
	int				m_nProps = 0;

	// The decoder. NOTE: this covers each RecvTable AND all its children (ie: its children
	// will have their own decoders that include props for all their children).
	CRecvDecoder	*m_pDecoder = NULL;

	const char		*m_pNetTableName = NULL;	// The name matched between client and server.


private:
	bool			m_bIsLeaf = true;
	bool			m_RefTableInited = false;
	bool			m_bInitialized;
	bool			m_bInMainList;
public:
	RecvTable* m_pNext = NULL;
};


inline int RecvTable::GetNumProps()
{
	return m_nProps;
}

inline RecvProp* RecvTable::GetProp( int i )
{
	Assert( i >= 0 && i < m_nProps ); 
	return &m_pProps[i]; 
}

inline const char* RecvTable::GetName()
{
	return m_pNetTableName; 
}

inline void RecvTable::SetInitialized( bool bInitialized )
{
	m_bInitialized = bInitialized;
}

inline bool RecvTable::IsInitialized() const
{
	return m_bInitialized;
}

inline void RecvTable::SetInMainList( bool bInList )
{
	m_bInMainList = bInList;
}

inline bool RecvTable::IsInMainList() const
{
	return m_bInMainList;
}

class CStandardSendProxies;
class CClientSendTable;
struct MatchingProp_t
{
	SendProp* m_pProp;
	RecvProp* m_pMatchingRecvProp;

	static bool LessFunc(const MatchingProp_t& lhs, const MatchingProp_t& rhs)
	{
		return lhs.m_pProp < rhs.m_pProp;
	}
};

class RecvTableManager {
public:

	int		GetRecvTablesCount() {
		return m_RecvTableMap.GetNumStrings();
	}

	RecvTable* FindRecvTable(const char* pName) {
		if (m_RecvTableMap.Defined(pName)) {
			return m_RecvTableMap[pName];
		}
		return NULL;
	}

	RecvTable* GetRecvTableHead() {
		return m_pRecvTableHead;
	}

	RecvTable* RegisteRecvTable(RecvTable* pRecvTable);

	// ------------------------------------------------------------------------------------------ //
// RecvTable functions.
// ------------------------------------------------------------------------------------------ //

// These are the module startup and shutdown routines.
	bool		RecvTable_Init();
	void		RecvTable_Term(bool clearall = true);

	//RecvTable* FindRecvTable(const char* pName);

	RecvTable* DataTable_FindRenamedTable(const char* pOldTableName);

	// After ALL the SendTables have been received, call this and it will create CRecvDecoders
	// for all the SendTable->RecvTable matches it finds.
	// Returns false if there is an unrecoverable error.
	//
	// bAllowMismatches is true when playing demos back so we can change datatables without breaking demos.
	// If pAnyMisMatches is non-null, it will be set to true if the client's recv tables mismatched the server's ones.
	bool		RecvTable_CreateDecoders(const CStandardSendProxies* pSendProxies, bool bAllowMismatches, bool* pAnyMismatches = NULL);

	bool DataTable_SetupReceiveTableFromSendTable(SendTable* sendTable, bool bNeedsDecoder);

	//CUtlLinkedList< RecvTable*, unsigned short >& GetRecvTables() {
	//	return g_RecvTables;
	//}
private:
	bool MatchRecvPropsToSendProps_R(CUtlRBTree< MatchingProp_t, unsigned short >& lookup, char const* sendTableName, SendTable* pSendTable, RecvTable* pRecvTable, bool bAllowMismatches, bool* pAnyMismatches);

	//CClientSendTable* FindClientSendTable(const char* pName);

	bool SetupClientSendTableHierarchy();

	SendTableManager m_SendTableManager;
	//CUtlLinkedList< RecvTable*, unsigned short > g_RecvTables;
	CUtlLinkedList< CRecvDecoder*, unsigned short > g_RecvDecoders;
	//CUtlLinkedList< CClientSendTable*, unsigned short > g_ClientSendTables;
	RecvTable* m_pRecvTableHead = NULL;
	CUtlStringMap< RecvTable* >	m_RecvTableMap;
};

RecvTableManager* GetRecvTableManager();

int TestRevFunction();

// ------------------------------------------------------------------------------------------------------ //
// See notes on BEGIN_SEND_TABLE for a description. These macros work similarly.
// ------------------------------------------------------------------------------------------------------ //
//#define BEGIN_RECV_TABLE_NOBASE(className, tableName) \
//	template <typename T> int ClientClassInit(T *); \
//	namespace tableName { \
//		struct ignored; \
//	} \
//	template <> int ClientClassInit<tableName::ignored>(tableName::ignored *); \
//	namespace tableName {	\
//		RecvTable g_RecvTable; \
//		int ccc= TestRevFunction();\
//		int g_RecvTableInit = ClientClassInit((tableName::ignored *)NULL); \
//	} \
//	template <> int ClientClassInit<tableName::ignored>(tableName::ignored *) \
//	{ \
//		typedef className currentRecvDTClass; \
//		const char *pRecvTableName = #tableName; \
//		RecvTable &RecvTable = tableName::g_RecvTable; \
//		static RecvProp RecvProps[] = { \
//			RecvPropInt("should_never_see_this", 0, sizeof(int)),		// It adds a dummy property at the start so you can define "empty" SendTables.
//
//
//#define BEGIN_RECV_TABLE(className, tableName) \
//	BEGIN_RECV_TABLE_NOBASE(className, tableName) \
//		RecvPropDataTable("baseclass", 0, 0, className::BaseClass::m_pClassRecvTable, DataTableRecvProxy_StaticDataTable),
//
//
//#define END_RECV_TABLE() \
//			}; \
//		RecvTable.Construct(RecvProps+1, sizeof(RecvProps) / sizeof(RecvProp) - 1, pRecvTableName); \
//		return 1; \
//	}

//#define BEGIN_INIT_RECV_TABLE(className) 
//
//#define BEGIN_RECV_TABLE_NOBASE(className, tableName) \
//	class RecvTable_##tableName : public RecvTable{\
//	public:\
//	\
//		RecvTable_##tableName(){ \
//			typedef className currentRecvDTClass; \
//			static bool registed = false;\
//			if( !registed ){\
//				registed = true;\
//				static const char *pTableName = #tableName; \
//				static RecvProp g_RecvProps[] = { \
//					RecvPropInt("should_never_see_this", 0, sizeof(int)),		// It adds a dummy property at the start so you can define "empty" SendTables.
//
//
//#define END_RECV_TABLE(tableName) \
//				};\
//				Construct(g_RecvProps+1, sizeof(g_RecvProps) / sizeof(RecvProp) - 1, pTableName);\
//				GetRecvTableManager()->RegisteRecvTable(this);\
//			}\
//		}\
//	\
//	};\
//	friend class RecvTable_##tableName;\
//	RecvTable_##tableName g_RecvTable_##tableName;
//
//#define BEGIN_RECV_TABLE(className, tableName, baseTableName) \
//	BEGIN_RECV_TABLE_NOBASE(className, tableName) \
//		RecvPropDataTable("baseclass", 0, 0, #baseTableName, DataTableRecvProxy_StaticDataTable),
//
//#define INIT_REFERENCE_RECV_TABLE(className) 
//
//#define END_INIT_RECV_TABLE() 


#define BEGIN_INIT_RECV_TABLE(className) \
		static void InitRecvTable(){\
			static bool inited = false; \
			if( inited ){\
				return;\
			}\
			inited = true;

#define BEGIN_RECV_TABLE_NOBASE(className, tableName) \
			{\
				typedef className currentRecvDTClass;\
				const char *pTableName = #tableName; \
				RecvProp* pRecvProps[] = { \
					RecvPropInt((int*)0, "should_never_see_this", 0, sizeof(int)),		// It adds a dummy property at the start so you can define "empty" SendTables.

#define END_RECV_TABLE(tableName) \
										};\
				RecvTable pRecvTable;		\
				pRecvTable.Construct(pRecvProps+1, sizeof(pRecvProps) / sizeof(RecvProp*) - 1, pTableName);\
				GetRecvTableManager()->RegisteRecvTable(&pRecvTable);\
				for(int i=0;i<sizeof(pRecvProps) / sizeof(RecvProp*);i++){\
					delete pRecvProps[i];\
				}\
			};

#define BEGIN_RECV_TABLE(className, tableName, baseTableName) \
	BEGIN_RECV_TABLE_NOBASE(className, tableName) \
		RecvPropDataTable("baseclass", 0, 0, #baseTableName, DataTableRecvProxy_StaticDataTable),

#define END_INIT_RECV_TABLE() \
		}

#define INIT_REFERENCE_RECV_TABLE(className) \
		className::InitRecvTable();

// Normal offset of is invalid on non-array-types, this is dubious as hell. The rest of the codebase converted to the
// legit offsetof from the C headers, so we'll use the old impl here to avoid exposing temptation to others
#define _hacky_dtrecv_offsetof(s,m)	((size_t)&(((s *)0)->m))
#define _hacky_dtrecv_typeof(s,m)	&(((s *)0)->m)

#define RECVINFO(varName)						_hacky_dtrecv_typeof(currentRecvDTClass, varName), #varName, _hacky_dtrecv_offsetof(currentRecvDTClass, varName), sizeof(((currentRecvDTClass*)0)->varName.m_Value)//
#define RECVINFO_NAME(varName, remoteVarName)	_hacky_dtrecv_typeof(currentRecvDTClass, varName), #remoteVarName, _hacky_dtrecv_offsetof(currentRecvDTClass, varName), sizeof(((currentRecvDTClass*)0)->varName.m_Value)//
#define RECVINFO_STRING(varName)				_hacky_dtrecv_typeof(currentRecvDTClass, varName), #varName, _hacky_dtrecv_offsetof(currentRecvDTClass, varName), STRINGBUFSIZE(currentRecvDTClass, varName)
#define RECVINFO_BASECLASS(tableName)			RecvPropDataTable("this", 0, 0, &REFERENCE_RECV_TABLE(tableName))
#define RECVINFO_INTERNALARRAY(varName)			sizeof(((currentRecvDTClass*)0)->varName.m_Value)/sizeof(((currentRecvDTClass*)0)->varName[0]), sizeof(((currentRecvDTClass*)0)->varName[0]), #varName //
#define RECVINFO_ARRAY(varName)					#varName, _hacky_dtrecv_offsetof(currentRecvDTClass, varName.m_Value), sizeof(((currentRecvDTClass*)0)->varName[0]), sizeof(((currentRecvDTClass*)0)->varName.m_Value)/sizeof(((currentRecvDTClass*)0)->varName[0])//
#define RECVINFO_ARRAY3(varName)				_hacky_dtrecv_typeof(currentRecvDTClass, varName[0]), #varName, _hacky_dtrecv_offsetof(currentRecvDTClass, varName.m_Value), sizeof(((currentRecvDTClass*)0)->varName[0])

#define RECVINFO_VECTORELEM(varName, i)		_hacky_dtrecv_typeof(currentRecvDTClass::MakeANetworkVar_##varName, varName.m_Value[i]), #varName "[" #i "]", _hacky_dtrecv_offsetof(currentRecvDTClass::MakeANetworkVar_##varName, varName.m_Value[i]), sizeof(((currentRecvDTClass*)0)->varName.m_Value[0])
#define RECVINFO_VECTORELEM_NAME(varName, i, remoteVarName)		_hacky_dtrecv_typeof(currentRecvDTClass::MakeANetworkVar_##varName, varName.m_Value[i]), #remoteVarName, _hacky_dtrecv_offsetof(currentRecvDTClass::MakeANetworkVar_##varName, varName.m_Value[i]), sizeof(((currentRecvDTClass*)0)->varName.m_Value[0])

#define RECVINFO_STRUCTELEM(varName)		_hacky_dtrecv_typeof(currentRecvDTClass, varName), #varName, _hacky_dtrecv_offsetof(currentRecvDTClass, varName), sizeof(((currentRecvDTClass*)0)->varName.m_Value)
#define RECVINFO_STRUCTARRAYELEM(varName, i)_hacky_dtrecv_typeof(currentRecvDTClass, varName.m_Value[i]), #varName "[" #i "]", _hacky_dtrecv_offsetof(currentRecvDTClass, varName.m_Value[i]), sizeof(((currentRecvDTClass*)0)->varName.m_Value[0])

// Just specify the name and offset. Used for strings and data tables.
#define RECVINFO_NOCHECK(varName)				_hacky_dtrecv_typeof(currentRecvDTClass, varName), #varName, _hacky_dtrecv_offsetof(currentRecvDTClass, varName), sizeof(((currentRecvDTClass*)0)->varName)// .m_Value
#define RECVINFO_NOSIZE(varName)				#varName, _hacky_dtrecv_offsetof(currentRecvDTClass, varName)
#define RECVINFO_DT(varName)					RECVINFO_NOSIZE(varName)
#define RECVINFO_DTNAME(varName,remoteVarName)	#remoteVarName, _hacky_dtrecv_offsetof(currentRecvDTClass, varName)

template<typename T= float>
void RecvProxy_FloatToFloat  ( const CRecvProxyData *pData, void *pStruct, void *pOut );
template<typename T>
void RecvProxy_FloatToFloat(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	Assert(IsFinite(pData->m_Value.m_Float));
	*((T*)pOut) = pData->m_Value.m_Float;//float
}

template<typename T= Vector>
void RecvProxy_VectorToVector( const CRecvProxyData *pData, void *pStruct, void *pOut );
template<typename T>
void RecvProxy_VectorToVector(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	const float* v = pData->m_Value.m_Vector;

	Assert(IsFinite(v[0]) && IsFinite(v[1]) && IsFinite(v[2]));
	((T*)pOut)->SetX(v[0]);//float
	((T*)pOut)->SetY(v[1]);
	((T*)pOut)->SetZ(v[2]);
}

template<typename T= float>
void RecvProxy_VectorXYToVectorXY( const CRecvProxyData *pData, void *pStruct, void *pOut );
template<typename T>
void RecvProxy_VectorXYToVectorXY(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	const float* v = pData->m_Value.m_Vector;

	Assert(IsFinite(v[0]) && IsFinite(v[1]));
	((T*)pOut)[0] = v[0];//float
	((T*)pOut)[1] = v[1];
}

void RecvProxy_QuaternionToQuaternion( const CRecvProxyData *pData, void *pStruct, void *pOut );

template<typename T= unsigned char>
void RecvProxy_Int32ToInt8   ( const CRecvProxyData *pData, void *pStruct, void *pOut );
template<typename T= unsigned short>
void RecvProxy_Int32ToInt16  ( const CRecvProxyData *pData, void *pStruct, void *pOut );
template<typename T= char>
void RecvProxy_StringToString( const CRecvProxyData *pData, void *pStruct, void *pOut );
template<typename T>
void RecvProxy_StringToString(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	T& pStrOut = *(T*)pOut;//char
	if (pData->m_pRecvProp->m_StringBufferSize <= 0)
	{
		return;
}

	for (int i = 0; i < pData->m_pRecvProp->m_StringBufferSize; i++)
	{
		pStrOut[i] = pData->m_Value.m_pString[i];
		if (pStrOut[i] == 0)
			break;
	}

	pStrOut[pData->m_pRecvProp->m_StringBufferSize - 1] = 0;
}
template<typename T= uint32>
void RecvProxy_Int32ToInt32  ( const CRecvProxyData *pData, void *pStruct, void *pOut );
#ifdef SUPPORTS_INT64
template<typename T= int64>
void RecvProxy_Int64ToInt64  ( const CRecvProxyData *pData, void *pStruct, void *pOut );
#endif

template<typename T>
void RecvProxy_Int32ToInt8(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	*((T*)pOut) = pData->m_Value.m_Int;//unsigned char(unsigned char)
}

template<typename T>
void RecvProxy_Int32ToInt16(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	*((T*)pOut) = pData->m_Value.m_Int;//unsigned short(unsigned short)
}

template<typename T>
void RecvProxy_Int32ToInt32(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	*((T*)pOut) = pData->m_Value.m_Int;//uint32(uint32)
}

#ifdef SUPPORTS_INT64
template<typename T>
void RecvProxy_Int64ToInt64(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	*((T*)pOut) = (int64)pData->m_Value.m_Int64;//int64
}
#endif

// StaticDataTable does *pOut = pData.
void DataTableRecvProxy_StaticDataTable(const RecvProp *pProp, void **pOut, void *pData, int objectID);

// PointerDataTable does *pOut = *((void**)pData)   (ie: pData is a pointer to the object to decode into).
void DataTableRecvProxy_PointerDataTable(const RecvProp *pProp, void **pOut, void *pData, int objectID);

class RecvPropFloat : public RecvProp {
public:
	RecvPropFloat() {}

	template<typename T=float>
	RecvPropFloat(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,	// Handled by RECVINFO macro, but set to SIZEOF_IGNORE if you don't want to bother.
		int flags = 0,
		RecvVarProxyFn varProxy = RecvProxy_FloatToFloat<T>
	);
	virtual	~RecvPropFloat() {}
	RecvPropFloat& operator=(const RecvPropFloat& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropFloat* pRecvProp = new RecvPropFloat;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

template<typename T>
RecvPropFloat::RecvPropFloat(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar,
	int flags,
	RecvVarProxyFn varProxy
)
{
	//RecvProp ret;

#ifdef _DEBUG
	if (varProxy == RecvProxy_FloatToFloat)
	{
		Assert(sizeofVar == 0 || sizeofVar == 4);
	}
#endif

	if (pVarName) {
		this->m_pVarName = COM_StringCopy(pVarName);
	}
	this->SetOffset(offset);
	this->m_RecvType = DPT_Float;
	this->m_Flags = flags;
	this->SetProxyFn(varProxy);

	//return ret;
}

class RecvPropVector : public RecvProp {
public:
	RecvPropVector() {}

	template<typename T = Vector>
	RecvPropVector(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,	// Handled by RECVINFO macro, but set to SIZEOF_IGNORE if you don't want to bother.
		int flags = 0,
		RecvVarProxyFn varProxy = RecvProxy_VectorToVector<T>
	);
	virtual	~RecvPropVector() {}
	RecvPropVector& operator=(const RecvPropVector& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropVector* pRecvProp = new RecvPropVector;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

template<typename T>
RecvPropVector::RecvPropVector(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar,
	int flags,
	RecvVarProxyFn varProxy
)
{
	//RecvProp ret;

#ifdef _DEBUG
	if (varProxy == RecvProxy_VectorToVector)
	{
		Assert(sizeofVar == sizeof(Vector));
	}
#endif

	if (pVarName) {
		this->m_pVarName = COM_StringCopy(pVarName);
	}
	this->SetOffset(offset);
	this->m_RecvType = DPT_Vector;
	this->m_Flags = flags;
	this->SetProxyFn(varProxy);

	//return ret;
}

class RecvPropVectorXY : public RecvProp {
public:
	RecvPropVectorXY() {}

	template<typename T= Vector>
	RecvPropVectorXY(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,	// Handled by RECVINFO macro, but set to SIZEOF_IGNORE if you don't want to bother.
		int flags = 0,
		RecvVarProxyFn varProxy = RecvProxy_VectorXYToVectorXY<T>
	);
	virtual	~RecvPropVectorXY() {}
	RecvPropVectorXY& operator=(const RecvPropVectorXY& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropVectorXY* pRecvProp = new RecvPropVectorXY;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

template<typename T>
RecvPropVectorXY::RecvPropVectorXY(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar,
	int flags,
	RecvVarProxyFn varProxy
)
{
	//RecvProp ret;

#ifdef _DEBUG
	if (varProxy == RecvProxy_VectorToVector)
	{
		Assert(sizeofVar == sizeof(Vector));
	}
#endif

	if (pVarName) {
		this->m_pVarName = COM_StringCopy(pVarName);
	}
	this->SetOffset(offset);
	this->m_RecvType = DPT_VectorXY;
	this->m_Flags = flags;
	this->SetProxyFn(varProxy);

	//return ret;
}

// This is here so the RecvTable can look more like the SendTable.
#define RecvPropQAngles RecvPropVector

#if 0 // We can't ship this since it changes the size of DTVariant to be 20 bytes instead of 16 and that breaks MODs!!!

RecvProp RecvPropQuaternion(
	const char *pVarName, 
	int offset, 
	int sizeofVar=SIZEOF_IGNORE,	// Handled by RECVINFO macro, but set to SIZEOF_IGNORE if you don't want to bother.
	int flags=0, 
	RecvVarProxyFn varProxy=RecvProxy_QuaternionToQuaternion
	);
#endif

class RecvPropInt : public RecvProp {
public:
	RecvPropInt() {}

	template<typename T = int>
	RecvPropInt(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,	// Handled by RECVINFO macro, but set to SIZEOF_IGNORE if you don't want to bother.
		int flags = 0,
		RecvVarProxyFn varProxy = 0
	);
	virtual	~RecvPropInt() {}
	RecvPropInt& operator=(const RecvPropInt& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropInt* pRecvProp = new RecvPropInt;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

template<typename T>
RecvPropInt::RecvPropInt(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar,
	int flags,
	RecvVarProxyFn varProxy
)
{
	//RecvProp ret;

	// If they didn't specify a proxy, then figure out what type we're writing to.
	if (varProxy == NULL)
	{
		if (sizeofVar == 1)
		{
			varProxy = RecvProxy_Int32ToInt8<T>;
		}
		else if (sizeofVar == 2)
		{
			varProxy = RecvProxy_Int32ToInt16<T>;
		}
		else if (sizeofVar == 4)
		{
			varProxy = RecvProxy_Int32ToInt32<T>;
		}
#ifdef SUPPORTS_INT64		
		else if (sizeofVar == 8)
		{
			varProxy = RecvProxy_Int64ToInt64<T>;
		}
#endif
		else
		{
			Assert(!"RecvPropInt var has invalid size");
			varProxy = RecvProxy_Int32ToInt8<T>;	// safest one...
		}
	}

	if (pVarName) {
		this->m_pVarName = COM_StringCopy(pVarName);
	}
	this->SetOffset(offset);
#ifdef SUPPORTS_INT64
	this->m_RecvType = (sizeofVar == 8) ? DPT_Int64 : DPT_Int;
#else
	this->m_RecvType = DPT_Int;
#endif
	this->m_Flags = flags;
	this->SetProxyFn(varProxy);

	//return ret;
}

class RecvPropString : public RecvProp {
public:
	RecvPropString(){}

	template<typename T = char*>
	RecvPropString(
		T* pType,
		const char* pVarName,
		int offset,
		int bufferSize,
		int flags = 0,
		RecvVarProxyFn varProxy = RecvProxy_StringToString<T>
	);
	virtual	~RecvPropString() {}
	RecvPropString& operator=(const RecvPropString& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropString* pRecvProp = new RecvPropString;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

template<typename T>
RecvPropString::RecvPropString(
	T* pType,
	const char* pVarName,
	int offset,
	int bufferSize,
	int flags,
	RecvVarProxyFn varProxy
)
{
	//RecvProp ret;

	if (pVarName) {
		this->m_pVarName = COM_StringCopy(pVarName);
	}
	this->SetOffset(offset);
	this->m_RecvType = DPT_String;
	this->m_Flags = flags;
	this->m_StringBufferSize = bufferSize;
	this->SetProxyFn(varProxy);

	//return ret;
}

class RecvPropDataTable : public RecvProp {
public:
	RecvPropDataTable() {}
	RecvPropDataTable(
		const char* pVarName,
		int offset,
		int flags,
		const char* pTableName,
		DataTableRecvVarProxyFn varProxy = DataTableRecvProxy_StaticDataTable
	);
	virtual	~RecvPropDataTable() {}
	RecvPropDataTable& operator=(const RecvPropDataTable& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropDataTable* pRecvProp = new RecvPropDataTable;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

class RecvPropArray3 : public RecvProp {
public:
	RecvPropArray3() {}
	RecvPropArray3(
		const char* pVarName,
		int offset,
		int sizeofVar,
		int elements,
		RecvProp&& pArrayProp,
		DataTableRecvVarProxyFn varProxy = DataTableRecvProxy_StaticDataTable
	);
	virtual	~RecvPropArray3() {}
	RecvPropArray3& operator=(const RecvPropArray3& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropArray3* pRecvProp = new RecvPropArray3;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

class RecvPropInternalArray : public RecvProp {
public:
	// Use the macro to let it automatically generate a table name. You shouldn't 
// ever need to reference the table name. If you want to exclude this array, then
// reference the name of the variable in varTemplate.
	RecvPropInternalArray() {}
	RecvPropInternalArray(
		const int elementCount,
		const int elementStride,
		const char* pName,
		RecvProp&& pArrayProp,
		ArrayLengthRecvProxyFn proxy = 0
	);
	virtual	~RecvPropInternalArray() {}
	RecvPropInternalArray& operator=(const RecvPropInternalArray& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropInternalArray* pRecvProp = new RecvPropInternalArray;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

//
// Use this if you want to completely manage the way the array data is stored.
// You'll need to provide a proxy inside varTemplate that looks for 'iElement'
// to figure out where to store the specified element.
//
#define RecvPropVirtualArray( arrayLengthProxy, maxArrayLength, varTemplate, propertyName ) \
	RecvPropInternalArray( \
		maxArrayLength, \
		0, \
		#propertyName, \
		varTemplate, \
		arrayLengthProxy \
		)


// Use this and pass the array name and it will figure out the count and stride automatically.
#define RecvPropVariableLengthArray( arrayLengthProxy, varTemplate, arrayName )			\
	RecvPropInternalArray(								\
		sizeof(((currentRecvDTClass*)0)->arrayName) / PROPSIZEOF(currentRecvDTClass, arrayName[0]), \
		PROPSIZEOF(currentRecvDTClass, arrayName[0]),	\
		#arrayName,										\
		varTemplate,									\
		arrayLengthProxy								\
		)


// Use this and pass the array name and it will figure out the count and stride automatically.
#define RecvPropArray( varTemplate, arrayName )			\
	RecvPropVariableLengthArray( 0, varTemplate, arrayName )


// Use this one to specify the element count and stride manually.
#define RecvPropArray2( arrayLengthProxy, varTemplate, elementCount, elementStride, arrayName )		\
																		\
	RecvPropInternalArray( elementCount, elementStride, #arrayName, varTemplate, arrayLengthProxy )


// ---------------------------------------------------------------------------------------- //
// Inlines.
// ---------------------------------------------------------------------------------------- //

inline void RecvProp::InitArray( int nElements, int elementStride )
{
	m_RecvType = DPT_Array;
	m_nElements = nElements;
	m_ElementStride = elementStride;
}

inline int RecvProp::GetNumElements() const
{
	return m_nElements;
}

inline void RecvProp::SetNumElements( int nElements )
{
	m_nElements = nElements;
}

inline int RecvProp::GetElementStride() const
{
	return m_ElementStride;
}

inline void RecvProp::SetElementStride( int stride )
{
	m_ElementStride = stride;
}

inline int RecvProp::GetFlags() const
{
	return m_Flags;
}

inline const char* RecvProp::GetName() const
{
	return m_pVarName; 
}

inline SendPropType RecvProp::GetType() const
{
	return m_RecvType; 
}

inline const char* RecvProp::GetDataTableName() {
	return m_pDataTableName;
}

inline void RecvProp::SetDataTableName(const char* pTableName)
{
	m_pDataTableName = pTableName;
}

inline RecvTable* RecvProp::GetDataTable() const 
{
	return m_pDataTable; 
}

inline RecvVarProxyFn RecvProp::GetProxyFn() const 
{
	return m_ProxyFn; 
}

inline void RecvProp::SetProxyFn( RecvVarProxyFn fn )
{
	m_ProxyFn = fn; 
}

inline DataTableRecvVarProxyFn RecvProp::GetDataTableProxyFn() const
{
	return m_DataTableProxyFn; 
}

inline void RecvProp::SetDataTableProxyFn( DataTableRecvVarProxyFn fn )
{
	m_DataTableProxyFn = fn; 
}

inline int RecvProp::GetOffset() const	
{
	return m_Offset; 
}

inline void RecvProp::SetOffset( int o )
{
	m_Offset = o; 
}

inline RecvProp* RecvProp::GetArrayProp() const
{
	return m_pArrayProp;
}

inline void RecvProp::SetArrayProp( RecvProp *pProp )
{
	m_pArrayProp = pProp;
}

inline void RecvProp::SetArrayLengthProxy( ArrayLengthRecvProxyFn proxy )
{
	m_ArrayLengthProxy = proxy;
}

inline ArrayLengthRecvProxyFn RecvProp::GetArrayLengthProxy() const
{
	return m_ArrayLengthProxy;
}

inline bool RecvProp::IsInsideArray() const
{
	return m_bInsideArray;
}

inline void RecvProp::SetInsideArray()
{
	m_bInsideArray = true;
}

inline const void* RecvProp::GetExtraData() const
{
	return m_pExtraData;
}

inline const char* RecvProp::GetParentArrayPropName()
{
	return m_pParentArrayPropName;
}

inline void	RecvProp::SetParentArrayPropName( const char *pArrayPropName )
{
	m_pParentArrayPropName = pArrayPropName;
}

#endif // DATATABLE_RECV_H
