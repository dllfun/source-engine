//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "server_class.h"
#include "utldict.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ServerClass *g_pServerClassHead=0;

//-----------------------------------------------------------------------------
// Entity creation factory
//-----------------------------------------------------------------------------
class CEntityFactoryDictionary : public IEntityFactoryDictionary
{
public:
	CEntityFactoryDictionary();

	virtual void InstallFactory(IEntityFactory* pFactory, const char* pClassName);
	virtual IServerNetworkable* Create(const char* pClassName, edict_t* edict);
	virtual void Destroy(const char* pClassName, IServerNetworkable* pNetworkable);
	virtual const char* GetCannonicalName(const char* pClassName);
	virtual void ReportEntityNames();
	void ReportEntitySizes();

private:
	IEntityFactory* FindFactory(const char* pClassName);
public:
	CUtlDict< IEntityFactory*, unsigned short > m_Factories;
};

//-----------------------------------------------------------------------------
// Singleton accessor
//-----------------------------------------------------------------------------
IEntityFactoryDictionary* EntityFactoryDictionary()
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
IEntityFactory* CEntityFactoryDictionary::FindFactory(const char* pClassName)
{
	unsigned short nIndex = m_Factories.Find(pClassName);
	if (nIndex == m_Factories.InvalidIndex())
		return NULL;
	return m_Factories[nIndex];
}


//-----------------------------------------------------------------------------
// Install a new factory
//-----------------------------------------------------------------------------
void CEntityFactoryDictionary::InstallFactory(IEntityFactory* pFactory, const char* pClassName)
{
	Assert(FindFactory(pClassName) == NULL);
	m_Factories.Insert(pClassName, pFactory);
}


//-----------------------------------------------------------------------------
// Instantiate something using a factory
//-----------------------------------------------------------------------------
IServerNetworkable* CEntityFactoryDictionary::Create(const char* pClassName, edict_t* edict)
{
	IEntityFactory* pFactory = FindFactory(pClassName);
	if (!pFactory)
	{
		Warning("Attempted to create unknown entity type %s!\n", pClassName);
		return NULL;
	}
#if defined(TRACK_ENTITY_MEMORY) && defined(USE_MEM_DEBUG)
	MEM_ALLOC_CREDIT_(m_Factories.GetElementName(m_Factories.Find(pClassName)));
#endif
	return pFactory->Create(pClassName, edict);
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
	IEntityFactory* pFactory = FindFactory(pClassName);
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



