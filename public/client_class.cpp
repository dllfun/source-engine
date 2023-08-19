//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "client_class.h"
#include "utldict.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ClientClass *g_pClientClassHead=0;

//-----------------------------------------------------------------------------
// Entity creation factory
//-----------------------------------------------------------------------------
class CEntityFactoryDictionary : public IClientEntityFactoryDictionary
{
public:
	CEntityFactoryDictionary();
	virtual void InstallFactory(IClientEntityFactory* pFactory, const char* pMapClassName);
	virtual IClientNetworkable* Create(const char* pMapClassName, int entnum, int serialNum);
	virtual void Destroy(const char* pMapClassName, IClientNetworkable* pNetworkable);
	virtual const char* GetCannonicalName(const char* pMapClassName);
	virtual void RegisteMapClassName(const char* pDllClassName, const char* pMapClassName);
	virtual const char* GetMapClassName(const char* pDllClassName);
	virtual size_t GetEntitySize(const char* pMapClassName);
	virtual void ReportEntityNames();
	void ReportEntitySizes();

private:
	IClientEntityFactory* FindFactory(const char* pMapClassName);
public:
	CUtlDict< IClientEntityFactory*, unsigned short > m_Factories;
	CUtlStringMap<const char*>	m_StringMap;
};

//-----------------------------------------------------------------------------
// Singleton accessor
//-----------------------------------------------------------------------------
IClientEntityFactoryDictionary* ClientEntityFactoryDictionary()
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
IClientEntityFactory* CEntityFactoryDictionary::FindFactory(const char* pMapClassName)
{
	unsigned short nIndex = m_Factories.Find(pMapClassName);
	if (nIndex == m_Factories.InvalidIndex())
		return NULL;
	return m_Factories[nIndex];
}


//-----------------------------------------------------------------------------
// Install a new factory
//-----------------------------------------------------------------------------
void CEntityFactoryDictionary::InstallFactory(IClientEntityFactory* pFactory, const char* pMapClassName)
{
	Assert(FindFactory(pMapClassName) == NULL);
	m_Factories.Insert(pMapClassName, pFactory);
}


//-----------------------------------------------------------------------------
// Instantiate something using a factory
//-----------------------------------------------------------------------------
IClientNetworkable* CEntityFactoryDictionary::Create(const char* pMapClassName, int entnum, int serialNum)
{
	IClientEntityFactory* pFactory = FindFactory(pMapClassName);
	if (!pFactory)
	{
		Warning("Attempted to create unknown entity type %s!\n", pMapClassName);
		return NULL;
	}
#if defined(TRACK_ENTITY_MEMORY) && defined(USE_MEM_DEBUG)
	MEM_ALLOC_CREDIT_(m_Factories.GetElementName(m_Factories.Find(pClassName)));
#endif
	return pFactory->Create(entnum, serialNum);//pClassName, 
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
const char* CEntityFactoryDictionary::GetCannonicalName(const char* pMapClassName)
{
	return m_Factories.GetElementName(m_Factories.Find(pMapClassName));
}

//-----------------------------------------------------------------------------
// Destroy a networkable
//-----------------------------------------------------------------------------
void CEntityFactoryDictionary::Destroy(const char* pMapClassName, IClientNetworkable* pNetworkable)
{
	IClientEntityFactory* pFactory = FindFactory(pMapClassName);
	if (!pFactory)
	{
		Warning("Attempted to destroy unknown entity type %s!\n", pMapClassName);
		return;
	}

	pFactory->Destroy(pNetworkable);
}

void CEntityFactoryDictionary::RegisteMapClassName(const char* pDllClassName, const char* pMapClassName) {
	if (m_StringMap.Defined(pDllClassName)) {
		Error("duplicate pDllClassName: %s\n", pDllClassName);	// dedicated servers exit
	}
	m_StringMap[pDllClassName] = pMapClassName;
}

const char* CEntityFactoryDictionary::GetMapClassName(const char* pDllClassName) {
	if (m_StringMap.Defined(pDllClassName)) {
		return m_StringMap[pDllClassName];
	}
	return "";
}

size_t CEntityFactoryDictionary::GetEntitySize(const char* pMapClassName) {
	IClientEntityFactory* pFactory = FindFactory(pMapClassName);
	if (!pFactory)
	{
		Warning("Attempted to destroy unknown entity type %s!\n", pMapClassName);
		return (size_t)0;
	}
	return pFactory->GetEntitySize();
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



