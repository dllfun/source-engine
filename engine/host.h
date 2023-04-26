//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#if !defined( HOST_H )
#define HOST_H
#ifdef _WIN32
#pragma once
#endif

#include "convar.h"
#include "steam/steamclientpublic.h"

#define SCRIPT_DIR			"scripts/"

class model_t;
struct AudioState_t;


//class CCommonHostState
//{
//public:
//	
//	void SetWorldModel( model_t *pModel );
//};

//extern CCommonHostState host_state;

//=============================================================================
// the host system specifies the base of the directory tree, the mod + base mod
// and the amount of memory available for the program to use
struct engineparms_t
{
	char	*basedir;	// Executable directory ("c:/program files/half-life 2", for example)
	char	*mod;		// Mod name ("cstrike", for example)
	char	*game;		// Root game name ("hl2", for example, in the case of cstrike)
	unsigned int	memsize;
};
//extern engineparms_t host_parms;

class NET_SetConVar;
class CGameClient;

class Host {
public:

	unsigned int GetMemSize() {
		return host_parms.memsize;
	}

	void SetMemSize(unsigned int memSize) {
		host_parms.memsize = memSize;
	}

	const char* GetMod()
	{
		return host_parms.mod;
	}

	void SetMod(char* mod) {
		host_parms.mod = mod;
	}

	const char* GetGameDir()
	{
		return host_parms.game;
	}

	void SetGameDir(char* gameDir) {
		host_parms.game = gameDir;
	}

	const char* GetBaseDir() {
		return host_parms.basedir;
	}

	void SetBaseDir(char* baseDir) {
		host_parms.basedir = baseDir;
	}

	float Host_GetFrameTime() {
		return host_frametime;
	}

	float Host_GetFrameTimeUnbounded() {
		return host_frametime_unbounded;
	}

	float Host_GetFrameTimeStddeviation() {
		return host_frametime_stddeviation;
	}

	int Host_GetTickCount() {
		return host_tickcount;
	}

	// Accumulated filtered time on machine ( i.e., host_framerate can alter host_time )
	double Host_GetRealTime() {
		return host_realtime;
	}

	//extern float host_tick_time;
	float Host_GetTickTime() {
		return host_tick_time;
	}

	int Host_GetFrameCount() {
		return host_framecount;
	}

	float Host_GetNextTick() {
		return host_nexttick;
	}

	int Host_GetFrameTicks() {
		return host_frameticks;
	}

	int Host_GetCurrentFrameTick() {
		return host_currentframetick;
	}

	double Host_GetIdealTime() {
		return host_idealtime;
	}

	float Host_GetIntervalPerTick() {
		return interval_per_tick;
	}

	void Host_SetIntervalPerTick(float interval_per_tick) {
		this->interval_per_tick= interval_per_tick;
	}

	model_t* Host_GetWorldModel() {
		return worldmodel;
	}

	void Host_SetWorldModel(model_t* model) {
		worldmodel = model;
	}

	bool Host_initialized() {
		return host_initialized;
	}

	CGameClient* Host_GetClient() {
		return host_client;
	}

	void Host_SetClient(CGameClient* client) {
		host_client = client;
	}

	void Host_CheckGore(void);

	void Host_Init(bool bIsDedicated);
	void Host_ReadConfiguration();
	void Host_WriteConfiguration(const char* filename = NULL, bool bAllVars = false);
	void Host_Shutdown(void);
	//int  Host_Frame (float time, int iState );
	void Host_ShutdownServer(void);
	bool Host_NewGame(char* mapName, bool loadGame, bool bBackgroundLevel, const char* pszOldMap = NULL, const char* pszLandmark = NULL, bool bOldSave = false);
	bool Host_Changelevel(bool loadfromsavedgame, const char* mapname, const char* start);
	void Disconnect();
	// start of every frame, never reset
	void Host_Error(PRINTF_FORMAT_STRING const char* error, ...) FMTFUNCTION(1, 2);
	void Host_EndGame(bool bShowMainMenu, PRINTF_FORMAT_STRING const char* message, ...) FMTFUNCTION(2, 3);

	// Returns true if host is not single stepping/pausing through code/
// FIXME:  Remove from final, retail version of code.
	bool Host_ShouldRun(void);
	void Host_FreeToLowMark(bool server);
	void Host_FreeStateAndWorld(bool server);
	void Host_Disconnect(bool bShowMainMenu, const char* pszReason = "");
	void Host_RunFrame(float time);
	void Host_DumpMemoryStats(void);
	void Host_UpdateMapList(void);
	float Host_GetSoundDuration(const char* pSample);
	bool Host_IsSinglePlayerGame(void);
	int Host_GetServerCount(void);
	bool Host_AllowQueuedMaterialSystem(bool bAllow);

	bool Host_IsSecureServerAllowed();
	void FORCEINLINE Host_DisallowSecureServers()
	{
#if !defined(SWDS)
		g_bAllowSecureServers = false;
#endif
	}

	bool		Host_AllowLoadModule(const char* pFilename, const char* pPathID, bool bAllowUnknown, bool bIsServerOnly = false);
	void		Host_BuildConVarUpdateMessage(NET_SetConVar* cvarMsg, int flags, bool nonDefault);
	char const* Host_CleanupConVarStringValue(char const* invalue);
	void		Host_SetAudioState(const AudioState_t& audioState);
	void		Host_DefaultMapFileName(const char* pFullMapName, /* out */ char* pDiskName, unsigned int nDiskNameSize);
	//-----------------------------------------------------------------------------
// Human readable methods to get at engineparms info
//-----------------------------------------------------------------------------
	
	void Host_AccumulateTime(float dt);

	// Globals
	int	gHostSpawnCount = 0;
	int g_HostErrorCount = 0;
	// These counters are for debugging in dumps.  If these are non-zero it may indicate some kind of 
	// heap problem caused by the setjmp/longjmp error handling
	int g_HostServerAbortCount = 0;
	int g_HostEndDemo = 0;
#ifndef SWDS
	bool g_bInEditMode = false;
	bool g_bInCommentaryMode = false;
#endif
	bool g_bLowViolence = false;
	bool g_bDedicatedServerBenchmarkMode = false;

private:
	void _Host_SetGlobalTime();
	void _Host_RunFrame(float time);
	void Host_ShowIPCCallCount();
	void Host_AbortServer()
	{
		g_HostServerAbortCount++;
		longjmp(host_abortserver, 1);
	}

	bool		host_initialized = false;		// true if into command execution
	int			host_hunklevel;
	bool		g_bAllowSecureServers = true;
	bool		g_bAbortServerSet = false;

	float		host_frametime_unbounded = 0.0f;
	float		host_frametime_stddeviation = 0.0f;
	jmp_buf 	host_abortserver;
	jmp_buf     host_enddemo;
	CGameClient* host_client;			// current client

	engineparms_t host_parms;
	//CCommonHostState host_state;
	model_t* worldmodel;	// cl_entitites[0].model
	//struct worldbrushdata_t *worldbrush;
	float		interval_per_tick;		// Tick interval for game
	float		host_tick_time = 0.0;
	float		host_frametime = 0.0f;
	int			host_tickcount = 0;
	double		host_realtime = 0;			// without any filtering or bounding
	int			host_framecount;
	float		host_nexttick = 0;		// next server tick in this many ms
	int			host_frameticks = 0;
	int			host_currentframetick = 0;
	double		host_idealtime = 0;		// "ideal" server time assuming perfect tick rate
	friend class CFrameTimer;
};

extern Host* g_pHost;


//=============================================================================

//
// host
// FIXME, move all this crap somewhere else
//

extern	ConVar		developer;
//extern	bool		host_initialized;		// true if into command execution
//extern	float		host_frametime;
//extern  float		host_frametime_unbounded;
//extern  float		host_frametime_stddeviation;
//extern	int			host_framecount;	// incremented every frame, never reset
//extern	double		host_realtime;			// not bounded in any way, changed at


// user message
#define MAX_USER_MSG_DATA 255

// build info
// day counter from Sep 30 2003
extern int build_number( void );


// Choke local client's/server's packets?
extern  ConVar		host_limitlocal;      
// Print a debug message when the client or server cache is missed
extern	ConVar		host_showcachemiss;

//extern bool			g_bInEditMode;
//extern bool			g_bInCommentaryMode;
//extern bool			g_bAllowSecureServers;
//extern bool			g_bLowViolence;


// Force the voice stuff to send a packet out.
// bFinal is true when the user is done talking.
void CL_SendVoicePacket(bool bFinal);



bool CheckVarRange_Generic( ConVar *pVar, int minVal, int maxVal );

// Total ticks run
//extern int	host_tickcount;
// Number of ticks being run this frame
//extern int	host_frameticks;
// Which tick are we currently on for this frame
//extern int	host_currentframetick;

// PERFORMANCE INFO
#define MIN_FPS         0.1         // Host minimum fps value for maxfps.
#define MAX_FPS         1000.0        // Upper limit for maxfps.

#define MAX_FRAMETIME	0.1
#define MIN_FRAMETIME	0.001

#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / g_pHost->Host_GetIntervalPerTick() ) )
#define TICKS_TO_TIME( dt )		( g_pHost->Host_GetIntervalPerTick() * (float)(dt) )

// Normally, this is off, and it keeps the VCR file size smaller, but it can help
// to turn it on when tracking down out-of-sync errors, because it verifies that more
// things are the same during playback.
extern ConVar vcr_verbose;

// Set by the game DLL to tell us to do the same timing tricks as timedemo.
//extern bool g_bDedicatedServerBenchmarkMode;

extern uint GetSteamAppID();
extern EUniverse GetSteamUniverse();

#define STEAMREMOTESTORAGE_CLOUD_OFF	0
#define STEAMREMOTESTORAGE_CLOUD_ON		1

#endif // HOST_H

