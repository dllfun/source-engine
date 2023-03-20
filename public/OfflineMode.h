//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include <vgui/ISystem.h>
#include <vgui_controls/Controls.h>

#define STEAM_OFFLINE_MODE "HKEY_CURRENT_USER\\Software\\Valve\\Steam\\Offline"
#define STEAM_AFS_MODE "HKEY_CURRENT_USER\\Software\\Valve\\Steam\\OfflineAFS"
#define OFFLINE_FILE "Steam\\cached\\offline_" // first part of filename

inline bool IsSteamInOfflineMode()
{
#ifndef NO_STEAM
	int offline = 0;
	vgui::system()->GetRegistryInteger( STEAM_OFFLINE_MODE, offline );
	return ( offline == 1 );
#else
	return false;
#endif
}inline bool IsSteamInAuthenticationFailSafeMode()
{
#ifndef NO_STEAM
	int offline = 0;
	vgui::system()->GetRegistryInteger( STEAM_AFS_MODE, offline );
	return ( offline == 1 );
#else
	return true;
#endif
}

inline bool IsSteamGameServerBrowsingEnabled()
{
#ifndef NO_STEAM
	return (IsSteamInAuthenticationFailSafeMode() || !IsSteamInOfflineMode());
#else
	return true;
#endif // !NO_STEAM
}
