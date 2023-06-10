//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef GLOBALVARS_BASE_H
#define GLOBALVARS_BASE_H

#ifdef _WIN32
#pragma once
#endif

class CSaveRestoreData;

//-----------------------------------------------------------------------------
// Purpose: Global variables used by shared code
//-----------------------------------------------------------------------------
class CGlobalVarsBase
{
public:

	CGlobalVarsBase( bool bIsClient );
	
	// This can be used to filter debug output or to catch the client or server in the act.
	bool IsClient() const;

	// for encoding m_flSimulationTime, m_flAnimTime
	int GetNetworkBase( int nTick, int nEntity );

public:
	
	void SetCurTime(float fCurtime) {
		curtime = fCurtime;
	}

	float GetCurTime() {
		return curtime;
	}

	void SetAbsoluteFrameTime(float fAbsoluteframetime) {
		absoluteframetime = fAbsoluteframetime;
	}

	float GetAbsoluteFrameTime() {
		return absoluteframetime;
	}

	void SetFrameTime(float fFrameTime) {
		frametime = fFrameTime;
	}

	float GetFrameTime() {
		return frametime;
	}

	void SetFrameCount(int iFramecount) {
		framecount = iFramecount;
	}

	int GetFrameCount() {
		return framecount;
	}

	int GetMaxClients() {
		return maxClients;
	}

	void SetMaxClients(int iMaxClients) {
		maxClients = iMaxClients;
	}

	void SetIntervalPerTick(float fIntervalPerTick) {
		interval_per_tick = fIntervalPerTick;
	}

	float GetIntervalPerTick() {
		return interval_per_tick;
	}

	void SetRealTime(float fRealtime) {
		realtime = fRealtime;
	}

	float GetRealTime() {
		return realtime;
	}

	int GetTickCount() {
		return tickcount;
	}

	void SetTickCount(int iTickCount) {
		tickcount = iTickCount;
	}

	void SetSaveData(CSaveRestoreData* ppSaveData) {
		pSaveData = ppSaveData;
	}

	CSaveRestoreData* GetSaveData() {
		return pSaveData;
	}

	void SetInterpolationAmount(float fInterpolationAmount) {
		interpolation_amount = fInterpolationAmount;
	}

	float GetInterpolationAmount() {
		return interpolation_amount;
	}

	void SetSimTicksThisFrame(int iSimTicksThisFrame) {
		simTicksThisFrame = iSimTicksThisFrame;
	}

	int GetSimTicksThisFrame() {
		return simTicksThisFrame;
	}

	int GetNetworkProtocol() {
		return network_protocol;
	}

	void SetNetworkProtocol(int iNetwork_protocol) {
		network_protocol = iNetwork_protocol;
	}
protected:
	// Absolute time (per frame still - Use Plat_FloatTime() for a high precision real time 
	//  perf clock, but not that it doesn't obey host_timescale/host_framerate)
	float			realtime;
	// Absolute frame counter
	int				framecount;
	// Non-paused frametime
	float			absoluteframetime;

	// Current time 
	//
	// On the client, this (along with tickcount) takes a different meaning based on what
	// piece of code you're in:
	// 
	//   - While receiving network packets (like in PreDataUpdate/PostDataUpdate and proxies),
	//     this is set to the SERVER TICKCOUNT for that packet. There is no interval between
	//     the server ticks.
	//     [server_current_Tick * tick_interval]
	//
	//   - While rendering, this is the exact client clock 
	//     [client_current_tick * tick_interval + interpolation_amount]
	//
	//   - During prediction, this is based on the client's current tick:
	//     [client_current_tick * tick_interval]
	float			curtime;
	
	// Time spent on last server or client frame (has nothing to do with think intervals)
	float			frametime;
	// current maxplayers setting
	int				maxClients;

	// Simulation ticks
	int				tickcount;

	// Simulation tick interval
	float			interval_per_tick;

	// interpolation amount ( client-only ) based on fraction of next tick which has elapsed
	float			interpolation_amount;
	int				simTicksThisFrame;

	int				network_protocol;

	// current saverestore data
	CSaveRestoreData *pSaveData;

private:
	// Set to true in client code.
	bool			m_bClient;

	// 100 (i.e., tickcount is rounded down to this base and then the "delta" from this base is networked
	int				nTimestampNetworkingBase;   
	// 32 (entindex() % nTimestampRandomizeWindow ) is subtracted from gpGlobals->GetTickCount() to set the networking basis, prevents
	//  all of the entities from forcing a new PackedEntity on the same tick (i.e., prevents them from getting lockstepped on this)
	int				nTimestampRandomizeWindow;  
	
};

inline int CGlobalVarsBase::GetNetworkBase( int nTick, int nEntity )
{
	int nEntityMod = nEntity % nTimestampRandomizeWindow;
	int nBaseTick = nTimestampNetworkingBase * (int)( ( nTick - nEntityMod ) / nTimestampNetworkingBase );
	return nBaseTick;
}

inline CGlobalVarsBase::CGlobalVarsBase( bool bIsClient ) :
	m_bClient( bIsClient ),
	nTimestampNetworkingBase( 100 ),
	nTimestampRandomizeWindow( 32 )
{
}

inline bool CGlobalVarsBase::IsClient() const
{
	return m_bClient;
}

#endif // GLOBALVARS_BASE_H
