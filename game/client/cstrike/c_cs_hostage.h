//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side CHostage class
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_CHOSTAGE_H
#define C_CHOSTAGE_H
#ifdef _WIN32
#pragma once
#endif

#include "c_ai_basenpc.h"
#include "utlvector.h"
#include "util_shared.h"
#include "cs_playeranimstate.h"
#include "c_cs_player.h"


// for shared code
#define CHostage C_CHostage

template<typename T= bool>
void RecvProxy_Rescued(const CRecvProxyData* pData, void* pStruct, void* pOut);

class RecvPropRescued : public RecvPropInt {
public:
	RecvPropRescued() {}

	template<typename T = int>
	RecvPropRescued(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,	// Handled by RECVINFO macro, but set to SIZEOF_IGNORE if you don't want to bother.
		int flags = 0,
		RecvVarProxyFn varProxy = RecvProxy_Rescued<T>
	);
	virtual	~RecvPropRescued() {}
	RecvPropRescued& operator=(const RecvPropRescued& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropRescued* pRecvProp = new RecvPropRescued;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

template<typename T>
RecvPropRescued::RecvPropRescued(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar,
	int flags,
	RecvVarProxyFn varProxy
):RecvPropInt(pType, pVarName, offset, sizeofVar, flags, varProxy)
{
	
}

//----------------------------------------------------------------------------------------------
/**
 * The client-side implementation of the Hostage
 */
class C_CHostage : public C_BaseCombatCharacter, public ICSPlayerAnimStateHelpers
{
public:
	DECLARE_CLASS( C_CHostage, C_BaseCombatCharacter );
	DECLARE_CLIENTCLASS();

	C_CHostage();
	virtual ~C_CHostage();

// ICSPlayerAnimState overrides.
public:
	virtual CWeaponCSBase* CSAnim_GetActiveWeapon();
	virtual bool CSAnim_CanMove();

public:	
	virtual void Spawn( void );
	virtual void UpdateClientSideAnimation();

	void OnPreDataChanged( DataUpdateType_t updateType );
	void OnDataChanged( DataUpdateType_t updateType );

	bool IsRescued( void ) { return m_isRescued; }
	bool WasRecentlyKilledOrRescued( void );

	int GetHealth( void ) const { return m_iHealth; }
	int GetMaxHealth( void ) const { return m_iMaxHealth; }

	virtual void ClientThink( void );

	C_CSPlayer *GetLeader( void ) const;			// return who we are following or NULL

	virtual C_BaseAnimating * BecomeRagdollOnClient();
	virtual bool ShouldDraw( void );

	void ImpactTrace( trace_t *pTrace, int iDamageType, const char *pCustomImpactName );
private:
	int  m_OldLifestate;
	CNetworkVar( int,  m_iMaxHealth);

	ICSPlayerAnimState *m_PlayerAnimState;

	CNetworkHandle(C_BaseEntity, m_leader );				// who we are following, or NULL EHANDLE

	CNetworkVar( bool, m_isRescued );
	float m_flDeadOrRescuedTime;

	CountdownTimer m_blinkTimer;

	Vector m_lookAt;		// point in space we are looking at
	void UpdateLookAt( CStudioHdr *pStudioHdr );	// orient head and eyes towards m_lookAt
	void LookAround( void );										// look around at various interesting things
	CountdownTimer m_lookAroundTimer;

	bool m_isInit;
	void Initialize( void );						// set up attachment and pose param indices

	int m_eyeAttachment;
	int m_chestAttachment;

	int m_bodyYawPoseParam;
	float m_bodyYawMin;
	float m_bodyYawMax;

	int m_headYawPoseParam;
	float m_headYawMin;
	float m_headYawMax;
	float m_flCurrentHeadYaw;
	float m_flLastBodyYaw;

	int m_headPitchPoseParam;
	float m_headPitchMin;
	float m_headPitchMax;
	float m_flCurrentHeadPitch;

	int m_seq;

	bool m_createdLowViolenceRagdoll;
	
private:
	C_CHostage( const C_CHostage & );				// not defined, not accessible

public:
	BEGIN_INIT_RECV_TABLE(C_CHostage)
	BEGIN_RECV_TABLE(C_CHostage, DT_CHostage, DT_BaseCombatCharacter)

		RecvPropRescued(RECVINFO(m_isRescued), 0),//, RecvProxy_Rescued
		RecvPropInt(RECVINFO(m_iHealth)),
		RecvPropInt(RECVINFO(m_iMaxHealth)),
		RecvPropInt(RECVINFO(m_lifeState)),

		RecvPropEHandle(RECVINFO(m_leader)),

	END_RECV_TABLE(DT_CHostage)
	END_INIT_RECV_TABLE()
};


inline C_CSPlayer *C_CHostage::GetLeader( void ) const
{
	return ToCSPlayer( m_leader );//.m_Value
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template<typename T>
void RecvProxy_Rescued(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	C_CHostage* pHostage = (C_CHostage*)pStruct;

	bool isRescued = pData->m_Value.m_Int != 0;

	if (isRescued && !pHostage->m_isRescued)
	{
		// hostage was rescued
		pHostage->m_flDeadOrRescuedTime = gpGlobals->GetCurTime() + 2;
		pHostage->SetRenderMode(kRenderGlow);
		pHostage->SetNextClientThink(gpGlobals->GetCurTime());
	}

	pHostage->m_isRescued = isRescued;
}

extern CUtlVector< C_CHostage* > g_Hostages;
extern CUtlVector< EHANDLE > g_HostageRagdolls;


#endif // C_CHOSTAGE_H
