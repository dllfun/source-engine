//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "server_class.h"
#include "utldict.h"
//#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//ServerClass *g_pServerClassHead=0;


//-----------------------------------------------------------------------------
// Entity creation factory
//-----------------------------------------------------------------------------
class CServerEntityFactoryDictionary : public IServerEntityFactoryDictionary
{
public:
	CServerEntityFactoryDictionary();

	virtual void InstallFactory(IServerEntityFactory* pFactory, const char* pMapClassName);
	virtual int RequiredEdictIndex(const char* pMapClassName);
	virtual IServerNetworkable* Create(const char* pMapClassName, edict_t* edict);
	virtual void Destroy(const char* pMapClassName, IServerNetworkable* pNetworkable);
	virtual const char* GetCannonicalName(const char* pMapClassName);
	virtual void ReportEntityNames();
	void ReportEntitySizes();

private:
	IServerEntityFactory* FindFactory(const char* pMapClassName);
public:
	CUtlDict< IServerEntityFactory*, unsigned short > m_Factories;
};

//-----------------------------------------------------------------------------
// Singleton accessor
//-----------------------------------------------------------------------------
IServerEntityFactoryDictionary* ServerEntityFactoryDictionary()
{
	static CServerEntityFactoryDictionary s_EntityFactory;
	return &s_EntityFactory;
}

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CServerEntityFactoryDictionary::CServerEntityFactoryDictionary() : m_Factories(true, 0, 128)
{
}


//-----------------------------------------------------------------------------
// Finds a new factory
//-----------------------------------------------------------------------------
IServerEntityFactory* CServerEntityFactoryDictionary::FindFactory(const char* pMapClassName)
{
	unsigned short nIndex = m_Factories.Find(pMapClassName);
	if (nIndex == m_Factories.InvalidIndex())
		return NULL;
	return m_Factories[nIndex];
}


//-----------------------------------------------------------------------------
// Install a new factory
//-----------------------------------------------------------------------------
void CServerEntityFactoryDictionary::InstallFactory(IServerEntityFactory* pFactory, const char* pMapClassName)
{
	Assert(FindFactory(pMapClassName) == NULL);
	m_Factories.Insert(pMapClassName, pFactory);
}

int CServerEntityFactoryDictionary::RequiredEdictIndex(const char* pMapClassName) {
	unsigned short nIndex = m_Factories.Find(pMapClassName);
	if (nIndex == m_Factories.InvalidIndex())
		return -1;
	return m_Factories[nIndex]->RequiredEdictIndex();
}
//-----------------------------------------------------------------------------
// Instantiate something using a factory
//-----------------------------------------------------------------------------
IServerNetworkable* CServerEntityFactoryDictionary::Create(const char* pMapClassName, edict_t* edict)
{
	IServerEntityFactory* pFactory = FindFactory(pMapClassName);
	if (!pFactory)
	{
		Warning("Attempted to create unknown entity type %s!\n", pMapClassName);
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
const char* CServerEntityFactoryDictionary::GetCannonicalName(const char* pMapClassName)
{
	return m_Factories.GetElementName(m_Factories.Find(pMapClassName));
}

//-----------------------------------------------------------------------------
// Destroy a networkable
//-----------------------------------------------------------------------------
void CServerEntityFactoryDictionary::Destroy(const char* pMapClassName, IServerNetworkable* pNetworkable)
{
	IServerEntityFactory* pFactory = FindFactory(pMapClassName);
	if (!pFactory)
	{
		Warning("Attempted to destroy unknown entity type %s!\n", pMapClassName);
		return;
	}

	pFactory->Destroy(pNetworkable);
}

void CServerEntityFactoryDictionary::ReportEntityNames() 
{
	for (int i = m_Factories.First(); i != m_Factories.InvalidIndex(); i = m_Factories.Next(i))
	{
		Warning("%s\n", m_Factories.GetElementName(i));
	}
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void CServerEntityFactoryDictionary::ReportEntitySizes()
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



int		ServerClassManager::GetServerClassesCount() {
	return GetServerClassMap().GetNumStrings();
}

ServerClass* ServerClassManager::FindServerClass(const char* pName) {
	if (GetServerClassMap().Defined(pName)) {
		return GetServerClassMap()[pName];
	}
	return NULL;
}

ServerClass* ServerClassManager::GetServerClassHead() {
	return m_pServerClassHead;
}

void	ServerClassManager::RegisteServerClass(ServerClass* pServerClass) {

	if (GetServerClassMap().Defined(pServerClass->GetNetworkName())) {
		Error("duplicate ServerClass: %s\n", pServerClass->GetNetworkName());	// dedicated servers exit
	}
	else {
		GetServerClassMap()[pServerClass->GetNetworkName()] = pServerClass;
	}
	// g_pServerClassHead is sorted alphabetically, so find the correct place to insert
	if (!m_pServerClassHead)
	{
		m_pServerClassHead = pServerClass;
		pServerClass->GetNext() = NULL;
	}
	else
	{
		ServerClass* p1 = m_pServerClassHead;
		ServerClass* p2 = p1->GetNext();

		// use _stricmp because Q_stricmp isn't hooked up properly yet
		if (_stricmp(p1->GetNetworkName(), pServerClass->GetNetworkName()) > 0)
		{
			pServerClass->GetNext() = m_pServerClassHead;
			m_pServerClassHead = pServerClass;
			p1 = NULL;
		}

		while (p1)
		{
			if (p2 == NULL || _stricmp(p2->GetNetworkName(), pServerClass->GetNetworkName()) > 0)
			{
				pServerClass->GetNext() = p2;
				p1->GetNext() = pServerClass;
				break;
			}
			p1 = p2;
			p2 = p2->GetNext();
		}
	}
}

static ServerClassManager s_ServerClassManager;
ServerClassManager* g_pServerClassManager = &s_ServerClassManager;
