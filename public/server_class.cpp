//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "server_class.h"
#include "utldict.h"
#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ServerClass *g_pServerClassHead=0;


//-----------------------------------------------------------------------------
// Entity creation factory
//-----------------------------------------------------------------------------
class CEntityFactoryDictionary : public IServerEntityFactoryDictionary
{
public:
	CEntityFactoryDictionary();

	virtual void InstallFactory(IServerEntityFactory* pFactory, const char* pClassName);
	virtual int RequiredEdictIndex(const char* pClassName);
	virtual IServerNetworkable* Create(const char* pClassName, edict_t* edict);
	virtual void Destroy(const char* pClassName, IServerNetworkable* pNetworkable);
	virtual const char* GetCannonicalName(const char* pClassName);
	virtual void ReportEntityNames();
	void ReportEntitySizes();

private:
	IServerEntityFactory* FindFactory(const char* pClassName);
public:
	CUtlDict< IServerEntityFactory*, unsigned short > m_Factories;
};

//-----------------------------------------------------------------------------
// Singleton accessor
//-----------------------------------------------------------------------------
IServerEntityFactoryDictionary* ServerEntityFactoryDictionary()
{
	static CEntityFactoryDictionary s_EntityFactory;
	return &s_EntityFactory;
}

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CEntityFactoryDictionary::CEntityFactoryDictionary() : m_Factories(true, 0, 128)
{
}


//-----------------------------------------------------------------------------
// Finds a new factory
//-----------------------------------------------------------------------------
IServerEntityFactory* CEntityFactoryDictionary::FindFactory(const char* pClassName)
{
	unsigned short nIndex = m_Factories.Find(pClassName);
	if (nIndex == m_Factories.InvalidIndex())
		return NULL;
	return m_Factories[nIndex];
}


//-----------------------------------------------------------------------------
// Install a new factory
//-----------------------------------------------------------------------------
void CEntityFactoryDictionary::InstallFactory(IServerEntityFactory* pFactory, const char* pClassName)
{
	Assert(FindFactory(pClassName) == NULL);
	m_Factories.Insert(pClassName, pFactory);
}

int CEntityFactoryDictionary::RequiredEdictIndex(const char* pClassName) {
	unsigned short nIndex = m_Factories.Find(pClassName);
	if (nIndex == m_Factories.InvalidIndex())
		return -1;
	return m_Factories[nIndex]->RequiredEdictIndex();
}
//-----------------------------------------------------------------------------
// Instantiate something using a factory
//-----------------------------------------------------------------------------
IServerNetworkable* CEntityFactoryDictionary::Create(const char* pClassName, edict_t* edict)
{
	IServerEntityFactory* pFactory = FindFactory(pClassName);
	if (!pFactory)
	{
		Warning("Attempted to create unknown entity type %s!\n", pClassName);
		return NULL;
	}
#if defined(TRACK_ENTITY_MEMORY) && defined(USE_MEM_DEBUG)
	MEM_ALLOC_CREDIT_(m_Factories.GetElementName(m_Factories.Find(pClassName)));
#endif
	return pFactory->Create(edict);//pClassName, 
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
const char* CEntityFactoryDictionary::GetCannonicalName(const char* pClassName)
{
	return m_Factories.GetElementName(m_Factories.Find(pClassName));
}

//-----------------------------------------------------------------------------
// Destroy a networkable
//-----------------------------------------------------------------------------
void CEntityFactoryDictionary::Destroy(const char* pClassName, IServerNetworkable* pNetworkable)
{
	IServerEntityFactory* pFactory = FindFactory(pClassName);
	if (!pFactory)
	{
		Warning("Attempted to destroy unknown entity type %s!\n", pClassName);
		return;
	}

	pFactory->Destroy(pNetworkable);
}

void CEntityFactoryDictionary::ReportEntityNames() 
{
	for (int i = m_Factories.First(); i != m_Factories.InvalidIndex(); i = m_Factories.Next(i))
	{
		Warning("%s\n", m_Factories.GetElementName(i));
	}
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void CEntityFactoryDictionary::ReportEntitySizes()
{
	for (int i = m_Factories.First(); i != m_Factories.InvalidIndex(); i = m_Factories.Next(i))
	{
		Msg(" %s: %d", m_Factories.GetElementName(i), m_Factories[i]->GetEntitySize());
	}
}



//CBaseNetworkable *CreateNetworkableByName( const char *className )
//{
//	IServerNetworkable *pNetwork = EntityFactoryDictionary()->Create( className );
//	if ( !pNetwork )
//		return NULL;
//
//	CBaseNetworkable *pNetworkable = pNetwork->GetBaseNetworkable();
//	Assert( pNetworkable );
//	return pNetworkable;
//}

// handling entity/edict transforms
CBaseEntity* GetContainingEntity(edict_t* pent)
{
	if (pent && pent->GetUnknown())
	{
		return pent->GetUnknown()->GetBaseEntity();
	}

	return NULL;
}

void FreeContainingEntity(edict_t* ed)
{
	if (ed)
	{
		CBaseEntity* ent = GetContainingEntity(ed);
		if (ent)
		{
			ed->SetEdict(NULL, false);
			CBaseEntity::PhysicsRemoveTouchedList(ent);
			CBaseEntity::PhysicsRemoveGroundList(ent);
			UTIL_RemoveImmediate(ent);
		}
	}
}


