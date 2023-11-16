//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "fx_cs_shared.h"
#include "c_cs_player.h"
#include "c_basetempentity.h"
#include <cliententitylist.h>


class C_TEFireBullets : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEFireBullets, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

public:
	CNetworkVar( int,		m_iPlayer);
	CNetworkVector(	m_vecOrigin);
	CNetworkQAngle(	m_vecAngles);
	CNetworkVar( int,		m_iWeaponID);
	CNetworkVar( int,		m_iMode);
	CNetworkVar( int,		m_iSeed);
	CNetworkVar( float,	m_fInaccuracy);
	CNetworkVar( float,	m_fSpread);

public:
	BEGIN_INIT_RECV_TABLE(C_TEFireBullets)
	BEGIN_RECV_TABLE_NOBASE(C_TEFireBullets, DT_TEFireBullets)
		RecvPropVector(RECVINFO(m_vecOrigin)),
		RecvPropFloat(RECVINFO_VECTORELEM(m_vecAngles,0)),
		RecvPropFloat(RECVINFO_VECTORELEM(m_vecAngles,1)),
		RecvPropInt(RECVINFO(m_iWeaponID)),
		RecvPropInt(RECVINFO(m_iMode)),
		RecvPropInt(RECVINFO(m_iSeed)),
		RecvPropInt(RECVINFO(m_iPlayer)),
		RecvPropFloat(RECVINFO(m_fInaccuracy)),
		RecvPropFloat(RECVINFO(m_fSpread)),
	END_RECV_TABLE(DT_TEFireBullets)
	END_INIT_RECV_TABLE()
};


void C_TEFireBullets::PostDataUpdate( DataUpdateType_t updateType )
{
	// Create the effect.
	
	m_vecAngles.SetZ(0);
	
	FX_FireBullets( 
		m_iPlayer+1,
		m_vecOrigin,
		m_vecAngles,
		m_iWeaponID,
		m_iMode,
		m_iSeed,
		m_fInaccuracy,
		m_fSpread
		);
}


IMPLEMENT_CLIENTCLASS_EVENT(C_TEFireBullets, DT_TEFireBullets, CTEFireBullets );
LINK_ENTITY_TO_CLASS(TEFireBullets, C_TEFireBullets);
PRECACHE_REGISTER(TEFireBullets);



class C_TEPlantBomb : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEPlantBomb, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

public:
	CNetworkVar( int,		m_iPlayer);
	CNetworkVector(	m_vecOrigin);
	CNetworkVar( int,	m_option);//PlantBombOption_t

public:
	BEGIN_INIT_RECV_TABLE(C_TEPlantBomb)
	BEGIN_RECV_TABLE_NOBASE(C_TEPlantBomb, DT_TEPlantBomb)
		RecvPropVector(RECVINFO(m_vecOrigin)),
		RecvPropInt(RECVINFO(m_iPlayer)),
		RecvPropInt(RECVINFO(m_option)),
	END_RECV_TABLE(DT_TEPlantBomb)
	END_INIT_RECV_TABLE()
};


void C_TEPlantBomb::PostDataUpdate( DataUpdateType_t updateType )
{
	// Create the effect.
	FX_PlantBomb( m_iPlayer+1, m_vecOrigin, (PlantBombOption_t)(int)m_option );
}


IMPLEMENT_CLIENTCLASS_EVENT(C_TEPlantBomb, DT_TEPlantBomb, CTEPlantBomb );
LINK_ENTITY_TO_CLASS(TEPlantBomb, C_TEPlantBomb);
PRECACHE_REGISTER(TEPlantBomb);



