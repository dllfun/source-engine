//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_SCENEENTITY_H
#define C_SCENEENTITY_H
#ifdef _WIN32
#pragma once
#endif

#include "ichoreoeventcallback.h"
#include "choreoscene.h"
#include "c_baseentity.h"

template<typename T= float>
void RecvProxy_ForcedClientTime(const CRecvProxyData* pData, void* pStruct, void* pOut);

class RecvPropForcedClientTime : public RecvPropFloat {
public:
	RecvPropForcedClientTime() {}

	template<typename T = float>
	RecvPropForcedClientTime(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,	// Handled by RECVINFO macro, but set to SIZEOF_IGNORE if you don't want to bother.
		int flags = 0,
		RecvVarProxyFn varProxy = RecvProxy_ForcedClientTime<T>
	);
	virtual	~RecvPropForcedClientTime() {}
	RecvPropForcedClientTime& operator=(const RecvPropForcedClientTime& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropForcedClientTime* pRecvProp = new RecvPropForcedClientTime;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

template<typename T>
RecvPropForcedClientTime::RecvPropForcedClientTime(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar,
	int flags,
	RecvVarProxyFn varProxy
):RecvPropFloat(pType, pVarName, offset, sizeofVar, flags, varProxy)
{
	
}

class C_SceneEntity : public C_BaseEntity, public IChoreoEventCallback
{
	friend class CChoreoEventCallback;

public:
	DECLARE_CLASS( C_SceneEntity, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	C_SceneEntity( void );
	~C_SceneEntity( void );

	// From IChoreoEventCallback
	virtual void			StartEvent( float currenttime, CChoreoScene *scene, CChoreoEvent *event );
	virtual void			EndEvent( float currenttime, CChoreoScene *scene, CChoreoEvent *event );
	virtual void			ProcessEvent( float currenttime, CChoreoScene *scene, CChoreoEvent *event );
	virtual bool			CheckEvent( float currenttime, CChoreoScene *scene, CChoreoEvent *event );


	virtual void PostDataUpdate( DataUpdateType_t updateType );
	virtual void PreDataUpdate( DataUpdateType_t updateType );

	virtual void StopClientOnlyScene();
	virtual void SetupClientOnlyScene( const char *pszFilename, C_BaseFlex *pOwner = NULL , bool bMultiplayer = false );

	virtual void ClientThink();

	void					OnResetClientTime();

	CHandle< C_BaseFlex >	GetActor( int i ){ return ( i < m_hActorList.Count() ) ? m_hActorList[i] : NULL; }

	virtual	void			DispatchStartSpeak( CChoreoScene *scene, C_BaseFlex *actor, CChoreoEvent *event, soundlevel_t iSoundlevel );
	virtual void			DispatchEndSpeak( CChoreoScene *scene, C_BaseFlex *actor, CChoreoEvent *event );

	bool IsClientOnly( void ){ return m_bClientOnly; }

private:

	void					ResetActorFlexesForScene();

	// Scene load/unload
	CChoreoScene			*LoadScene( const char *filename );
	void					LoadSceneFromFile( const char *filename );
	void					UnloadScene( void );
	void					PrefetchAnimBlocks( CChoreoScene *pScene );

	C_BaseFlex				*FindNamedActor( CChoreoActor *pChoreoActor );

	virtual void			DispatchStartFlexAnimation( CChoreoScene *scene, C_BaseFlex *actor, CChoreoEvent *event );
	virtual void			DispatchEndFlexAnimation( CChoreoScene *scene, C_BaseFlex *actor, CChoreoEvent *event );
	virtual void			DispatchStartExpression( CChoreoScene *scene, C_BaseFlex *actor, CChoreoEvent *event );
	virtual void			DispatchEndExpression( CChoreoScene *scene, C_BaseFlex *actor, CChoreoEvent *event );
	virtual void			DispatchStartGesture( CChoreoScene *scene, C_BaseFlex *actor, CChoreoEvent *event );
	virtual void			DispatchProcessGesture( CChoreoScene *scene, C_BaseFlex *actor, CChoreoEvent *event );
	virtual void			DispatchEndGesture( CChoreoScene *scene, C_BaseFlex *actor, CChoreoEvent *event );
	virtual void			DispatchStartSequence( CChoreoScene *scene, C_BaseFlex *actor, CChoreoEvent *event );
	virtual void			DispatchProcessSequence( CChoreoScene *scene, C_BaseFlex *actor, CChoreoEvent *event );
	virtual void			DispatchEndSequence( CChoreoScene *scene, C_BaseFlex *actor, CChoreoEvent *event );
	void					DispatchProcessLoop( CChoreoScene *scene, CChoreoEvent *event );

	char const				*GetSceneFileName();

	void					DoThink( float frametime );

	void					ClearSceneEvents( CChoreoScene *scene, bool canceled );
	void					SetCurrentTime( float t, bool forceClientSync );

	bool					GetHWMorphSceneFileName( const char *pFilename, char *pHWMFilename );

private:

	void					CheckQueuedEvents();
	void					WipeQueuedEvents();
	void					QueueStartEvent( float starttime, CChoreoScene *scene, CChoreoEvent *event );

	CNetworkVar( bool,		m_bIsPlayingBack);
	CNetworkVar( bool,		m_bPaused);
	CNetworkVar( bool,		m_bMultiplayer);
	float		m_flCurrentTime;
	CNetworkVar( float,		m_flForceClientTime);
	CNetworkVar( int,			m_nSceneStringIndex);
	bool		m_bClientOnly;

	CHandle< C_BaseFlex >	m_hOwner; // if set, this overrides the m_hActorList in FindNamedActor()

	CUtlVector< CHandle< C_BaseFlex > > m_hActorList;		

private:
	bool		m_bWasPlaying;

	CChoreoScene *m_pScene;

	struct QueuedEvents_t
	{
		float			starttime;
		CChoreoScene	*scene;
		CChoreoEvent	*event;
	};

	CUtlVector< QueuedEvents_t > m_QueuedEvents;

public:
	BEGIN_INIT_RECV_TABLE(C_SceneEntity)
	BEGIN_RECV_TABLE(C_SceneEntity, DT_SceneEntity, DT_BaseEntity)
		RecvPropInt(RECVINFO(m_nSceneStringIndex)),
		RecvPropBool(RECVINFO(m_bIsPlayingBack)),
		RecvPropBool(RECVINFO(m_bPaused)),
		RecvPropBool(RECVINFO(m_bMultiplayer)),
		RecvPropForcedClientTime(RECVINFO(m_flForceClientTime), 0),//, RecvProxy_ForcedClientTime
		RecvPropUtlVector(
			RECVINFO_UTLVECTOR(m_hActorList),
			MAX_ACTORS_IN_SCENE,
			RecvPropEHandle((CHandle< C_BaseFlex >*)0, NULL, 0, 0)),
	END_RECV_TABLE(DT_SceneEntity)
	END_INIT_RECV_TABLE()
};

//-----------------------------------------------------------------------------
// Purpose: Decodes animtime and notes when it changes
// Input  : *pStruct - ( C_BaseEntity * ) used to flag animtime is changine
//			*pVarData - 
//			*pIn - 
//			objectID - 
//-----------------------------------------------------------------------------
template<typename T>
void RecvProxy_ForcedClientTime(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	C_SceneEntity* pScene = reinterpret_cast<C_SceneEntity*>(pStruct);
	*(T*)pOut = pData->m_Value.m_Float;//float
	pScene->OnResetClientTime();
}

//-----------------------------------------------------------------------------
// Binary compiled VCDs get their strings from a pool
//-----------------------------------------------------------------------------
class CChoreoStringPool : public IChoreoStringPool
{
public:
	short FindOrAddString( const char *pString )
	{
		// huh?, no compilation at run time, only fetches
		Assert( 0 );
		return -1;
	}

	bool GetString( short stringId, char *buff, int buffSize );
};

#endif // C_SCENEENTITY_H
