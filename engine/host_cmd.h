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
#if !defined( HOST_CMD_H )
#define HOST_CMD_H
#ifdef _WIN32
#pragma once
#endif

#include "savegame_version.h"
#include "host_saverestore.h"
#include "convar.h"

// The launcher includes this file, too
#ifndef LAUNCHERONLY



extern ConVar host_name;

//extern int  gHostSpawnCount;

#endif // LAUNCHERONLY
#endif // HOST_CMD_H
