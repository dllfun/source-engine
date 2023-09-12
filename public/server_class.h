//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef SERVER_CLASS_H
#define SERVER_CLASS_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/dbg.h"
#include "dt_send.h"
#include "networkstringtabledefs.h"
#include "iservernetworkable.h"

// entity creation
// creates an entity that has not been linked to a classname
template< class T >
T* _CreateEntityTemplate(T* newEnt, const char* className, edict_t* edict)
{
	newEnt = new T; // this is the only place 'new' should be used!
	newEnt->PostConstructor(className, edict );
	return newEnt;
}

#define CREATE_UNSAVED_ENTITY( newClass, className ) _CreateEntityTemplate( (newClass*)NULL, className ,NULL)




class ServerClass;
class SendTable;

class ServerClassManager {
public:

	int		GetServerClassesCount();
	ServerClass* FindServerClass(const char* pName);
	ServerClass* GetServerClassHead();
	void	RegisteServerClass(ServerClass* pServerClass);
	void	RegisteServerClassAlias(ServerClass* pServerClass, const char* pMapClassName);
private:
	ServerClass* m_pServerClassHead = NULL;
	CUtlStringMap< ServerClass* >& GetMainServerClassMap() {
		static CUtlStringMap< ServerClass* >	s_MainServerClassMap;
		return s_MainServerClassMap;
	}
	CUtlStringMap< ServerClass* >& GetAliasServerClassMap() {
		static CUtlStringMap< ServerClass* >	s_AliasServerClassMap;
		return s_AliasServerClassMap;
	}
};

//extern ServerClass *g_pServerClassHead;
extern ServerClassManager* g_pServerClassManager;

class ServerClass {

public:
	virtual void InitRefSendTable(SendTableManager* pSendTableNanager) = 0;
	virtual const char* GetNetworkName() = 0;
	virtual int& GetClassID() = 0;
	virtual int& GetInstanceBaselineIndex() = 0;
	virtual SendTable*& GetDataTable() = 0;
	virtual ServerClass*& GetNext() = 0;
	//virtual int RequiredEdictIndex() = 0;
	//virtual IServerNetworkable* Create(edict_t* edict) = 0;//const char* pClassName, 
	//virtual void Destroy(IServerNetworkable* pNetworkable) = 0;
	//virtual size_t GetEntitySize() = 0;
};

template <class T>
class PrototypeServerClass : public ServerClass
{
public:
				PrototypeServerClass( const char *pNetworkName, const char* pDataTableName )
				{
					if (!pDataTableName || !pDataTableName[0]) {
						Error("pTableName can not been NULL: %s\n", pNetworkName);
					}
					m_pNetworkName = pNetworkName;
					m_pDataTableName = pDataTableName;
					m_InstanceBaselineIndex = INVALID_STRING_INDEX;
					g_pServerClassManager->RegisteServerClass(this);
				}

				void InitRefSendTable(SendTableManager* pSendTableNanager) {
					if (m_pDataTableName&& m_pDataTableName[0]) {
						m_pDataTable = pSendTableNanager->FindSendTable(m_pDataTableName);
						if (!m_pDataTable) {
							Error("not found SendTable: %s\n", m_pDataTableName);	// dedicated servers exit
						}
					}
				}

	const char*	GetNetworkName()		{ return m_pNetworkName; }

	virtual int& GetClassID() { return m_ClassID; }

	virtual int& GetInstanceBaselineIndex() { return m_InstanceBaselineIndex; }

	virtual SendTable*& GetDataTable() { return m_pDataTable; };

	virtual ServerClass*& GetNext() { return m_pNext; };

	//IServerNetworkable* Create(edict_t* edict)
	//{
	//	T* pEnt = _CreateEntityTemplate((T*)NULL, m_pMapClassName, edict);
	//	return pEnt->NetworkProp();
	//}

	//void Destroy(IServerNetworkable* pNetworkable)
	//{
	//	if (pNetworkable)
	//	{
	//		pNetworkable->Release();
	//	}
	//}

	//virtual size_t GetEntitySize()
	//{
	//	return sizeof(T);
	//}

	//virtual int RequiredEdictIndex() {
	//	return m_RequiredEdictIndex;
	//};

private:
	const char					*m_pNetworkName = NULL;
	const char*					m_pDataTableName = NULL;
	const char*					m_pMapClassName = NULL;
	SendTable					*m_pDataTable = NULL;
	ServerClass					*m_pNext = NULL;
	int							m_ClassID = 0;	// Managed by the engine.

	// This is an index into the network string table (sv.GetInstanceBaselineTable()).
	int							m_InstanceBaselineIndex; // INVALID_STRING_INDEX if not initialized yet.
	int							m_RequiredEdictIndex = -1;
};




class CBaseNetworkable;

// If you do a DECLARE_SERVERCLASS, you need to do this inside the class definition.
#define DECLARE_SERVERCLASS()									\
	public:														\
		static ServerClass* GetServerClassStatic();				\
		virtual ServerClass* GetServerClass();					\
		virtual int YouForgotToImplementOrDeclareServerClass();	

#define DECLARE_SERVERCLASS_NOBASE()							

// Use this macro to expose your class's data across the network.
#define IMPLEMENT_SERVERCLASS( DLLClassName, sendTable ) \
	IMPLEMENT_SERVERCLASS_INTERNAL( DLLClassName, sendTable )

// You can use this instead of BEGIN_SEND_TABLE and it will do a DECLARE_SERVERCLASS automatically.
#define IMPLEMENT_SERVERCLASS_ST(DLLClassName, sendTable, baseTableName) \
	IMPLEMENT_SERVERCLASS_INTERNAL( DLLClassName, sendTable )\
	BEGIN_SEND_TABLE(DLLClassName, sendTable, baseTableName)

#define IMPLEMENT_SERVERCLASS_ST_NOBASE(DLLClassName, sendTable) \
	IMPLEMENT_SERVERCLASS_INTERNAL( DLLClassName, sendTable )\
	BEGIN_SEND_TABLE_NOBASE( DLLClassName, sendTable )


#ifdef VALIDATE_DECLARE_CLASS
	#define CHECK_DECLARE_CLASS( DLLClassName, sendTable ) \
		template <typename T> int CheckDeclareClass_Access(T *); \
		template <> int CheckDeclareClass_Access<sendTable::ignored>(sendTable::ignored *, const char *pIgnored) \
		{ \
			return DLLClassName::CheckDeclareClass( #DLLClassName ); \
		} \
		namespace sendTable \
		{ \
			int verifyDeclareClass = CheckDeclareClass_Access( (sendTable::ignored*)0 ); \
		}
#else
	#define CHECK_DECLARE_CLASS( DLLClassName, sendTable )
#endif


//#define IMPLEMENT_SERVERCLASS_INTERNAL( DLLClassName, sendTable ) \
//	namespace sendTable \
//	{ \
//		struct ignored; \
//		extern SendTable g_SendTable; \
//	} \
//	CHECK_DECLARE_CLASS( DLLClassName, sendTable ) \
//	static ServerClass g_##DLLClassName##_ClassReg(\
//		#DLLClassName, \
//		&sendTable::g_SendTable\
//	); \
//	\
//	ServerClass* DLLClassName::GetServerClass() {return &g_##DLLClassName##_ClassReg;} \
//	SendTable *DLLClassName::m_pClassSendTable = &sendTable::g_SendTable;\
//	int DLLClassName::YouForgotToImplementOrDeclareServerClass() {return 0;}

#define IMPLEMENT_SERVERCLASS_INTERNAL( DLLClassName, sendTable ) \
	CHECK_DECLARE_CLASS( DLLClassName, sendTable ) \
	static PrototypeServerClass<DLLClassName> g_##DLLClassName##_ClassReg(\
		#DLLClassName, \
		#sendTable\
	); \
	ServerClass* DLLClassName::GetServerClassStatic() {return &g_##DLLClassName##_ClassReg;} \
	ServerClass* DLLClassName::GetServerClass() {return &g_##DLLClassName##_ClassReg;} \
	int DLLClassName::YouForgotToImplementOrDeclareServerClass() {return 0;}\
	static DLLClassName g_##DLLClassName##_EntityReg;

template <class T>
class ServerClassAliasRegister
{
public:
	ServerClassAliasRegister(const char* pMapClassName) {
		if (!pMapClassName || !pMapClassName[0]) {
			Error("pMapClassName can not been NULL\n");
		}
		ServerClass* serverClass = T::GetServerClassStatic();
		g_pServerClassManager->RegisteServerClassAlias(serverClass, pMapClassName);
	}
};



class IServerEntityFactory;
// This is the glue that hooks .MAP entity class names to our CPP classes
abstract_class IServerEntityFactoryDictionary
{
public:
	virtual void InstallFactory(IServerEntityFactory * pFactory, const char* pMapClassName) = 0;
	virtual int RequiredEdictIndex(const char* pMapClassName) = 0;
	virtual IServerNetworkable* Create(const char* pMapClassName, edict_t* edict) = 0;
	virtual void Destroy(const char* pMapClassName, IServerNetworkable* pNetworkable) = 0;
	virtual IServerEntityFactory* FindFactory(const char* pMapClassName) = 0;
	virtual const char* GetCannonicalName(const char* pMapClassName) = 0;
	virtual void ReportEntityNames() = 0;
	virtual void ReportEntitySizes() = 0;
};

IServerEntityFactoryDictionary* ServerEntityFactoryDictionary();

inline bool CanCreateServerEntity(const char* pszClassname)
{
	return (ServerEntityFactoryDictionary() != NULL && ServerEntityFactoryDictionary()->FindFactory(pszClassname) != NULL);
}

abstract_class IServerEntityFactory
{
public:
	virtual int RequiredEdictIndex() = 0;
	virtual IServerNetworkable* Create(edict_t* edict) = 0;//const char* pClassName, 
	virtual void Destroy(IServerNetworkable* pNetworkable) = 0;
	virtual size_t GetEntitySize() = 0;
};

template <class T>
class CServerEntityFactory : public IServerEntityFactory
{
public:
	CServerEntityFactory(const char* pMapClassName, int RequiredEdictIndex = -1)
	{
		m_pMapClassName = pMapClassName;
		m_RequiredEdictIndex = RequiredEdictIndex;
		ServerEntityFactoryDictionary()->InstallFactory(this, pMapClassName);
	}

	IServerNetworkable* Create(edict_t* edict)
	{
		T* pEnt = _CreateEntityTemplate((T*)NULL, m_pMapClassName, edict);
		return pEnt->NetworkProp();
	}

	void Destroy(IServerNetworkable* pNetworkable)
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

	virtual int RequiredEdictIndex() {
		return m_RequiredEdictIndex;
	};

private:
	const char* m_pMapClassName = NULL;
	int m_RequiredEdictIndex = -1;
};

#define LINK_ENTITY_TO_CLASS(mapClassName,DLLClassName) \
	static CServerEntityFactory<DLLClassName> g_##mapClassName##_ClassReg( #mapClassName );\
	static ServerClassAliasRegister<DLLClassName> g_##mapClassName##_ClassAliasReg( #mapClassName );

#define LINK_WORLD_TO_CLASS(mapClassName,DLLClassName) \
	static CServerEntityFactory<DLLClassName> mapClassName( #mapClassName, 0 );\
	static ServerClassAliasRegister<DLLClassName> g_##mapClassName##_ClassAliasReg( #mapClassName );

#endif



