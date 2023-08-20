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

//ClientClass *g_pClientClassHead=0;

//-----------------------------------------------------------------------------
// Entity creation factory
//-----------------------------------------------------------------------------
class CClientEntityFactoryDictionary : public IClientEntityFactoryDictionary
{
public:
	CClientEntityFactoryDictionary();
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
	static CClientEntityFactoryDictionary s_EntityFactory;
	return &s_EntityFactory;
}

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CClientEntityFactoryDictionary::CClientEntityFactoryDictionary() : m_Factories(true, 0, 128)
{
}


//-----------------------------------------------------------------------------
// Finds a new factory
//-----------------------------------------------------------------------------
IClientEntityFactory* CClientEntityFactoryDictionary::FindFactory(const char* pMapClassName)
{
	unsigned short nIndex = m_Factories.Find(pMapClassName);
	if (nIndex == m_Factories.InvalidIndex())
		return NULL;
	return m_Factories[nIndex];
}


//-----------------------------------------------------------------------------
// Install a new factory
//-----------------------------------------------------------------------------
void CClientEntityFactoryDictionary::InstallFactory(IClientEntityFactory* pFactory, const char* pMapClassName)
{
	Assert(FindFactory(pMapClassName) == NULL);
	m_Factories.Insert(pMapClassName, pFactory);
}


//-----------------------------------------------------------------------------
// Instantiate something using a factory
//-----------------------------------------------------------------------------
IClientNetworkable* CClientEntityFactoryDictionary::Create(const char* pMapClassName, int entnum, int serialNum)
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
const char* CClientEntityFactoryDictionary::GetCannonicalName(const char* pMapClassName)
{
	return m_Factories.GetElementName(m_Factories.Find(pMapClassName));
}

//-----------------------------------------------------------------------------
// Destroy a networkable
//-----------------------------------------------------------------------------
void CClientEntityFactoryDictionary::Destroy(const char* pMapClassName, IClientNetworkable* pNetworkable)
{
	IClientEntityFactory* pFactory = FindFactory(pMapClassName);
	if (!pFactory)
	{
		Warning("Attempted to destroy unknown entity type %s!\n", pMapClassName);
		return;
	}

	pFactory->Destroy(pNetworkable);
}

void CClientEntityFactoryDictionary::RegisteMapClassName(const char* pDllClassName, const char* pMapClassName) {
	if (m_StringMap.Defined(pDllClassName)) {
		Error("duplicate pDllClassName: %s\n", pDllClassName);	// dedicated servers exit
	}
	m_StringMap[pDllClassName] = pMapClassName;
}

const char* CClientEntityFactoryDictionary::GetMapClassName(const char* pDllClassName) {
	if (m_StringMap.Defined(pDllClassName)) {
		return m_StringMap[pDllClassName];
	}
	return "";
}

size_t CClientEntityFactoryDictionary::GetEntitySize(const char* pMapClassName) {
	IClientEntityFactory* pFactory = FindFactory(pMapClassName);
	if (!pFactory)
	{
		Warning("Attempted to destroy unknown entity type %s!\n", pMapClassName);
		return (size_t)0;
	}
	return pFactory->GetEntitySize();
}

void CClientEntityFactoryDictionary::ReportEntityNames()
{
	for (int i = m_Factories.First(); i != m_Factories.InvalidIndex(); i = m_Factories.Next(i))
	{
		Warning("%s\n", m_Factories.GetElementName(i));
	}
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void CClientEntityFactoryDictionary::ReportEntitySizes()
{
	for (int i = m_Factories.First(); i != m_Factories.InvalidIndex(); i = m_Factories.Next(i))
	{
		Msg(" %s: %d", m_Factories.GetElementName(i), m_Factories[i]->GetEntitySize());
	}
}

int		ClientClassManager::GetClientClassesCount() {
	return GetClientClassMap().GetNumStrings();
}

ClientClass* ClientClassManager::FindClientClass(const char* pName) {
	if (GetClientClassMap().Defined(pName)) {
		return GetClientClassMap()[pName];
	}
	return NULL;
}

ClientClass* ClientClassManager::GetClientClassHead() {
	return m_pClientClassHead;
}

void	ClientClassManager::RegisteClientClass(ClientClass* pClientClass) {

	if (GetClientClassMap().Defined(pClientClass->GetName())) {
		Error("duplicate ClientClass: %s\n", pClientClass->GetName());	// dedicated servers exit
	}
	else {
		GetClientClassMap()[pClientClass->GetName()] = pClientClass;
	}
	// g_pClientClassHead is sorted alphabetically, so find the correct place to insert
	if (!m_pClientClassHead)
	{
		m_pClientClassHead = pClientClass;
		pClientClass->m_pNext = NULL;
	}
	else
	{
		ClientClass* p1 = m_pClientClassHead;
		ClientClass* p2 = p1->m_pNext;

		// use _stricmp because Q_stricmp isn't hooked up properly yet
		if (_stricmp(p1->GetName(), pClientClass->GetName()) > 0)
		{
			pClientClass->m_pNext = m_pClientClassHead;
			m_pClientClassHead = pClientClass;
			p1 = NULL;
		}

		while (p1)
		{
			if (p2 == NULL || _stricmp(p2->GetName(), pClientClass->GetName()) > 0)
			{
				pClientClass->m_pNext = p2;
				p1->m_pNext = pClientClass;
				break;
			}
			p1 = p2;
			p2 = p2->m_pNext;
		}
	}
}

static ClientClassManager s_ClientClassManager;
ClientClassManager* g_pClientClassManager = &s_ClientClassManager;

