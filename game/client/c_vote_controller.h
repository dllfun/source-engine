//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Client VoteController
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_VoteController_H
#define C_VoteController_H
#ifdef _WIN32
#pragma once
#endif

#include "shareddefs.h"
#include "GameEventListener.h"

template<typename T= int>
void		RecvProxy_VoteType(const CRecvProxyData* pData, void* pStruct, void* pOut);

class RecvPropVoteType : public RecvPropInt {
public:
	RecvPropVoteType() {}

	template<typename T = int>
	RecvPropVoteType(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,	// Handled by RECVINFO macro, but set to SIZEOF_IGNORE if you don't want to bother.
		int flags = 0,
		RecvVarProxyFn varProxy = RecvProxy_VoteType<T>
	);
	virtual	~RecvPropVoteType() {}
	RecvPropVoteType& operator=(const RecvPropVoteType& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropVoteType* pRecvProp = new RecvPropVoteType;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

template<typename T>
RecvPropVoteType::RecvPropVoteType(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar,
	int flags,
	RecvVarProxyFn varProxy
):RecvPropInt(pType, pVarName, offset, sizeofVar, flags, varProxy)
{
	
}

template<typename T= int>
void		RecvProxy_VoteOption(const CRecvProxyData* pData, void* pStruct, void* pOut);

class RecvPropVoteOption : public RecvPropInt {
public:
	RecvPropVoteOption() {}

	template<typename T = int>
	RecvPropVoteOption(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,	// Handled by RECVINFO macro, but set to SIZEOF_IGNORE if you don't want to bother.
		int flags = 0,
		RecvVarProxyFn varProxy = RecvProxy_VoteOption<T>
	);
	virtual	~RecvPropVoteOption() {}
	RecvPropVoteOption& operator=(const RecvPropVoteOption& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropVoteOption* pRecvProp = new RecvPropVoteOption;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

template<typename T>
RecvPropVoteOption::RecvPropVoteOption(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar,
	int flags,
	RecvVarProxyFn varProxy
) :RecvPropInt(pType, pVarName, offset, sizeofVar, flags, varProxy)
{

}

class C_VoteController : public C_BaseEntity, public CGameEventListener
{
	DECLARE_CLASS( C_VoteController, C_BaseEntity );
public:
	DECLARE_CLIENTCLASS();

	C_VoteController();
	virtual ~C_VoteController();

	virtual void	Spawn( void );
	virtual void	ClientThink( void );

	template<typename T>
	friend void RecvProxy_VoteType(const CRecvProxyData* pData, void* pStruct, void* pOut);
	template<typename T>
	friend void RecvProxy_VoteOption(const CRecvProxyData* pData, void* pStruct, void* pOut);

	void			FireGameEvent( IGameEvent *event );
protected:
	void			ResetData();

	CNetworkVar( int,				m_iActiveIssueIndex);
	CNetworkVar( int,				m_iOnlyTeamToVote);
	CNetworkArray( int,				m_nVoteOptionCount,MAX_VOTE_OPTIONS);
	int				m_iVoteChoiceIndex;
	CNetworkVar( int,				m_nPotentialVotes);
	bool			m_bVotesDirty;	// Received a vote, so remember to tell the Hud
	bool			m_bTypeDirty;	// Vote type changed, so show or hide the Hud
	CNetworkVar( bool,			m_bIsYesNoVote);

public:
	BEGIN_INIT_RECV_TABLE(C_VoteController)
	BEGIN_RECV_TABLE(C_VoteController, DT_VoteController, DT_BaseEntity)
		RecvPropVoteType(RECVINFO(m_iActiveIssueIndex), 0),//, C_VoteController::RecvProxy_VoteType
		RecvPropInt(RECVINFO(m_iOnlyTeamToVote)),
		RecvPropArray3(RECVINFO_ARRAY(m_nVoteOptionCount), RecvPropVoteOption(RECVINFO_ARRAY3(m_nVoteOptionCount), 0)),//, C_VoteController::RecvProxy_VoteOption
		RecvPropInt(RECVINFO(m_nPotentialVotes)),
		RecvPropBool(RECVINFO(m_bIsYesNoVote))
	END_RECV_TABLE(DT_VoteController)
	END_INIT_RECV_TABLE()
};


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
template<typename T>
void RecvProxy_VoteType(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	C_VoteController* pMe = (C_VoteController*)pStruct;
	if (memcmp(&pMe->m_iActiveIssueIndex, &pData->m_Value.m_Int, sizeof(pData->m_Value.m_Int)) == 0)
		return;

	memcpy(&pMe->m_iActiveIssueIndex, &pData->m_Value.m_Int, sizeof(pData->m_Value.m_Int));
	pMe->m_bTypeDirty = true;

	// Since the contents of a new vote are in three parts, we can't directly send an event to the Hud
	// because we don't really know if we have all three parts yet.  So we'll mark dirty, and our think
	// can notice that and send the event.
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
template<typename T>
void RecvProxy_VoteOption(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	int index = pData->m_pRecvProp->GetOffset() / sizeof(int);

	size_t offset = offsetof(C_VoteController, m_nVoteOptionCount);
	C_VoteController* pMe = (C_VoteController*)((byte*)pStruct - offset);
	if (pMe->m_nVoteOptionCount[index] == pData->m_Value.m_Int)
		return;

	pMe->m_nVoteOptionCount[index] = pData->m_Value.m_Int;
	pMe->m_bVotesDirty = true;
	pMe->SetNextClientThink(gpGlobals->GetCurTime() + 0.001);
}

#endif // C_VoteController_H
