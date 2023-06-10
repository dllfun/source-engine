//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef EDICT_H
#define EDICT_H

#ifdef _WIN32
#pragma once
#endif

#include "mathlib/vector.h"
#include "cmodel.h"
#include "const.h"
#include "iserverentity.h"
#include "globalvars_base.h"
#include "engine/ICollideable.h"
#include "iservernetworkable.h"
#include "bitvec.h"

class edict_t;


//-----------------------------------------------------------------------------
// Purpose: Defines the ways that a map can be loaded.
//-----------------------------------------------------------------------------
enum MapLoadType_t
{
	MapLoad_NewGame = 0,
	MapLoad_LoadGame,
	MapLoad_Transition,
	MapLoad_Background,
};


//-----------------------------------------------------------------------------
// Purpose: Global variables shared between the engine and the game .dll
//-----------------------------------------------------------------------------
class CGlobalVars : public CGlobalVarsBase
{	
public:

	CGlobalVars( bool bIsClient );

	float GetCurTime() {
		return curtime;
	}

	void SetCurTime(float fCurTime) {
		curtime = fCurTime;
	}

	void SetFrameTime(float fFrametime) {
		frametime = fFrametime;
	}

	void SetSaveData(CSaveRestoreData* ppSaveData) {
		pSaveData = ppSaveData;
	}

	void SetMaxClients(int iMaxClients) {
		maxClients = iMaxClients;
	}

	int GetTickCount() {
		return tickcount;
	}

	void SetTickCount(int iTickcount) {
		tickcount = iTickcount;
	}

	int GetFrameCount() {
		return framecount;
	}

	void SetFrameCount(int iFrameCount) {
		framecount = iFrameCount;
	}

	float GetRealTime() {
		return realtime;
	}

	void SetRealTime(float fRealTime) {
		realtime = fRealTime;
	}

	void SetAbsoluteFrameTime(float fAbsoluteFrameTime) {
		absoluteframetime = fAbsoluteFrameTime;
	}

	void SetIntervalPerTick(float iinterval_per_tick) {
		interval_per_tick = iinterval_per_tick;
	}

	void SetSimTicksThisFrame(int iSimTicksThisFrame) {
		simTicksThisFrame = iSimTicksThisFrame;
	}

	int GetSimTicksThisFrame() {
		return simTicksThisFrame;
	}

	void SetInterpolationAmount(float iinterpolation_amount) {
		interpolation_amount = iinterpolation_amount;
	}
public:
	
	// Current map
	string_t		mapname;
	int				mapversion;
	string_t		startspot;
	MapLoadType_t	eLoadType;		// How the current map was loaded
	bool			bMapLoadFailed;	// Map has failed to load, we need to kick back to the main menu

	// game specific flags
	bool			deathmatch;
	bool			coop;
	bool			teamplay;
	// current maxentities
	int				maxEntities;

	int				serverCount;
};

inline CGlobalVars::CGlobalVars( bool bIsClient ) : 
	CGlobalVarsBase( bIsClient )
{
	serverCount = 0;
}


class CPlayerState;
class IServerNetworkable;
class IServerEntity;


#define FL_EDICT_CHANGED	(1<<0)	// Game DLL sets this when the entity state changes
									// Mutually exclusive with FL_EDICT_PARTIAL_CHANGE.
									
#define FL_EDICT_FREE		(1<<1)	// this edict if free for reuse
#define FL_EDICT_FULL		(1<<2)	// this is a full server entity

#define FL_EDICT_FULLCHECK	(0<<0)  // call ShouldTransmit() each time, this is a fake flag
#define FL_EDICT_ALWAYS		(1<<3)	// always transmit this entity
#define FL_EDICT_DONTSEND	(1<<4)	// don't transmit this entity
#define FL_EDICT_PVSCHECK	(1<<5)	// always transmit entity, but cull against PVS

// Used by local network backdoor.
#define FL_EDICT_PENDING_DORMANT_CHECK	(1<<6)

// This is always set at the same time EFL_DIRTY_PVS_INFORMATION is set, but it 
// gets cleared in a different place.
#define FL_EDICT_DIRTY_PVS_INFORMATION	(1<<7)

// This is used internally to edict_t to remember that it's carrying a 
// "full change list" - all its properties might have changed their value.
#define FL_FULL_EDICT_CHANGED			(1<<8)


// Max # of variable changes we'll track in an entity before we treat it
// like they all changed.
#define MAX_CHANGE_OFFSETS	19
#define MAX_EDICT_CHANGE_INFOS	100


class CEdictChangeInfo
{
public:
	// Edicts remember the offsets of properties that change 
	unsigned short m_ChangeOffsets[MAX_CHANGE_OFFSETS];
	unsigned short m_nChangeOffsets;
};

// Shared between engine and game DLL.
class CSharedEdictChangeInfo
{
public:
	CSharedEdictChangeInfo()
	{
		m_iSerialNumber = 1;
	}
	
	// Matched against edict_t::m_iChangeInfoSerialNumber to determine if its
	// change info is valid.
	unsigned short m_iSerialNumber;
	
	CEdictChangeInfo m_ChangeInfos[MAX_EDICT_CHANGE_INFOS];
	unsigned short m_nChangeInfos;	// How many are in use this frame.
};
extern CSharedEdictChangeInfo *g_pSharedChangeInfo;

class IChangeInfoAccessor
{
public:
	inline void SetChangeInfo( unsigned short info )
	{
		m_iChangeInfo = info;
	}

	inline void SetChangeInfoSerialNumber( unsigned short sn )
	{
		m_iChangeInfoSerialNumber = sn;
	}

	inline unsigned short	 GetChangeInfo() const
	{
		return m_iChangeInfo;
	}

	inline unsigned short	 GetChangeInfoSerialNumber() const
	{
		return m_iChangeInfoSerialNumber;
	}

private:
	unsigned short m_iChangeInfo;
	unsigned short m_iChangeInfoSerialNumber;
};



//-----------------------------------------------------------------------------
// Purpose: The engine's internal representation of an entity, including some
//  basic collision and position info and a pointer to the class wrapped on top
//  of the structure
//-----------------------------------------------------------------------------
class edict_t
{
public:
	
	// Returns an IServerEntity if FL_FULLEDICT is set or NULL if this 
	// is a lightweight networking entity.
	virtual IServerEntity* GetIServerEntity() = 0;
	virtual const IServerEntity* GetIServerEntity() const = 0;

	virtual IServerNetworkable* GetNetworkable() = 0;
	virtual IServerUnknown* GetUnknown() = 0;

	// Set when initting an entity. If it's only a networkable, this is false.
	virtual void				SetEdict(IServerUnknown* pUnk, bool bFullEdict) = 0;

	virtual int					AreaNum() const = 0;
	virtual int					GetIndex() = 0;
	virtual short				GetNetworkSerialNumber() = 0;
	virtual void				SetNetworkSerialNumber(short NetworkSerialNumber) = 0;
	virtual const char* GetClassName() const = 0;
	virtual int&				GetStateFlags() = 0;

	virtual bool				IsFree() const = 0;
	virtual void				SetFree() = 0;
	virtual void				ClearFree() = 0;

	virtual bool				HasStateChanged() const = 0;
	virtual void				ClearStateChanged() = 0;
	virtual void				StateChanged() = 0;
	virtual void				StateChanged(unsigned short offset) = 0;

	virtual bool				IsDontSend() = 0;
	virtual bool				IsPendingDormantCheck() = 0;
	virtual void				SetPendingDormantCheck() = 0;
	virtual void				ClearPendingDormantCheck() = 0;
	virtual void				ClearTransmitState() = 0;

	virtual void				SetDirtyPvsInformation() = 0;

	virtual void SetChangeInfo(unsigned short info) = 0;
	virtual void SetChangeInfoSerialNumber(unsigned short sn) = 0;
	virtual unsigned short	 GetChangeInfo() const = 0;
	virtual unsigned short	 GetChangeInfoSerialNumber() const = 0;
	virtual IChangeInfoAccessor* GetChangeAccessor() = 0; // The engine implements this and the game .dll implements as
	virtual const IChangeInfoAccessor* GetChangeAccessor() const = 0; // The engine implements this and the game .dll implements as
	// as callback through to the engine!!!
	virtual ICollideable *GetCollideable() = 0;

	
};



void SV_AllocateEdicts();
int SV_MAX_Edicts();
int SV_NUM_Edicts();
int SV_FREE_Edicts();
IChangeInfoAccessor* SV_Edictchangeinfo(int n);
// If iForceEdictIndex is not -1, then it will return the edict with that index. If that edict index
// is already used, it'll return null.
edict_t* ED_Alloc(int iForceEdictIndex = -1);
void	ED_Free(edict_t* ed);

// Clear the FL_EDICT_FREE flag and the g_FreeEdicts bit.
void	ED_ClearFreeFlag(edict_t* pEdict);

edict_t* EDICT_NUM(int n);
int NUM_FOR_EDICT(const edict_t* e);

void ED_AllowImmediateReuse();
void ED_ClearFreeEdictList();
void SV_DeallocateEdicts();

#endif // EDICT_H
