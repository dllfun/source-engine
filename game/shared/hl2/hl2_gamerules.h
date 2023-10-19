//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Game rules for Half-Life 2.
//
//=============================================================================//

#ifndef HL2_GAMERULES_H
#define HL2_GAMERULES_H
#ifdef _WIN32
#pragma once
#endif

#include "gamerules.h"
#include "singleplay_gamerules.h"
#include "hl2_shareddefs.h"

#ifdef CLIENT_DLL
	#define CHalfLife2 C_HalfLife2
	#define CHalfLife2Proxy C_HalfLife2Proxy
#endif

#ifdef CLIENT_DLL
void RecvProxy_HL2GameRules(const RecvProp* pProp, void** pOut, void* pData, int objectID);
#else
void* SendProxy_HL2GameRules(const SendProp* pProp, const void* pStructBase, const void* pData, CSendProxyRecipients* pRecipients, int objectID);
#endif

class CHalfLife2Proxy : public CGameRulesProxy
{
public:
	DECLARE_CLASS( CHalfLife2Proxy, CGameRulesProxy );
	DECLARE_NETWORKCLASS();

public:
#ifdef GAME_DLL
	BEGIN_INIT_SEND_TABLE(CHalfLife2Proxy)
	BEGIN_SEND_TABLE(CHalfLife2Proxy, DT_HalfLife2Proxy, DT_GameRulesProxy)
		SendPropDataTable("hl2_gamerules_data", 0, REFERENCE_SEND_TABLE(DT_HL2GameRules), SendProxy_HL2GameRules)
	END_SEND_TABLE()
	END_INIT_SEND_TABLE()
#endif

#ifdef CLIENT_DLL
	BEGIN_INIT_RECV_TABLE(CHalfLife2Proxy)
	BEGIN_RECV_TABLE(CHalfLife2Proxy, DT_HalfLife2Proxy, DT_GameRulesProxy)
		RecvPropDataTable("hl2_gamerules_data", 0, 0, REFERENCE_RECV_TABLE(DT_HL2GameRules), RecvProxy_HL2GameRules)
	END_RECV_TABLE()
	END_INIT_RECV_TABLE()
#endif
};


class CHalfLife2 : public CSingleplayRules
{
public:
	DECLARE_CLASS( CHalfLife2, CSingleplayRules );
#if !defined( CLIENT_DLL )
	DECLARE_SERVERCLASS()
#else
	DECLARE_CLIENTCLASS();
#endif
	// Damage Query Overrides.
	virtual bool			Damage_IsTimeBased( int iDmgType );
	// TEMP:
	virtual int				Damage_GetTimeBased( void );
	
	virtual bool			ShouldCollide( int collisionGroup0, int collisionGroup1 );
	virtual bool			ShouldUseRobustRadiusDamage(CBaseEntity *pEntity);
#ifndef CLIENT_DLL
	virtual bool			ShouldAutoAim( CBasePlayer *pPlayer, edict_t *target );
	virtual float			GetAutoAimScale( CBasePlayer *pPlayer );
	virtual float			GetAmmoQuantityScale( int iAmmoIndex );
	virtual void			LevelInitPreEntity();
#endif

private:
	// Rules change for the mega physgun
	CNetworkVar( bool, m_bMegaPhysgun );

#ifdef CLIENT_DLL

	DECLARE_CLIENTCLASS_NOBASE(); // This makes datatables able to access our private vars.

#else

	DECLARE_SERVERCLASS_NOBASE(); // This makes datatables able to access our private vars.

	CHalfLife2();
	virtual ~CHalfLife2() {}

	virtual void			Think( void );

	virtual bool			ClientCommand( CBaseEntity *pEdict, const CCommand &args );
	virtual void			PlayerSpawn( CBasePlayer *pPlayer );

	virtual void			InitDefaultAIRelationships( void );
	virtual const char*		AIClassText(int classType);
	virtual const char *GetGameDescription( void ) { return "Half-Life 2"; }

	// Ammo
	virtual void			PlayerThink( CBasePlayer *pPlayer );
	virtual float			GetAmmoDamage( CBaseEntity *pAttacker, CBaseEntity *pVictim, int nAmmoType );

	virtual bool			ShouldBurningPropsEmitLight();
public:

	bool AllowDamage( CBaseEntity *pVictim, const CTakeDamageInfo &info );

	bool	NPC_ShouldDropGrenade( CBasePlayer *pRecipient );
	bool	NPC_ShouldDropHealth( CBasePlayer *pRecipient );
	void	NPC_DroppedHealth( void );
	void	NPC_DroppedGrenade( void );
	bool	MegaPhyscannonActive( void ) { return m_bMegaPhysgun;	}
	
	virtual bool IsAlyxInDarknessMode();

private:

	float	m_flLastHealthDropTime;
	float	m_flLastGrenadeDropTime;

	void AdjustPlayerDamageTaken( CTakeDamageInfo *pInfo );
	float AdjustPlayerDamageInflicted( float damage );

	int						DefaultFOV( void ) { return 75; }
#endif

public:
	//BEGIN_NETWORK_TABLE_NOBASE(CHalfLife2, DT_HL2GameRules)
#ifdef CLIENT_DLL
	BEGIN_INIT_RECV_TABLE(CHalfLife2)
	BEGIN_RECV_TABLE_NOBASE(CHalfLife2, DT_HL2GameRules)
		RecvPropBool(RECVINFO(m_bMegaPhysgun)),
	END_RECV_TABLE()
	END_INIT_RECV_TABLE()
#else
	BEGIN_INIT_SEND_TABLE(CHalfLife2)
	BEGIN_SEND_TABLE_NOBASE(CHalfLife2, DT_HL2GameRules)
		SendPropBool(SENDINFO(m_bMegaPhysgun)),
	END_SEND_TABLE()
	END_INIT_SEND_TABLE()
#endif
	//END_NETWORK_TABLE()
};


//-----------------------------------------------------------------------------
// Gets us at the Half-Life 2 game rules
//-----------------------------------------------------------------------------
inline CHalfLife2* HL2GameRules()
{
#if ( !defined( HL2_DLL ) && !defined( HL2_CLIENT_DLL ) ) || defined( HL2MP )
	Assert( 0 );	// g_pGameRules is NOT an instance of CHalfLife2 and bad things happen
#endif

	return static_cast<CHalfLife2*>(g_pGameRules);
}



#endif // HL2_GAMERULES_H
