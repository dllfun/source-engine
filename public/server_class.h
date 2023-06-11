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


CBaseEntity* GetContainingEntity(edict_t* pent);

void FreeContainingEntity(edict_t* ed);


class IEntityFactory;
// This is the glue that hooks .MAP entity class names to our CPP classes
abstract_class IEntityFactoryDictionary
{
public:
	virtual void InstallFactory(IEntityFactory * pFactory, const char* pClassName) = 0;
	virtual int RequiredEdictIndex(const char* pClassName) = 0;
	virtual IServerNetworkable* Create(const char* pClassName, edict_t* edict) = 0;
	virtual void Destroy(const char* pClassName, IServerNetworkable* pNetworkable) = 0;
	virtual IEntityFactory* FindFactory(const char* pClassName) = 0;
	virtual const char* GetCannonicalName(const char* pClassName) = 0;
	virtual void ReportEntityNames() = 0;
	virtual void ReportEntitySizes() = 0;
};

IEntityFactoryDictionary* EntityFactoryDictionary();

inline bool CanCreateEntityClass(const char* pszClassname)
{
	return (EntityFactoryDictionary() != NULL && EntityFactoryDictionary()->FindFactory(pszClassname) != NULL);
}

abstract_class IEntityFactory
{
public:
	virtual int RequiredEdictIndex() = 0;
	virtual IServerNetworkable * Create(edict_t* edict) = 0;//const char* pClassName, 
	virtual void Destroy(IServerNetworkable* pNetworkable) = 0;
	virtual size_t GetEntitySize() = 0;
};

template <class T>
class CEntityFactory : public IEntityFactory
{
public:
	CEntityFactory(const char* pClassName, int RequiredEdictIndex = -1)
	{
		m_pClassName = pClassName;
		m_RequiredEdictIndex = RequiredEdictIndex;
		EntityFactoryDictionary()->InstallFactory(this, pClassName);
	}

	IServerNetworkable* Create(edict_t* edict)
	{
		T* pEnt = _CreateEntityTemplate((T*)NULL, m_pClassName, edict);
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
	const char* m_pClassName = NULL;
	int m_RequiredEdictIndex = -1;
};

#define LINK_ENTITY_TO_CLASS(mapClassName,DLLClassName) \
	static CEntityFactory<DLLClassName> mapClassName( #mapClassName );

#define LINK_WORLD_TO_CLASS(mapClassName,DLLClassName) \
	static CEntityFactory<DLLClassName> mapClassName( #mapClassName, 0 );


class ServerClass;
class SendTable;

extern ServerClass *g_pServerClassHead;


class ServerClass
{
public:
				ServerClass( const char *pNetworkName, const char* pTableName )
				{
					m_pNetworkName = pNetworkName;
					m_pTableName = pTableName;
					m_InstanceBaselineIndex = INVALID_STRING_INDEX;
					// g_pServerClassHead is sorted alphabetically, so find the correct place to insert
					if ( !g_pServerClassHead )
					{
						g_pServerClassHead = this;
						m_pNext = NULL;
					}
					else
					{
						ServerClass *p1 = g_pServerClassHead;
						ServerClass *p2 = p1->m_pNext;

						// use _stricmp because Q_stricmp isn't hooked up properly yet
						if ( _stricmp( p1->GetName(), pNetworkName ) > 0)
						{
							m_pNext = g_pServerClassHead;
							g_pServerClassHead = this;
							p1 = NULL;
						}

						while( p1 )
						{
							if ( p2 == NULL || _stricmp( p2->GetName(), pNetworkName ) > 0)
							{
								m_pNext = p2;
								p1->m_pNext = this;
								break;
							}
							p1 = p2;
							p2 = p2->m_pNext;
						}	
					}
				}

				void InitRefSendTable(SendTableManager* pSendTableNanager) {
					m_pTable = pSendTableNanager->FindSendTable(m_pTableName);
					if (!m_pTable) {
						Error("not found SendTable: %s\n", m_pTableName);	// dedicated servers exit
					}
				}

	const char*	GetName()		{ return m_pNetworkName; }


public:
	const char					*m_pNetworkName;
	const char*					m_pTableName;
	SendTable					*m_pTable;
	ServerClass					*m_pNext;
	int							m_ClassID;	// Managed by the engine.

	// This is an index into the network string table (sv.GetInstanceBaselineTable()).
	int							m_InstanceBaselineIndex; // INVALID_STRING_INDEX if not initialized yet.
};


class CBaseNetworkable;

// If you do a DECLARE_SERVERCLASS, you need to do this inside the class definition.
#define DECLARE_SERVERCLASS()									\
	public:														\
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
	static ServerClass g_##DLLClassName##_ClassReg(\
		#DLLClassName, \
		#sendTable\
	); \
	\
	ServerClass* DLLClassName::GetServerClass() {return &g_##DLLClassName##_ClassReg;} \
	int DLLClassName::YouForgotToImplementOrDeclareServerClass() {return 0;}\
	static DLLClassName g_##DLLClassName##_EntityReg;

#endif



