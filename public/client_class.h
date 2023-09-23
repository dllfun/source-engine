//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//
#if !defined( CLIENT_CLASS_H )
#define CLIENT_CLASS_H
#ifdef _WIN32
#pragma once
#endif

#include "interface.h"
#include "dt_recv.h"

//-----------------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------------

class Vector;
class CMouthInfo;


//-----------------------------------------------------------------------------
// represents a handle used only by the client DLL
//-----------------------------------------------------------------------------

#include "iclientrenderable.h"
#include "iclientnetworkable.h"
#include "icliententity.h"

// entity creation
// creates an entity that has not been linked to a classname
template< class T >
T* _CreateEntityTemplate(T* newEnt, int entnum, int serialNum)
{
	newEnt = new T; // this is the only place 'new' should be used!
	if (entnum >= 0) {
		newEnt->Init(entnum, serialNum);
	}
	return newEnt;
}



class ClientClass;

class ClientClassManager {
public:

	int		GetClientClassesCount();
	ClientClass* FindClientClass(const char* pName);
	ClientClass* GetClientClassHead();
	void	RegisteClientClass(ClientClass* pClientClass);
	void	RegisteClientClassAlias(ClientClass* pClientClass, const char* pMapClassName);
private:
	ClientClass* m_pClientClassHead = NULL;
	CUtlStringMap< ClientClass* >& GetClientClassMap() {
		static CUtlStringMap< ClientClass* >	s_ClientClassMap;
		return s_ClientClassMap;
	}
	CUtlStringMap< ClientClass* >& GetAliasClientClassMap() {
		static CUtlStringMap< ClientClass* >	s_AliasClientClassMap;
		return s_AliasClientClassMap;
	}
};

extern ClientClassManager* g_pClientClassManager;

// Linked list of all known client classes
//extern ClientClass *g_pClientClassHead;

// The serial number that gets passed in is used for ehandles.
//typedef IClientNetworkable*	(*CreateClientClassFn)( int entnum, int serialNum );
//typedef IClientNetworkable*	(*CreateEventFn)();

//-----------------------------------------------------------------------------
// Purpose: Client side class definition
//-----------------------------------------------------------------------------

class ClientClass {
public:
	virtual void InitRefRecvTable(RecvTableManager* pRecvTableNanager) = 0;
	virtual const char* GetNetworkName() const = 0;
	virtual const char* GetClassName() const = 0;
	virtual RecvTable* GetDataTable() const = 0;
	virtual RecvTable*& GetDataTable() = 0;
	virtual ClientClass*& GetNext() = 0;
	virtual int&		GetClassID() = 0;
};

class SimpleClientClass : public ClientClass {
public:
	SimpleClientClass(const char* pNetworkName, const char* pDataTableName, RecvTable* pRecvTable = NULL)
	{
		if (!pDataTableName || !pDataTableName[0]) {
			Error("pTableName can not been NULL: %s\n", pNetworkName);
		}
		m_pNetworkName = pNetworkName;
		m_pDataTableName = pDataTableName;
		m_pDataTable = pRecvTable;
		// Link it in
		//m_pNext				= g_pClientClassHead;
		//g_pClientClassHead	= this;
		g_pClientClassManager->RegisteClientClass(this);
	}

	void InitRefRecvTable(RecvTableManager* pRecvTableNanager) {
		m_pDataTable = pRecvTableNanager->FindRecvTable(m_pDataTableName);
		if (!m_pDataTable) {
			Error("not found RecvTable: %s\n", m_pDataTableName);	// dedicated servers exit
		}
	}

	const char* GetNetworkName() const
	{
		return m_pNetworkName;
	}

	const char* GetClassName() const
	{
		Error("never call this");
	}

	virtual RecvTable* GetDataTable() const {
		return m_pDataTable;
	}

	virtual RecvTable*& GetDataTable() {
		return m_pDataTable;
	}

	virtual ClientClass*& GetNext() {
		return m_pNext;
	}

	virtual int& GetClassID() {
		return m_ClassID;
	}

private:
	const char* m_pNetworkName;
	const char* m_pDataTableName;
	RecvTable* m_pDataTable = NULL;
	ClientClass* m_pNext = NULL;
	int						m_ClassID = 0;	// Managed by the engine.
};

template <class T>
class PrototypeClientClass : public ClientClass
{
public:
	PrototypeClientClass( const char* pDllClassName, const char *pNetworkName, const char *pDataTableName, RecvTable* pRecvTable=NULL)
	{
		if (!pDataTableName || !pDataTableName[0]) {
			Error("pTableName can not been NULL: %s\n", pNetworkName);
		}
		if (!strcmp(pDllClassName, pNetworkName)) {
			Error("ClientClassName:%s can not been same as ServerClassName: %s\n", pDllClassName, pNetworkName);
		}
		T::InitRecvTable();
		m_pDllClassName = pDllClassName;
		m_pNetworkName	= pNetworkName;
		m_pDataTableName= pDataTableName;
		m_pDataTable	= pRecvTable;
		// Link it in
		//m_pNext				= g_pClientClassHead;
		//g_pClientClassHead	= this;
		g_pClientClassManager->RegisteClientClass(this);
	}

	void InitRefRecvTable(RecvTableManager* pRecvTableNanager) {
		m_pDataTable = pRecvTableNanager->FindRecvTable(m_pDataTableName);
		if (!m_pDataTable) {
			Error("not found RecvTable: %s\n", m_pDataTableName);	// dedicated servers exit
		}
	}

	const char* GetNetworkName() const
	{
		return m_pNetworkName;
	}

	const char* GetClassName() const
	{
		return m_pDllClassName;
	}

	virtual RecvTable* GetDataTable() const{
		return m_pDataTable;
	}

	virtual RecvTable*& GetDataTable() {
		return m_pDataTable;
	}

	virtual ClientClass*& GetNext() {
		return m_pNext;
	}

	virtual int&		GetClassID() {
		return m_ClassID;
	}

private:
	const char*				m_pDllClassName;
	const char				*m_pNetworkName;
	const char*				m_pDataTableName;
	RecvTable				*m_pDataTable = NULL;
	ClientClass				*m_pNext = NULL;
	int						m_ClassID = 0;	// Managed by the engine.
};

class SingletonClientClass : public ClientClass
{
public:
	SingletonClientClass(const char* pDllClassName, const char* pNetworkName, const char* pDataTableName, RecvTable* pRecvTable = NULL)
	{
		m_pDllClassName = pDllClassName;
		m_pNetworkName = pNetworkName;
		m_pDataTableName = pDataTableName;
		m_pDataTable = pRecvTable;
		// Link it in
		//m_pNext				= g_pClientClassHead;
		//g_pClientClassHead	= this;
		g_pClientClassManager->RegisteClientClass(this);
	}

	void InitRefRecvTable(RecvTableManager* pRecvTableNanager) {
		m_pDataTable = pRecvTableNanager->FindRecvTable(m_pDataTableName);
		if (!m_pDataTable) {
			Error("not found RecvTable: %s\n", m_pDataTableName);	// dedicated servers exit
		}
	}

	const char* GetNetworkName() const
	{
		return m_pNetworkName;
	}

	const char* GetClassName() const
	{
		return m_pDllClassName;
	}

	virtual RecvTable* GetDataTable() const {
		return m_pDataTable;
	}

	virtual RecvTable*& GetDataTable() {
		return m_pDataTable;
	}

	virtual ClientClass*& GetNext() {
		return m_pNext;
	}

	virtual int& GetClassID() {
		return m_ClassID;
	}

private:
	const char* m_pDllClassName;
	const char* m_pNetworkName;
	const char* m_pDataTableName;
	RecvTable* m_pDataTable = NULL;
	ClientClass* m_pNext = NULL;
	int						m_ClassID = 0;	// Managed by the engine.
};

#define DECLARE_CLIENTCLASS() \
	virtual int YouForgotToImplementOrDeclareClientClass();\
	static ClientClass* GetClientClassStatic();\
	virtual ClientClass* GetClientClass();\
	DECLARE_CLIENTCLASS_NOBASE()


// This can be used to give all datatables access to protected and private members of the class.
#define ALLOW_DATATABLES_PRIVATE_ACCESS() \
	template <typename T> friend int ClientClassInit(T *);


#define DECLARE_CLIENTCLASS_NOBASE ALLOW_DATATABLES_PRIVATE_ACCESS
	
// This macro adds a ClientClass to the linked list in g_pClientClassHead (so
// the list can be given to the engine).
// Use this macro to expose your client class to the engine.
// networkName must match the network name of a class registered on the server.
#define IMPLEMENT_CLIENTCLASS_NO_FACTORY(DLLClassName, recvTable, serverClassName) \
	static PrototypeClientClass<DLLClassName> __g_##DLLClassName##_ClassReg(\
		#DLLClassName, \
		#serverClassName, \
		#recvTable\
	);\
	ClientClass*	DLLClassName::GetClientClassStatic(){return &__g_##DLLClassName##_ClassReg;}\
	ClientClass*	DLLClassName::GetClientClass() {return &__g_##DLLClassName##_ClassReg;}\
	int				DLLClassName::YouForgotToImplementOrDeclareClientClass() {return 0;}\

#define IMPLEMENT_CLIENTCLASS(DLLClassName, recvTable, serverClassName) \
	IMPLEMENT_CLIENTCLASS_NO_FACTORY(DLLClassName, recvTable, serverClassName)\
	static CClientEntityFactory<DLLClassName> __g_##DLLClassName##Factory(#DLLClassName );\
	//static DLLClassName g_##DLLClassName##_EntityReg;											

// Implement a client class and provide a factory so you can allocate and delete it yourself
// (or make it a singleton).
//#define IMPLEMENT_CLIENTCLASS_FACTORY(DLLClassName, recvTable, serverClassName, factory) \
//	INTERNAL_IMPLEMENT_CLIENTCLASS_PROLOGUE(DLLClassName, recvTable, serverClassName) \
//	ClientClass __g_##DLLClassName##ClientClass(#DLLClassName, #serverClassName, \
//													factory, \
//													NULL,\
//													#recvTable);\
//	static DLLClassName g_##DLLClassName##_EntityReg;

// The IMPLEMENT_CLIENTCLASS_DT macros do IMPLEMENT_CLIENT_CLASS and also do BEGIN_RECV_TABLE.
#define IMPLEMENT_CLIENTCLASS_DT(DLLClassName, recvTable, serverClassName)\
	IMPLEMENT_CLIENTCLASS(DLLClassName, recvTable, serverClassName)\
	BEGIN_RECV_TABLE(DLLClassName, recvTable)

#define IMPLEMENT_CLIENTCLASS_DT_NOBASE(DLLClassName, recvTable, serverClassName)\
	IMPLEMENT_CLIENTCLASS(DLLClassName, recvTable, serverClassName)\
	BEGIN_RECV_TABLE_NOBASE(DLLClassName, recvTable)
	

// Using IMPLEMENT_CLIENTCLASS_EVENT means the engine thinks the entity is an event so the entity
// is responsible for freeing itself.
#define IMPLEMENT_CLIENTCLASS_EVENT(DLLClassName, recvTable, serverClassName)\
	static PrototypeClientClass<DLLClassName> __g_##DLLClassName##_ClassReg(\
		#DLLClassName, \
		#serverClassName, \
		#recvTable\
	);\
	ClientClass*	DLLClassName::GetClientClassStatic(){return &__g_##DLLClassName##_ClassReg;}\
	ClientClass*	DLLClassName::GetClientClass() {return &__g_##DLLClassName##_ClassReg;}\
	int				DLLClassName::YouForgotToImplementOrDeclareClientClass() {return 0;}\
	static CClientEntityFactory<DLLClassName> __g_##DLLClassName##Factory(#DLLClassName );\
	//static DLLClassName g_##DLLClassName##_EntityReg;										


#define IMPLEMENT_CLIENTCLASS_EVENT_DT(DLLClassName, recvTable, serverClassName)\
	IMPLEMENT_CLIENTCLASS_EVENT(DLLClassName, recvTable, serverClassName)\
	BEGIN_RECV_TABLE(DLLClassName, recvTable)


// Register a client event singleton but specify a pointer to give to the engine rather than
// have a global instance. This is useful if you're using Initializers and your object's constructor
// uses some other global object (so you must use Initializers so you're constructed afterwards).
//#define IMPLEMENT_CLIENTCLASS_EVENT_POINTER(DLLClassName, recvTable, serverClassName, ptr)\
//	INTERNAL_IMPLEMENT_CLIENTCLASS_PROLOGUE(DLLClassName, recvTable, serverClassName)\
//	static IClientNetworkable* _##DLLClassName##_CreateObject() {return ptr;}\
//	ClientClass __g_##DLLClassName##ClientClass(#DLLClassName, #serverClassName, \
//													NULL,\
//													_##DLLClassName##_CreateObject, \
//													#recvTable);\
//	static DLLClassName g_##DLLClassName##_EntityReg;

//#define IMPLEMENT_CLIENTCLASS_EVENT_NONSINGLETON(DLLClassName, recvTable, serverClassName)\
//	static IClientNetworkable* _##DLLClassName##_CreateObject() \
//	{ \
//		DLLClassName *p = new DLLClassName; \
//		if ( p ) \
//			p->Init( -1, 0 ); \
//		return p; \
//	} \
//	ClientClass __g_##DLLClassName##ClientClass(#DLLClassName, #serverClassName, \
//													NULL,\
//													_##DLLClassName##_CreateObject, \
//													#recvTable);\
//	static DLLClassName g_##DLLClassName##_EntityReg;


// Used internally..
//#define INTERNAL_IMPLEMENT_CLIENTCLASS_PROLOGUE(DLLClassName, recvTable, serverClassName) \
//	extern PrototypeClientClass __g_##DLLClassName##ClientClass;\

template <class T>
class ClientClassAliasRegister
{
public:
	ClientClassAliasRegister(const char* pMapClassName) {
		if (!pMapClassName || !pMapClassName[0]) {
			Error("pMapClassName can not been NULL\n");
		}
		ClientClass* clientClass = T::GetClientClassStatic();
		g_pClientClassManager->RegisteClientClassAlias(clientClass, pMapClassName);
	}
};


class IClientEntityFactory;
// This is the glue that hooks .MAP entity class names to our CPP classes
abstract_class IClientEntityFactoryDictionary
{
public:
	virtual void InstallFactory(IClientEntityFactory * pFactory, const char* pMapClassName) = 0;
	virtual IClientEntity* Create(const char* pMapClassName, int entnum, int serialNum) = 0;
	virtual void Destroy(const char* pMapClassName, IClientEntity* pNetworkable) = 0;
	virtual IClientEntityFactory* FindFactory(const char* pMapClassName) = 0;
	virtual const char* GetCannonicalName(const char* pMapClassName) = 0;
	virtual void RegisteMapClassName(const char* pDllClassName, const char* pMapClassName) = 0;
	virtual const char* GetMapClassName(const char* pDllClassName) = 0;
	virtual size_t GetEntitySize(const char* pMapClassName) = 0;
	virtual void ReportEntityNames() = 0;
	virtual void ReportEntitySizes() = 0;
};

IClientEntityFactoryDictionary* ClientEntityFactoryDictionary();

inline bool CanCreateClientEntity(const char* pszClassname)
{
	return (ClientEntityFactoryDictionary() != NULL && ClientEntityFactoryDictionary()->FindFactory(pszClassname) != NULL);
}

abstract_class IClientEntityFactory
{
public:
	virtual IClientEntity* Create(int entnum, int serialNum) = 0;//const char* pClassName, 
	virtual void Destroy(IClientEntity* pNetworkable) = 0;
	virtual size_t GetEntitySize() = 0;
};

template <class T>
class CClientEntityFactory : public IClientEntityFactory
{
public:
	CClientEntityFactory(const char* pDllClassName, const char* pMapClassName = NULL)
	{
		if (pMapClassName) {
			m_pMapClassName = pMapClassName;
			ClientEntityFactoryDictionary()->InstallFactory(this, pMapClassName);
			ClientEntityFactoryDictionary()->RegisteMapClassName(pDllClassName, pMapClassName);
		}
		else {
			ClientEntityFactoryDictionary()->InstallFactory(this, pDllClassName);
		}
	}

	IClientEntity* Create(int entnum, int serialNum)
	{
		T* pEnt = _CreateEntityTemplate((T*)NULL, entnum, serialNum);
		return pEnt;//->NetworkProp()
	}

	void Destroy(IClientEntity* pNetworkable)
	{
		if (pNetworkable)
		{
			pNetworkable->Release();
		}
	}

	virtual size_t GetEntitySize()
	{
		return sizeof(T);
	}

private:
	const char* m_pMapClassName = NULL;
};

template <class T>
class CClientEntitySingletonFactory : public IClientEntityFactory
{
public:
	CClientEntitySingletonFactory(const char* pDllClassName, const char* pMapClassName = NULL)
	{
		ClientEntityFactoryDictionary()->InstallFactory(this, pDllClassName);
		if (pMapClassName) {
			m_pMapClassName = pMapClassName;
			ClientEntityFactoryDictionary()->InstallFactory(this, pMapClassName);
			ClientEntityFactoryDictionary()->RegisteMapClassName(pDllClassName, pMapClassName);
		}
	}

	IClientEntity* Create(int entnum, int serialNum)
	{
		return pEnt;
	}

	void Destroy(IClientEntity* pNetworkable)
	{

	}

	virtual size_t GetEntitySize()
	{
		return sizeof(T);
	}

private:
	const char* m_pMapClassName = NULL;
	T pEnt;
};

// On the client .dll this creates a mapping between a classname and
//  a client side class.  Probably could be templatized at some point.

#define LINK_ENTITY_TO_CLASS( mapClassName, DLLClassName )						\
	static CClientEntityFactory<DLLClassName> mapClassName(#DLLClassName, #mapClassName );\
	static ClientClassAliasRegister<DLLClassName> g_##mapClassName##_ClassAliasReg( #mapClassName );

#endif // CLIENT_CLASS_H
