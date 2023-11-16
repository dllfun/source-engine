//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef C_CS_PLAYER_H
#define C_CS_PLAYER_H
#ifdef _WIN32
#pragma once
#endif


#include "cs_playeranimstate.h"
#include "c_baseplayer.h"
#include "cs_shareddefs.h"
#include "weapon_csbase.h"
#include "baseparticleentity.h"
#include "beamdraw.h"
#include "history_resource.h"
#include "c_team.h"


class C_PhysicsProp;

extern ConVar cl_disablefreezecam;
extern const float CycleLatchTolerance;

class CAddonModel
{
public:
	CHandle<C_BaseAnimating> m_hEnt;	// The model for the addon.
	int m_iAddon;						// One of the ADDON_ bits telling which model this is.
	int m_iAttachmentPoint;				// Which attachment point on the player model this guy is on.
};

template<typename T= bool>
void RecvProxy_HasDefuser(const CRecvProxyData* pData, void* pStruct, void* pOut);

class RecvPropHasDefuser : public RecvPropInt {
public:
	RecvPropHasDefuser() {}

	template<typename T = int>
	RecvPropHasDefuser(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,	// Handled by RECVINFO macro, but set to SIZEOF_IGNORE if you don't want to bother.
		int flags = 0,
		RecvVarProxyFn varProxy = RecvProxy_HasDefuser<T>
	);
	virtual	~RecvPropHasDefuser() {}
	RecvPropHasDefuser& operator=(const RecvPropHasDefuser& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropHasDefuser* pRecvProp = new RecvPropHasDefuser;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

template<typename T>
RecvPropHasDefuser::RecvPropHasDefuser(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar,
	int flags,
	RecvVarProxyFn varProxy
):RecvPropInt(pType, pVarName, offset, sizeofVar, flags, varProxy)
{
	
}

template<typename T= bool>
void RecvProxy_NightVision(const CRecvProxyData* pData, void* pStruct, void* pOut);

class RecvPropNightVision : public RecvPropInt {
public:
	RecvPropNightVision() {}

	template<typename T = int>
	RecvPropNightVision(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,	// Handled by RECVINFO macro, but set to SIZEOF_IGNORE if you don't want to bother.
		int flags = 0,
		RecvVarProxyFn varProxy = RecvProxy_NightVision<T>
	);
	virtual	~RecvPropNightVision() {}
	RecvPropNightVision& operator=(const RecvPropNightVision& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropNightVision* pRecvProp = new RecvPropNightVision;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

template<typename T>
RecvPropNightVision::RecvPropNightVision(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar,
	int flags,
	RecvVarProxyFn varProxy
) :RecvPropInt(pType, pVarName, offset, sizeofVar, flags, varProxy)
{

}

template<typename T= float>
void RecvProxy_FlashTime(const CRecvProxyData* pData, void* pStruct, void* pOut);

class RecvPropFlashTime : public RecvPropFloat {
public:
	RecvPropFlashTime() {}

	template<typename T = float>
	RecvPropFlashTime(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,	// Handled by RECVINFO macro, but set to SIZEOF_IGNORE if you don't want to bother.
		int flags = 0,
		RecvVarProxyFn varProxy = RecvProxy_FlashTime<T>
	);
	virtual	~RecvPropFlashTime() {}
	RecvPropFlashTime& operator=(const RecvPropFlashTime& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropFlashTime* pRecvProp = new RecvPropFlashTime;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

template<typename T>
RecvPropFlashTime::RecvPropFlashTime(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar,
	int flags,
	RecvVarProxyFn varProxy
):RecvPropFloat(pType, pVarName, offset, sizeofVar, flags, varProxy)
{
	
}

// Having the RecvProxy in the player allows us to keep the var private
template<typename T= float>
void RecvProxy_CycleLatch(const CRecvProxyData* pData, void* pStruct, void* pOut);

class RecvPropCycleLatch : public RecvPropInt {
public:
	RecvPropCycleLatch() {}

	template<typename T = int>
	RecvPropCycleLatch(
		T* pType,
		const char* pVarName,
		int offset,
		int sizeofVar = SIZEOF_IGNORE,	// Handled by RECVINFO macro, but set to SIZEOF_IGNORE if you don't want to bother.
		int flags = 0,
		RecvVarProxyFn varProxy = RecvProxy_CycleLatch<T>
	);
	virtual	~RecvPropCycleLatch() {}
	RecvPropCycleLatch& operator=(const RecvPropCycleLatch& srcSendProp) {
		RecvProp::operator=(srcSendProp);
		return *this;
	}
	operator RecvProp* () {
		RecvPropCycleLatch* pRecvProp = new RecvPropCycleLatch;
		*pRecvProp = *this;
		return pRecvProp;
	}
};

template<typename T>
RecvPropCycleLatch::RecvPropCycleLatch(
	T* pType,
	const char* pVarName,
	int offset,
	int sizeofVar,
	int flags,
	RecvVarProxyFn varProxy
) :RecvPropInt(pType, pVarName, offset, sizeofVar, flags, varProxy)
{

}


class C_CSPlayer : public C_BasePlayer, public ICSPlayerAnimStateHelpers
{
public:
	DECLARE_CLASS( C_CSPlayer, C_BasePlayer );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();

	C_CSPlayer();
	~C_CSPlayer();

	virtual void Simulate();

	bool HasDefuser() const;

	void GiveDefuser();
	void RemoveDefuser();

	bool HasNightVision() const;

	static C_CSPlayer* GetLocalCSPlayer();
	CSPlayerState State_Get() const;

	virtual float GetMinFOV() const;

	// Get how much $$$ this guy has.
	int GetAccount() const;

	// Returns one of the CS_CLASS_ enums.
	int PlayerClass() const;

	bool IsInBuyZone();
	bool CanShowTeamMenu() const;	// Returns true if we're allowed to show the team menu right now.

	// Get the amount of armor the player has.
	int ArmorValue() const;
	bool HasHelmet() const;
	int GetCurrentAssaultSuitPrice();

	virtual const QAngle& EyeAngles();
	virtual const QAngle& GetRenderAngles();
	virtual void CalcObserverView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov );

	virtual void			GetRenderBounds( Vector& theMins, Vector& theMaxs );
	virtual void			GetShadowRenderBounds( Vector &mins, Vector &maxs, ShadowType_t shadowType );
	virtual bool			GetShadowCastDirection( Vector *pDirection, ShadowType_t shadowType ) const;

	virtual void VPhysicsUpdate( IPhysicsObject *pPhysics );

	// Get the ID target entity index. The ID target is the player that is behind our crosshairs, used to
	// display the player's name.
	int GetIDTarget() const;

	virtual void NotifyShouldTransmit( ShouldTransmitState_t state );
	virtual void ClientThink();

	virtual void OnDataChanged( DataUpdateType_t type );
	virtual void PostDataUpdate( DataUpdateType_t updateType );
	virtual bool Interpolate( float currentTime );
	virtual void UpdateStepSound( surfacedata_t *psurface, const Vector &vecOrigin, const Vector &vecVelocity  );
	virtual surfacedata_t * GetFootstepSurface( const Vector &origin, const char *surfaceName );
	virtual void ValidateModelIndex( void );

	virtual int	GetMaxHealth() const;

	bool		Weapon_CanSwitchTo(C_BaseCombatWeapon *pWeapon);

	virtual void UpdateClientSideAnimation();
	virtual void ProcessMuzzleFlashEvent();

	virtual const Vector& GetRenderOrigin( void );

	bool CreateMove( float flInputSampleTime, CUserCmd *pCmd );

	CUtlVector< C_BaseParticleEntity* > m_SmokeGrenades;

	virtual bool ShouldDraw( void );
	virtual void BuildTransformations( CStudioHdr *pStudioHdr, Vector *pos, Quaternion q[], const matrix3x4_t& cameraTransform, int boneMask, CBoneBitList &boneComputed );

	virtual C_BaseAnimating * BecomeRagdollOnClient();
	virtual IRagdoll* GetRepresentativeRagdoll() const;

	void ImpactTrace( trace_t *pTrace, int iDamageType, const char *pCustomImpactName );

	// Have this player play the sounds from his view model's reload animation.
	void PlayReloadEffect();

	virtual void FireEvent( const Vector& origin, const QAngle& angles, int event, const char *options );

	bool		 HasC4( void );

	virtual void CreateLightEffects( void ) {}	//no dimlight effects

	// Sometimes the server wants to update the client's cycle to get the two to run in sync (for proper hit detection)
	virtual void SetServerIntendedCycle( float intended ) { m_serverIntendedCycle = intended; }
	virtual float GetServerIntendedCycle( void ) { return m_serverIntendedCycle; }

	virtual bool ShouldReceiveProjectedTextures( int flags )
	{
		return ( this != C_BasePlayer::GetLocalPlayer() );
	}

	void ClearSoundEvents()
	{
		m_SoundEvents.RemoveAll();
	}

	//=============================================================================
	// HPE_BEGIN:
	// [menglish] Returns whether this player is dominating or is being dominated by the specified player
	//=============================================================================
	bool IsPlayerDominated( int iPlayerIndex );
	bool IsPlayerDominatingMe( int iPlayerIndex );

	virtual void CalcFreezeCamView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov );

	virtual float GetDeathCamInterpolationTime();
	//=============================================================================
	// HPE_END
	//=============================================================================


// Called by shared code.
public:

	// ICSPlayerAnimState overrides.
	virtual CWeaponCSBase* CSAnim_GetActiveWeapon();
	virtual bool CSAnim_CanMove();


	void DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 );


// Implemented in shared code.
public:
	virtual float GetPlayerMaxSpeed();

	void GetBulletTypeParameters(
		int iBulletType,
		float &fPenetrationPower,
		float &flPenetrationDistance );

	void FireBullet(
		Vector vecSrc,
		const QAngle &shootAngles,
		float flDistance,
		int iPenetration,
		int iBulletType,
		int iDamage,
		float flRangeModifier,
		CBaseEntity *pevAttacker,
		bool bDoEffects,
		float xSpread, float ySpread );

	void KickBack(
		float up_base,
		float lateral_base,
		float up_modifier,
		float lateral_modifier,
		float up_max,
		float lateral_max,
		int direction_change );

	// Returns true if the player is allowed to move.
	bool CanMove() const;

	void OnJump( float fImpulse );
	void OnLand( float fVelocity );

	bool HasC4() const;	// Is this player carrying a C4 bomb?
	bool IsVIP() const;	// Is this player the VIP?

	virtual void SetAnimation( PLAYER_ANIM playerAnim );


public:

	void UpdateIDTarget( void );
	void RemoveAddonModels( void );
	void UpdateMinModels( void );

	void SetActivity( Activity eActivity );
	Activity GetActivity( void ) const;

	ICSPlayerAnimState *GetPlayerAnimState() { return m_PlayerAnimState; }

public:

	ICSPlayerAnimState *m_PlayerAnimState;

	// Used to control animation state.
	Activity m_Activity;

	// Predicted variables.
	CNetworkVar( bool, m_bResumeZoom );
	CNetworkVar( int , m_iLastZoom ); // after firing a shot, set the FOV to 90, and after showing the animation, bring the FOV back to last zoom level.
	CNetworkVar( CSPlayerState, m_iPlayerState );	// SupraFiend: this gives the current state in the joining process, the states are listed above
	CNetworkVar( bool, m_bIsDefusing );			// tracks whether this player is currently defusing a bomb
	CNetworkVar( bool, m_bInBombZone );
	CNetworkVar( bool, m_bInBuyZone );
	CNetworkVar( int, m_iThrowGrenadeCounter );	// used to trigger grenade throw animations.

	bool IsInHostageRescueZone( void );

	// This is a combination of the ADDON_ flags in cs_shareddefs.h.
	CNetworkVar( int, m_iAddonBits );

	// Clients don't know about holstered weapons, so we need to be told about them here
	CNetworkVar( int, m_iPrimaryAddon );
	CNetworkVar( int, m_iSecondaryAddon );

	// How long the progress bar takes to get to the end. If this is 0, then the progress bar
	// should not be drawn.
	CNetworkVar( int, m_iProgressBarDuration );

	// When the progress bar should start.
	CNetworkVar( float, m_flProgressBarStartTime );

	CNetworkVar( float, m_flStamina );
	CNetworkVar( int, m_iDirection );	// The current lateral kicking direction; 1 = right,  0 = left
	CNetworkVar( int, m_iShotsFired );	// number of shots fired recently
	CNetworkVar( bool, m_bNightVisionOn );
	CNetworkVar( bool, m_bHasNightVision );

    //=============================================================================
    // HPE_BEGIN:
    // [dwenger] Added for fun-fact support
    //=============================================================================

    //CNetworkVar( bool, m_bPickedUpDefuser );
    //CNetworkVar( bool, m_bDefusedWithPickedUpKit );

    //=============================================================================
    // HPE_END
    //=============================================================================

    CNetworkVar( float, m_flVelocityModifier );

	bool		m_bDetected;

	CNetworkHandle(C_BaseEntity,	m_hRagdoll);

	CWeaponCSBase* GetActiveCSWeapon() const;
	CWeaponCSBase* GetCSWeapon( CSWeaponID id ) const;

	virtual ShadowType_t		ShadowCastType();

#ifdef CS_SHIELD_ENABLED
	bool HasShield( void ) { return m_bHasShield; }
	bool IsShieldDrawn( void ) { return m_bShieldDrawn;	}
	void SetShieldDrawnState( bool bState ) { m_bShieldDrawn = bState; }
#else
	bool HasShield( void ) { return false; }
	bool IsShieldDrawn( void ) { return false; }
	void SetShieldDrawnState( bool bState ) {}
#endif

	float m_flNightVisionAlpha;

	float m_flFlashAlpha;
	float m_flFlashBangTime;
	CNetworkVar( float, m_flFlashMaxAlpha );
	CNetworkVar( float, m_flFlashDuration );

	

	// Bots and hostages auto-duck during jumps
	bool m_duckUntilOnGround;

	Vector m_lastStandingPos; // used by the gamemovement code for finding ladders

	void SurpressLadderChecks( const Vector& pos, const Vector& normal );
	bool CanGrabLadder( const Vector& pos, const Vector& normal );

//=============================================================================
// HPE_BEGIN:
//=============================================================================

// [tj] checks if this player has another given player on their Steam friends list.
	bool HasPlayerAsFriend(C_CSPlayer* player);

private:
	CountdownTimer m_ladderSurpressionTimer;
	Vector m_lastLadderNormal;
	Vector m_lastLadderPos;

	void UpdateRadar();
	void UpdateSoundEvents();

	void CreateAddonModel( int i );
	void UpdateAddonModels();

	void PushawayThink();

	CNetworkVar( int,		m_iAccount);
	CNetworkVar( bool,	m_bHasHelmet);
	CNetworkVar( int,		m_iClass);
	CNetworkVar( int,		m_ArmorValue);
	CNetworkQAngle(	m_angEyeAngles);
	CNetworkVar( bool,	m_bHasDefuser);
	CNetworkVar( bool,	m_bInHostageRescueZone);
	float	m_fNextThinkPushAway;

    bool    m_bPlayingFreezeCamSound;

#ifdef CS_SHIELD_ENABLED
	bool	m_bHasShield;
	bool	m_bShieldDrawn;
#endif

	CNetworkVector( m_vecRagdollVelocity);

	CInterpolatedVar< QAngle >	m_iv_angEyeAngles;

	// ID Target
	int					m_iIDEntIndex;
	CountdownTimer		m_delayTargetIDTimer;

	// Show the ID target after the cursor leaves the entity
	int					m_iOldIDEntIndex;
	CountdownTimer		m_holdTargetIDTimer;

	void ReleaseFlashlight( void );
	Beam_t	*m_pFlashlightBeam;

	class CCSSoundEvent
	{
	public:
		string_t m_SoundName;
		float m_flEventTime;	// Play the event when gpGlobals->GetCurTime() goes past this.
	};
	CUtlLinkedList<CCSSoundEvent,int> m_SoundEvents;


	// This is the list of addons hanging off the guy (grenades, C4, nightvision, etc).
	CUtlLinkedList<CAddonModel, int> m_AddonModels;
	int m_iLastAddonBits;
	int m_iLastPrimaryAddon;
	int m_iLastSecondaryAddon;

	CNetworkVar( int, m_cycleLatch);				// server periodically updates this to fix up our anims, here it is a 4 bit fixed point
	float m_serverIntendedCycle;	// server periodically updates this to fix up our anims, here it is the float we want, or -1 for no override



    //=============================================================================
    // HPE_BEGIN:
    // [tj] Network variables that track who are dominating and being dominated by
    //=============================================================================

    CNetworkArray( bool, m_bPlayerDominated, MAX_PLAYERS+1 );		// array of state per other player whether player is dominating other players
    CNetworkArray( bool, m_bPlayerDominatingMe, MAX_PLAYERS+1 );	// array of state per other player whether other players are dominating this player

    //=============================================================================
    // HPE_END
    //=============================================================================



	C_CSPlayer( const C_CSPlayer & );

public:
	BEGIN_INIT_RECV_TABLE(C_CSPlayer)
	BEGIN_RECV_TABLE_NOBASE(C_CSPlayer, DT_CSLocalPlayerExclusive)
		RecvPropFloat(RECVINFO(m_flStamina)),
		RecvPropInt(RECVINFO(m_iDirection)),
		RecvPropInt(RECVINFO(m_iShotsFired)),
		RecvPropFloat(RECVINFO(m_flVelocityModifier)),

		RecvPropVector(RECVINFO_NAME(m_vecNetworkOrigin, m_vecOrigin)),

		//=============================================================================
		// HPE_BEGIN:
		// [tj]Set up the receive table for per-client domination data
		//=============================================================================

		RecvPropArray3(RECVINFO_ARRAY(m_bPlayerDominated), RecvPropBool(RECVINFO_ARRAY3(m_bPlayerDominated))),
		RecvPropArray3(RECVINFO_ARRAY(m_bPlayerDominatingMe), RecvPropBool(RECVINFO_ARRAY3(m_bPlayerDominatingMe)))

		//=============================================================================
		// HPE_END
		//=============================================================================

	END_RECV_TABLE(DT_CSLocalPlayerExclusive)

	BEGIN_RECV_TABLE_NOBASE(C_CSPlayer, DT_CSNonLocalPlayerExclusive)
		RecvPropVector(RECVINFO_NAME(m_vecNetworkOrigin, m_vecOrigin)),
	END_RECV_TABLE(DT_CSNonLocalPlayerExclusive)

	BEGIN_RECV_TABLE(C_CSPlayer, DT_CSPlayer, DT_BasePlayer)
		RecvPropDataTable("cslocaldata", 0, 0, REFERENCE_RECV_TABLE(DT_CSLocalPlayerExclusive)),
		RecvPropDataTable("csnonlocaldata", 0, 0, REFERENCE_RECV_TABLE(DT_CSNonLocalPlayerExclusive)),
		RecvPropInt(RECVINFO(m_iAddonBits)),
		RecvPropInt(RECVINFO(m_iPrimaryAddon)),
		RecvPropInt(RECVINFO(m_iSecondaryAddon)),
		RecvPropInt(RECVINFO(m_iThrowGrenadeCounter)),
		RecvPropInt(RECVINFO(m_iPlayerState)),
		RecvPropInt(RECVINFO(m_iAccount)),
		RecvPropInt(RECVINFO(m_bInBombZone)),
		RecvPropInt(RECVINFO(m_bInBuyZone)),
		RecvPropInt(RECVINFO(m_iClass)),
		RecvPropInt(RECVINFO(m_ArmorValue)),
		RecvPropFloat(RECVINFO_VECTORELEM(m_angEyeAngles,0)),
		RecvPropFloat(RECVINFO_VECTORELEM(m_angEyeAngles,1)),
		RecvPropFloat(RECVINFO(m_flStamina)),
		RecvPropHasDefuser(RECVINFO(m_bHasDefuser), 0),//, RecvProxy_HasDefuser
		RecvPropNightVision(RECVINFO(m_bNightVisionOn), 0),//, RecvProxy_NightVision
		RecvPropBool(RECVINFO(m_bHasNightVision)),


		//=============================================================================
		// HPE_BEGIN:
		// [dwenger] Added for fun-fact support
		//=============================================================================

		//RecvPropBool( RECVINFO( m_bPickedUpDefuser ) ),
		//RecvPropBool( RECVINFO( m_bDefusedWithPickedUpKit ) ),

		//=============================================================================
		// HPE_END
		//=============================================================================

		RecvPropBool(RECVINFO(m_bInHostageRescueZone)),
		RecvPropInt(RECVINFO(m_ArmorValue)),
		RecvPropBool(RECVINFO(m_bIsDefusing)),
		RecvPropBool(RECVINFO(m_bResumeZoom)),
		RecvPropInt(RECVINFO(m_iLastZoom)),

#ifdef CS_SHIELD_ENABLED
		RecvPropBool(RECVINFO(m_bHasShield)),
		RecvPropBool(RECVINFO(m_bShieldDrawn)),
#endif
		RecvPropInt(RECVINFO(m_bHasHelmet)),
		RecvPropVector(RECVINFO(m_vecRagdollVelocity)),
		RecvPropFlashTime(RECVINFO(m_flFlashDuration), 0),//, RecvProxy_FlashTime
		RecvPropFloat(RECVINFO(m_flFlashMaxAlpha)),
		RecvPropInt(RECVINFO(m_iProgressBarDuration)),
		RecvPropFloat(RECVINFO(m_flProgressBarStartTime)),
		RecvPropEHandle(RECVINFO(m_hRagdoll)),
		RecvPropCycleLatch(RECVINFO(m_cycleLatch), 0),//, RecvProxy_CycleLatch

	END_RECV_TABLE(DT_CSPlayer)
	END_INIT_RECV_TABLE()
};

C_CSPlayer* GetLocalOrInEyeCSPlayer( void );

inline C_CSPlayer *ToCSPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

	return dynamic_cast<C_CSPlayer*>( pEntity );
}

namespace vgui
{
	class IImage;
}

vgui::IImage* GetDefaultAvatarImage( C_BasePlayer *pPlayer );

template<typename T>
void RecvProxy_HasDefuser(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	C_CSPlayer* pPlayerData = (C_CSPlayer*)pStruct;

	if (pPlayerData == NULL)
	{
		return;
	}

	bool drawIcon = false;

	if (pData->m_Value.m_Int == 0)
	{
		pPlayerData->RemoveDefuser();
	}
	else
	{
		if (pPlayerData->HasDefuser() == false)
		{
			drawIcon = true;
		}
		pPlayerData->GiveDefuser();
	}

	if (pPlayerData->IsLocalPlayer() && drawIcon)
	{
		// add to pickup history
		CHudHistoryResource* pHudHR = GET_HUDELEMENT(CHudHistoryResource);

		if (pHudHR)
		{
			pHudHR->AddToHistory(HISTSLOT_ITEM, "defuser_pickup");
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Called when the player toggles nightvision
// Input  : *pData - the int value of the nightvision state
//			*pStruct - the player
//			*pOut -
//-----------------------------------------------------------------------------
template<typename T>
void RecvProxy_NightVision(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	C_CSPlayer* pPlayerData = (C_CSPlayer*)pStruct;

	bool bNightVisionOn = (pData->m_Value.m_Int > 0);

	if (pPlayerData->m_bNightVisionOn != bNightVisionOn)
	{
		if (bNightVisionOn)
			pPlayerData->m_flNightVisionAlpha = 1;
	}

	pPlayerData->m_bNightVisionOn = bNightVisionOn;
}

template<typename>
void RecvProxy_FlashTime(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	C_CSPlayer* pPlayerData = (C_CSPlayer*)pStruct;

	if (pPlayerData != C_BasePlayer::GetLocalPlayer())
		return;

	if ((pPlayerData->m_flFlashDuration != pData->m_Value.m_Float) && pData->m_Value.m_Float > 0)
	{
		pPlayerData->m_flFlashAlpha = 1;
	}

	pPlayerData->m_flFlashDuration = pData->m_Value.m_Float;
	pPlayerData->m_flFlashBangTime = gpGlobals->GetCurTime() + pPlayerData->m_flFlashDuration;
}

template<typename T>
void RecvProxy_CycleLatch(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	// This receive proxy looks to see if the server's value is close enough to what we think it should
	// be.  We've been running the same code; this is an error correction for changes we didn't simulate
	// while they were out of PVS.
	C_CSPlayer* pPlayer = (C_CSPlayer*)pStruct;
	if (pPlayer->IsLocalPlayer())
		return; // Don't need to fixup ourselves.

	float incomingCycle = (float)(pData->m_Value.m_Int) / 16; // Came in as 4 bit fixed point
	float currentCycle = pPlayer->GetCycle();
	bool closeEnough = fabs(currentCycle - incomingCycle) < CycleLatchTolerance;
	if (fabs(currentCycle - incomingCycle) > (1 - CycleLatchTolerance))
	{
		closeEnough = true;// Handle wrapping around 1->0
	}

	if (!closeEnough)
	{
		// Server disagrees too greatly.  Correct our value.
		if (pPlayer && pPlayer->GetTeam())
		{
			DevMsg(2, "%s %s(%d): Cycle latch wants to correct %.2f in to %.2f.\n",
				pPlayer->GetTeam()->Get_Name(), pPlayer->GetPlayerName(), pPlayer->entindex(), currentCycle, incomingCycle);
		}
		pPlayer->SetServerIntendedCycle(incomingCycle);
	}
}

#endif // C_CS_PLAYER_H
