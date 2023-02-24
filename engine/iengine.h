//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//===========================================================================//
#if !defined( IENGINE_H )
#define IENGINE_H
#ifdef _WIN32
#pragma once
#endif

#include "quakedef.h"
#include "interface.h"
#include "appframework/IAppSystem.h"

//-----------------------------------------------------------------------------
// Main engine interface to launcher + tools
//-----------------------------------------------------------------------------
#define VENGINE_VERSION "VENGINE_VERSION001"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
abstract_class IEngine : public IAppSystem
{
public:
	enum
	{
		QUIT_NOTQUITTING = 0,
		QUIT_TODESKTOP,
		QUIT_RESTART
	};

	// Engine State Flags
	enum EngineState_t
	{
		DLL_INACTIVE = 0,		// no dll
		DLL_ACTIVE,				// engine is focused
		DLL_CLOSE,				// closing down dll
		DLL_RESTART,			// engine is shutting down but will restart right away
		DLL_PAUSED,				// engine is paused, can become active from this state
	};

	virtual bool Connect(CreateInterfaceFn factory) = 0;
	virtual void Disconnect() = 0;
	virtual void* QueryInterface(const char* pInterfaceName) = 0;
	virtual InitReturnVal_t Init() = 0;
	virtual void Shutdown() = 0;

	virtual			~IEngine( void ) { }

	virtual	bool	Load( bool dedicated, const char *rootdir ) = 0;
	virtual void	Unload( void ) = 0;
	virtual void	SetNextState( EngineState_t iNextState ) = 0;
	virtual EngineState_t GetState( void ) = 0;

	virtual void	Frame( void ) = 0;

	virtual float	GetFrameTime( void ) = 0;
	virtual float	GetCurTime( void ) = 0;

	virtual int		GetQuitting( void ) = 0;
	virtual void	SetQuitting( int quittype ) = 0;
	
private:
};

void SetEngineInstance(IEngine* eng);
IEngine* GetEngineInstance();
//extern IEngine *eng;


#endif // IENGINE_H
