//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef PLAYERSTATE_H
#define PLAYERSTATE_H
#ifdef _WIN32
#pragma once
#endif

#include "edict.h"
#include "networkvar.h"
// Only care about this stuff in game/client .dlls
#if defined( CLIENT_DLL )
#include "predictable_entity.h"
#endif

class CPlayerState
{
public:
	DECLARE_CLASS_NOBASE( CPlayerState );
	DECLARE_EMBEDDED_NETWORKVAR();
	
	// This virtual method is necessary to generate a vtable in all cases
	// (DECLARE_PREDICTABLE will generate a vtable also)!
	virtual ~CPlayerState() {}

	// true if the player is dead
	CNetworkVar( bool, deadflag );	
	// Viewing angle (player only)
	QAngle		v_angle;		
	
// The client .dll only cares about deadflag
//  the game and engine .dlls need to worry about the rest of this data
#if !defined( CLIENT_DLL )
	// Player's network name
	string_t	netname;
	// 0:nothing, 1:force view angles, 2:add avelocity
	int			fixangle;
	// delta angle for fixangle == FIXANGLE_RELATIVE
	QAngle		anglechange;
	// flag to single the HLTV/Replay fake client, not transmitted
	bool		hltv;
	bool		replay;
	int			frags;
	int			deaths;
#endif

// NOTE:  Only care about this stuff in game/client dlls
// Put at end in case it has any effect on size of structure
#if defined( GAME_DLL )
	DECLARE_SIMPLE_DATADESC();
	DECLARE_SEND_TABLE_ACCESS(DT_PlayerState);
#endif

#if defined( CLIENT_DLL )
	DECLARE_PREDICTABLE();
#endif

	// -------------------------------------------------------------------------------- //
// SendTable for CPlayerState.
// -------------------------------------------------------------------------------- //

#if defined( GAME_DLL )
	BEGIN_SEND_TABLE_NOBASE(CPlayerState, DT_PlayerState)
		SendPropInt(SENDINFO(deadflag), 1, SPROP_UNSIGNED),
	END_SEND_TABLE(DT_PlayerState)
#endif

#ifdef CLIENT_DLL
	BEGIN_RECV_TABLE_NOBASE(CPlayerState, DT_PlayerState)
		RecvPropInt(RECVINFO(deadflag)),
	END_RECV_TABLE(DT_PlayerState)
#endif
};

#endif // PLAYERSTATE_H
