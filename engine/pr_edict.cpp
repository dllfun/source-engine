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
#include "quakedef.h"
#include <stddef.h>
#include "vengineserver_impl.h"
#include "server.h"
#include "pr_edict.h"
#include "world.h"
#include "ispatialpartition.h"
#include "utllinkedlist.h"
#include "framesnapshot.h"
#include "sv_log.h"
#include "tier1/utlmap.h"
#include "tier1/utlvector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Edicts won't get reallocated for this many seconds after being freed.
#define EDICT_FREETIME	1.0



#ifdef _DEBUG
#ifdef OSX
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
#endif // _DEBUG

static ConVar		sv_useexplicitdelete( "sv_useexplicitdelete", "1", FCVAR_DEVELOPMENTONLY, "Explicitly delete dormant client entities caused by AllowImmediateReuse()." );
static ConVar		sv_lowedict_threshold( "sv_lowedict_threshold", "8", FCVAR_NONE, "When only this many edicts are free, take the action specified by sv_lowedict_action.", true, 0, true, MAX_EDICTS );
static ConVar		sv_lowedict_action( "sv_lowedict_action", "0", FCVAR_NONE, "0 - no action, 1 - warn to log file, 2 - attempt to restart the game, if applicable, 3 - restart the map, 4 - go to the next map in the map cycle, 5 - spew all edicts.", true, 0, true, 5 );

class CBaseEdict;
// Bitmask of free edicts.
static CBitVec< MAX_EDICTS > g_FreeEdicts;
static CBaseEdict* g_pEdicts;			// Can array index now, edict_t is fixed
static int			num_edicts;
static int			max_edicts;
static int			free_edicts; // how many edicts in num_edicts are free, in use is num_edicts - free_edicts
static IChangeInfoAccessor* edictchangeinfo; // HACK to allow backward compat since we can't change edict_t layout

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
// NOTE: YOU CAN'T CHANGE THE LAYOUT OR SIZE OF CBASEEDICT AND REMAIN COMPATIBLE WITH HL2_VC6!!!!!
class CBaseEdict : public edict_t
{
public:

	CBaseEdict(int index) :m_EdictIndex(index)
	{

	}
	// Returns an IServerEntity if FL_FULLEDICT is set or NULL if this 
	// is a lightweight networking entity.
	IServerEntity* GetIServerEntity();
	const IServerEntity* GetIServerEntity() const;

	IServerNetworkable* GetNetworkable();
	IServerUnknown* GetUnknown();

	// Set when initting an entity. If it's only a networkable, this is false.
	void				SetEdict(IServerUnknown* pUnk, bool bFullEdict);

	int					AreaNum() const;
	int					GetIndex();
	short				GetNetworkSerialNumber();
	void				SetNetworkSerialNumber(short NetworkSerialNumber);
	const char* GetClassName() const;
	int& GetStateFlags();

	bool				IsFree() const;
	void				SetFree();
	void				ClearFree();

	bool				HasStateChanged() const;
	void				ClearStateChanged();
	void				StateChanged();
	void				StateChanged(unsigned short offset);

	bool				IsDontSend();
	bool				IsPendingDormantCheck();
	void				SetPendingDormantCheck();
	void				ClearPendingDormantCheck();
	void				ClearTransmitState();

	void				SetDirtyPvsInformation();

	void SetChangeInfo(unsigned short info);
	void SetChangeInfoSerialNumber(unsigned short sn);
	unsigned short	 GetChangeInfo() const;
	unsigned short	 GetChangeInfoSerialNumber() const;
	IChangeInfoAccessor* GetChangeAccessor(); // The engine implements this and the game .dll implements as
	const IChangeInfoAccessor* GetChangeAccessor() const; // The engine implements this and the game .dll implements as
	// as callback through to the engine!!!
	ICollideable* GetCollideable();
public:

	// NOTE: this is in the edict instead of being accessed by a virtual because the engine needs fast access to it.
	// NOTE: YOU CAN'T CHANGE THE LAYOUT OR SIZE OF CBASEEDICT AND REMAIN COMPATIBLE WITH HL2_VC6!!!!!
#ifdef _XBOX
	unsigned short m_fStateFlags = 0;
#else
	int	m_fStateFlags = 0;
#endif	

	// NOTE: this is in the edict instead of being accessed by a virtual because the engine needs fast access to it.
	// int m_NetworkSerialNumber;

	// NOTE: m_EdictIndex is an optimization since computing the edict index
	// from a CBaseEdict* pointer otherwise requires divide-by-20. values for
	// m_NetworkSerialNumber all fit within a 16-bit integer range, so we're
	// repurposing the other 16 bits to cache off the index without changing
	// the overall layout or size of this struct. existing mods compiled with
	// a full 32-bit serial number field should still work. henryg 8/17/2011
#if VALVE_LITTLE_ENDIAN
	short m_NetworkSerialNumber = 0;
	const short m_EdictIndex;
#else
	short m_EdictIndex;
	const short m_NetworkSerialNumber = 0;
#endif

	// The server timestampe at which the edict was freed (so we can try to use other edicts before reallocating this one)
	float		freetime = 0.0;
protected:
	// NOTE: this is in the edict instead of being accessed by a virtual because the engine needs fast access to it.
	IServerNetworkable* m_pNetworkable = NULL;
public:
	IServerUnknown* m_pUnk = NULL;


public:



	// NOTE: YOU CAN'T CHANGE THE LAYOUT OR SIZE OF CBASEEDICT AND REMAIN COMPATIBLE WITH HL2_VC6!!!!!
	// This breaks HL2_VC6!!!!!
	// References a CEdictChangeInfo with a list of modified network props.
	//unsigned short m_iChangeInfo;
	//unsigned short m_iChangeInfoSerialNumber;

	friend void InitializeEntityDLLFields(edict_t* pEdict);
};


//-----------------------------------------------------------------------------
// CBaseEdict inlines.
//-----------------------------------------------------------------------------
inline IServerEntity* CBaseEdict::GetIServerEntity()
{
	if (m_fStateFlags & FL_EDICT_FULL)
		return (IServerEntity*)m_pUnk;
	else
		return 0;
}

inline bool CBaseEdict::IsFree() const
{
	return (m_fStateFlags & FL_EDICT_FREE) != 0;
}



inline bool	CBaseEdict::HasStateChanged() const
{
	return (m_fStateFlags & FL_EDICT_CHANGED) != 0;
}

inline void	CBaseEdict::ClearStateChanged()
{
	m_fStateFlags &= ~(FL_EDICT_CHANGED | FL_FULL_EDICT_CHANGED);
	SetChangeInfoSerialNumber(0);
}

inline void	CBaseEdict::StateChanged()
{
	// Note: this should only happen for properties in data tables that used some
	// kind of pointer dereference. If the data is directly offsetable 
	m_fStateFlags |= (FL_EDICT_CHANGED | FL_FULL_EDICT_CHANGED);
	SetChangeInfoSerialNumber(0);
}

inline void	CBaseEdict::StateChanged(unsigned short offset)
{
	if (m_fStateFlags & FL_FULL_EDICT_CHANGED)
		return;

	m_fStateFlags |= FL_EDICT_CHANGED;

	IChangeInfoAccessor* accessor = GetChangeAccessor();

	if (accessor->GetChangeInfoSerialNumber() == g_pSharedChangeInfo->m_iSerialNumber)
	{
		// Ok, I still own this one.
		CEdictChangeInfo* p = &g_pSharedChangeInfo->m_ChangeInfos[accessor->GetChangeInfo()];

		// Now add this offset to our list of changed variables.		
		for (unsigned short i = 0; i < p->m_nChangeOffsets; i++)
			if (p->m_ChangeOffsets[i] == offset)
				return;

		if (p->m_nChangeOffsets == MAX_CHANGE_OFFSETS)
		{
			// Invalidate our change info.
			accessor->SetChangeInfoSerialNumber(0);
			m_fStateFlags |= FL_FULL_EDICT_CHANGED; // So we don't get in here again.
		}
		else
		{
			p->m_ChangeOffsets[p->m_nChangeOffsets++] = offset;
		}
	}
	else
	{
		if (g_pSharedChangeInfo->m_nChangeInfos == MAX_EDICT_CHANGE_INFOS)
		{
			// Shucks.. have to mark the edict as fully changed because we don't have room to remember this change.
			accessor->SetChangeInfoSerialNumber(0);
			m_fStateFlags |= FL_FULL_EDICT_CHANGED;
		}
		else
		{
			// Get a new CEdictChangeInfo and fill it out.
			accessor->SetChangeInfo(g_pSharedChangeInfo->m_nChangeInfos);
			g_pSharedChangeInfo->m_nChangeInfos++;

			accessor->SetChangeInfoSerialNumber(g_pSharedChangeInfo->m_iSerialNumber);

			CEdictChangeInfo* p = &g_pSharedChangeInfo->m_ChangeInfos[accessor->GetChangeInfo()];
			p->m_ChangeOffsets[0] = offset;
			p->m_nChangeOffsets = 1;
		}
	}
}



inline void CBaseEdict::SetFree()
{
	m_fStateFlags |= FL_EDICT_FREE;
}

// WARNING: Make sure you don't really want to call ED_ClearFreeFlag which will also
//  remove this edict from the g_FreeEdicts bitset.
inline void CBaseEdict::ClearFree()
{
	m_fStateFlags &= ~FL_EDICT_FREE;
}

inline bool	CBaseEdict::IsDontSend() {
	return (m_fStateFlags & FL_EDICT_DONTSEND) != 0;
}

inline bool	CBaseEdict::IsPendingDormantCheck() {
	return (m_fStateFlags & FL_EDICT_PENDING_DORMANT_CHECK) != 0;
}

inline void	CBaseEdict::SetPendingDormantCheck() {
	m_fStateFlags |= FL_EDICT_PENDING_DORMANT_CHECK;
}

inline void	CBaseEdict::ClearPendingDormantCheck() {
	m_fStateFlags &= ~FL_EDICT_PENDING_DORMANT_CHECK;
}

inline void	CBaseEdict::ClearTransmitState()
{
	m_fStateFlags &= ~(FL_EDICT_ALWAYS | FL_EDICT_PVSCHECK | FL_EDICT_DONTSEND);
}

inline void	CBaseEdict::SetDirtyPvsInformation() {
	m_fStateFlags |= FL_EDICT_DIRTY_PVS_INFORMATION;
}

inline const IServerEntity* CBaseEdict::GetIServerEntity() const
{
	if (m_fStateFlags & FL_EDICT_FULL)
		return (IServerEntity*)m_pUnk;
	else
		return 0;
}

inline IServerUnknown* CBaseEdict::GetUnknown()
{
	return m_pUnk;
}

inline IServerNetworkable* CBaseEdict::GetNetworkable()
{
	return m_pNetworkable;
}

inline void CBaseEdict::SetEdict(IServerUnknown* pUnk, bool bFullEdict)
{
	m_pUnk = pUnk;
	if ((pUnk != NULL) && bFullEdict)
	{
		m_fStateFlags = FL_EDICT_FULL;
	}
	else
	{
		m_fStateFlags = 0;
	}
	// Cache our IServerNetworkable pointer for the engine for fast access.
	if (pUnk) {
		m_pNetworkable = pUnk->GetNetworkable();
	}
	else {
		m_pNetworkable = NULL;
	}
}

inline int CBaseEdict::AreaNum() const
{
	if (!m_pUnk)
		return 0;

	return m_pNetworkable->AreaNum();
}

inline int CBaseEdict::GetIndex() {
	return m_EdictIndex;
}

inline short CBaseEdict::GetNetworkSerialNumber() {
	return m_NetworkSerialNumber;
}

inline void	CBaseEdict::SetNetworkSerialNumber(short NetworkSerialNumber) {
	m_NetworkSerialNumber = NetworkSerialNumber;
}

inline const char* CBaseEdict::GetClassName() const
{
	if (!m_pUnk)
		return "";
	return m_pNetworkable->GetClassName();
}

inline int& CBaseEdict::GetStateFlags() {
	return m_fStateFlags;
}

inline void CBaseEdict::SetChangeInfo(unsigned short info)
{
	GetChangeAccessor()->SetChangeInfo(info);
}

inline void CBaseEdict::SetChangeInfoSerialNumber(unsigned short sn)
{
	GetChangeAccessor()->SetChangeInfoSerialNumber(sn);
}

inline unsigned short CBaseEdict::GetChangeInfo() const
{
	return GetChangeAccessor()->GetChangeInfo();
}

inline unsigned short CBaseEdict::GetChangeInfoSerialNumber() const
{
	return GetChangeAccessor()->GetChangeInfoSerialNumber();
}

inline ICollideable* CBaseEdict::GetCollideable()
{
	IServerEntity* pEnt = GetIServerEntity();
	if (pEnt)
		return pEnt->GetCollideable();
	else
		return NULL;
}

void SV_AllocateEdicts()
{
	// Allocate server memory
	max_edicts = MAX_EDICTS;
	g_pEdicts = (CBaseEdict*)Hunk_AllocName(max_edicts * sizeof(CBaseEdict), "edicts");

	COMPILE_TIME_ASSERT(MAX_EDICT_BITS + 1 <= 8 * sizeof(g_pEdicts[0].m_EdictIndex));

	// Invoke the constructor so the vtable is set correctly..
	for (int i = 0; i < max_edicts; ++i)
	{
		new(&g_pEdicts[i]) CBaseEdict(i);
		//g_pEdicts[i].m_EdictIndex = i;
		g_pEdicts[i].freetime = 0;
	}
	ED_ClearFreeEdictList();

	edictchangeinfo = (IChangeInfoAccessor*)Hunk_AllocName(max_edicts * sizeof(IChangeInfoAccessor), "edictchangeinfo");
}

int SV_MAX_Edicts() {
	return max_edicts;
}

int SV_NUM_Edicts() {
	return num_edicts;
}

int SV_FREE_Edicts() {
	return free_edicts;
}

IChangeInfoAccessor* SV_Edictchangeinfo(int n) {
	return &edictchangeinfo[n];
}

void SV_DeallocateEdicts() {
	num_edicts = 0;
	max_edicts = 0;
	free_edicts = 0;
	g_pEdicts = NULL;
}
/*
=================
ED_ClearEdict

Sets everything to NULL, done when new entity is allocated for game.dll
=================
*/
static void ED_ClearEdict(CBaseEdict*e )
{
	e->ClearFree();
	e->ClearStateChanged();
	e->SetChangeInfoSerialNumber( 0 );
	
	serverGameEnts->FreeContainingEntity(e);
	InitializeEntityDLLFields(e);

	e->m_NetworkSerialNumber = -1;  // must be filled by game.dll
	Assert( (int) e->m_EdictIndex == (e - sv.edicts) );
}

void ED_ClearFreeFlag(edict_t* edict )
{
	CBaseEdict* ed = (CBaseEdict*)edict;
	ed->ClearFree();
	g_FreeEdicts.Clear( ed->m_EdictIndex );
}

void ED_ClearFreeEdictList()
{
	// Clear the free edicts bitfield.
	g_FreeEdicts.ClearAll();
}

static void SpewEdicts()
{
	CUtlMap< const char*, int > mapEnts;
	mapEnts.SetLessFunc( StringLessThan );

	// Tally up each class
	int nEdictNum = 1;
	for( int i=0; i<num_edicts; ++i )
	{
		edict_t *e = &g_pEdicts[i];
		++nEdictNum;
		unsigned short nIndex = mapEnts.Find( e->GetClassName() );
		if ( nIndex == mapEnts.InvalidIndex() )
		{
			nIndex = mapEnts.Insert( e->GetClassName() );
			mapEnts[ nIndex ] = 0;
		}
		mapEnts[ nIndex ] = mapEnts[ nIndex ] + 1;
	}

	struct EdictCount_t
	{
		EdictCount_t( const char *pszClassName, int nCount )
			:
			m_pszClassName( pszClassName ),
			m_nCount( nCount )
		{}

		const char *m_pszClassName;
		int m_nCount;
	};

	CUtlVector<EdictCount_t*> vecEnts;

	FOR_EACH_MAP_FAST( mapEnts, i )
	{
		vecEnts.AddToTail( new EdictCount_t( mapEnts.Key( i ), mapEnts[ i ] ) );
	}

	struct EdictSorter_t
	{
		static int SortEdicts( EdictCount_t* const *p1, EdictCount_t* const *p2 )
		{
			return (*p1)->m_nCount - (*p2)->m_nCount;
		}
	};

	vecEnts.Sort( &EdictSorter_t::SortEdicts );

	g_Log.Printf( "Spewing edict counts:\n" );
	FOR_EACH_VEC( vecEnts, i )
	{
		g_Log.Printf( "(%3.2f%%) %d\t%s\n", vecEnts[i]->m_nCount/(float)nEdictNum * 100.f, vecEnts[i]->m_nCount, vecEnts[i]->m_pszClassName );
	}
	g_Log.Printf( "Total edicts: %d\n", nEdictNum );

	vecEnts.PurgeAndDeleteElements();
}

/*
=================
ED_Alloc

Either finds a free edict, or allocates a new one.
Try to avoid reusing an entity that was recently freed, because it
can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/


edict_t *ED_Alloc( int iForceEdictIndex )
{
	if ( iForceEdictIndex >= 0 )
	{
		if ( iForceEdictIndex >= num_edicts )
		{
			Warning( "ED_Alloc( %d ) - invalid edict index specified.", iForceEdictIndex );
			return NULL;
		}
		
		CBaseEdict *e = &g_pEdicts[ iForceEdictIndex ];
		if ( e->IsFree() )
		{
			Assert( iForceEdictIndex == e->m_EdictIndex );
			--free_edicts;
			Assert( g_FreeEdicts.IsBitSet( iForceEdictIndex ) );
			g_FreeEdicts.Clear( iForceEdictIndex );
			ED_ClearEdict( e );
			return e;
		}
		else
		{
			return NULL;
		}
	}

	// Check the free list first.
	int iBit = -1;
	for ( ;; )
	{
		iBit = g_FreeEdicts.FindNextSetBit( iBit + 1 );
		if ( iBit < 0 )
			break;

		CBaseEdict *pEdict = &g_pEdicts[ iBit ];

		// If this assert goes off, someone most likely called pedict->ClearFree() and not ED_ClearFreeFlag()?
		Assert( pEdict->IsFree() );
		Assert( iBit == pEdict->m_EdictIndex );
		if ( ( pEdict->freetime < 2 ) || ( sv.GetTime() - pEdict->freetime >= EDICT_FREETIME ) )
		{
			// If we have no freetime, we've had AllowImmediateReuse() called. We need
			// to explicitly delete this old entity.
			if ( pEdict->freetime == 0 && sv_useexplicitdelete.GetBool() )
			{
				//Warning("ADDING SLOT to snapshot: %d\n", i );
				framesnapshotmanager->AddExplicitDelete( iBit );
			}

			--free_edicts;
			g_FreeEdicts.Clear( pEdict->m_EdictIndex );
			ED_ClearEdict( pEdict );
			return pEdict;
		}
	}

	// Allocate a new edict.
	if ( num_edicts >= max_edicts )
	{
		AssertMsg( 0, "Can't allocate edict" );

		SpewEdicts(); // Log the entities we have before we die

		if ( max_edicts == 0 )
			Sys_Error( "ED_Alloc: No edicts yet" );
		Sys_Error ("ED_Alloc: no free edicts");
	}

	// Do this before clearing since clear now needs to call back into the edict to deduce the index so can get the changeinfo data in the parallel structure
	CBaseEdict *pEdict = g_pEdicts + num_edicts++;

	// We should not be in the free list...
	Assert( !g_FreeEdicts.IsBitSet( pEdict->m_EdictIndex ) );
	ED_ClearEdict( pEdict );

	if ( sv_lowedict_action.GetInt() > 0 && num_edicts >= max_edicts - sv_lowedict_threshold.GetInt() )
	{
		int nEdictsRemaining = max_edicts - num_edicts;
		g_Log.Printf( "Warning: free edicts below threshold. %i free edict%s remaining.\n", nEdictsRemaining, nEdictsRemaining == 1 ? "" : "s" );

		switch ( sv_lowedict_action.GetInt() )
		{
		case 2:
			// restart the game
			{
				ConVarRef mp_restartgame_immediate( "mp_restartgame_immediate" );
				if ( mp_restartgame_immediate.IsValid() )
				{
					mp_restartgame_immediate.SetValue( 1 );
				}
				else
				{
					ConVarRef mp_restartgame( "mp_restartgame" );
					if ( mp_restartgame.IsValid() )
					{
						mp_restartgame.SetValue( 1 );
					}
				}
			}
			break;
		case 3:
			// restart the map
			g_pVEngineServer->ChangeLevel( sv.GetMapName(), NULL );
			break;
		case 4:
			// go to the next map
			g_pVEngineServer->ServerCommand( "changelevel_next\n" );
			break;
		case 5:
			// spew all edicts
			SpewEdicts();
			break;
		}
	}
	
	return pEdict;
}


void ED_AllowImmediateReuse()
{
	CBaseEdict *pEdict = g_pEdicts + sv.GetMaxClients() + 1;
	for ( int i=sv.GetMaxClients()+1; i < num_edicts; i++ )
	{
		if ( pEdict->IsFree() )
		{
			pEdict->freetime = 0;
		}

		pEdict++;
	}
}


/*
=================
ED_Free

Marks the edict as free
FIXME: walk all entities and NULL out references to this entity
=================
*/
void ED_Free (edict_t* edict)
{
	CBaseEdict* ed = (CBaseEdict*)edict;
	if (ed->IsFree())
	{
#ifdef _DEBUG
//		ConDMsg("duplicate free on '%s'\n", pr_strings + ed->classname );
#endif
		return;
	}

	// don't free player edicts
	if ( (ed - g_pEdicts) >= 1 && (ed - g_pEdicts) <= sv.GetMaxClients() )
		return;

	// release the DLL entity that's attached to this edict, if any
	serverGameEnts->FreeContainingEntity( ed );

	ed->SetFree();
	ed->freetime = sv.GetTime();

	++free_edicts;
	Assert( !g_FreeEdicts.IsBitSet( ed->m_EdictIndex ) );
	g_FreeEdicts.Set( ed->m_EdictIndex );

	// Increment the serial number so it knows to send explicit deletes the clients.
	ed->m_NetworkSerialNumber++; 
}

//
// 	serverGameEnts->FreeContainingEntity( pEdict )  frees up memory associated with a DLL entity.
// InitializeEntityDLLFields clears out fields to NULL or UNKNOWN.
// Release is for terminating a DLL entity.  Initialize is for initializing one.
//
void InitializeEntityDLLFields(edict_t* edict )
{
	CBaseEdict* ed = (CBaseEdict*)edict;
	// clear all the game variables
	size_t sz = offsetof(CBaseEdict, m_pUnk ) + sizeof( void* );
	memset( ((byte*)ed) + sz, 0, sizeof(CBaseEdict) - sz );
}

int NUM_FOR_EDICT(const edict_t* edict)
{
	CBaseEdict* ed = (CBaseEdict*)edict;
	if ( &g_pEdicts[ed->m_EdictIndex] == ed ) // NOTE: old server.dll may stomp m_EdictIndex
		return ed->m_EdictIndex;
	int index = ed - g_pEdicts;
	if ( (unsigned int) index >= (unsigned int) num_edicts )
		Sys_Error ("NUM_FOR_EDICT: bad pointer");
	return index;
}

edict_t *EDICT_NUM(int n)
{
	if ((unsigned int) n >= (unsigned int) max_edicts)
		Sys_Error ("EDICT_NUM: bad number %i", n);
	if (n == 0) {
		return g_pEdicts;
	}
	return &g_pEdicts[n];
}


static inline int NUM_FOR_EDICTINFO(const edict_t* edict)
{
	CBaseEdict* ed = (CBaseEdict*)edict;
	if ( &g_pEdicts[ed->m_EdictIndex] == ed ) // NOTE: old server.dll may stomp m_EdictIndex
		return ed->m_EdictIndex;
	int index = ed - g_pEdicts;
	if ( (unsigned int) index >= (unsigned int) max_edicts )
		Sys_Error ("NUM_FOR_EDICTINFO: bad pointer");
	return index;
}


IChangeInfoAccessor *CBaseEdict::GetChangeAccessor()
{
	return &edictchangeinfo[ NUM_FOR_EDICTINFO( (const CBaseEdict*)this ) ];
}

const IChangeInfoAccessor *CBaseEdict::GetChangeAccessor() const
{
	return &edictchangeinfo[ NUM_FOR_EDICTINFO( (const CBaseEdict*)this ) ];
}
