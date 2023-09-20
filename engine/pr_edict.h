//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Expose things from pr_edict.cpp.
//
// $NoKeywords: $
//===========================================================================//

#ifndef PR_EDICT_H
#define PR_EDICT_H


#include "edict.h"

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
	const char*			GetClassName() const;
	int&				GetStateFlags();

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

	void				SetChangeInfo(unsigned short info);
	void				SetChangeInfoSerialNumber(unsigned short sn);
	unsigned short		GetChangeInfo() const;
	unsigned short		GetChangeInfoSerialNumber() const;
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

	friend void InitializeEntityDLLFields(CBaseEdict* pEdict);
};

void InitializeEntityDLLFields(CBaseEdict* pEdict );

void SV_AllocateEdicts();
int SV_MAX_Edicts();
int SV_NUM_Edicts();
int SV_FREE_Edicts();
IChangeInfoAccessor* SV_Edictchangeinfo(int n);
// If iForceEdictIndex is not -1, then it will return the edict with that index. If that edict index
// is already used, it'll return null.
CBaseEdict* ED_Alloc(int iForceEdictIndex = -1);
void	ED_Free(CBaseEdict* ed);
// Clear the FL_EDICT_FREE flag and the g_FreeEdicts bit.
void	ED_ClearFreeFlag(CBaseEdict* pEdict);
CBaseEdict* EDICT_NUM(int n);
int NUM_FOR_EDICT(const CBaseEdict* e);
void ED_AllowImmediateReuse();
void ED_ClearFreeEdictList();
void SV_DeallocateEdicts();

#endif


