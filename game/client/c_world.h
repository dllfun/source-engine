//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#if !defined( C_WORLD_H )
#define C_WORLD_H
#ifdef _WIN32
#pragma once
#endif

#include "c_baseentity.h"

#if defined( CLIENT_DLL )
#define CWorld C_World
#endif

class C_World : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_World, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	C_World( void );
	~C_World( void );
	
	// Override the factory create/delete functions since the world is a singleton.
	virtual bool Init( int entnum, int iSerialNum );
	virtual void Release();

	virtual void Precache();
	virtual void Spawn();

	// Don't worry about adding the world to the collision list; it's already there
	virtual CollideType_t	GetCollideType( void )	{ return ENTITY_SHOULD_NOT_COLLIDE; }

	virtual void OnDataChanged( DataUpdateType_t updateType );
	virtual void PreDataUpdate( DataUpdateType_t updateType );

	float GetWaveHeight() const;
	const char *GetDetailSpriteMaterial() const;

public:
	enum
	{
		MAX_DETAIL_SPRITE_MATERIAL_NAME_LENGTH = 256,
	};

	float	m_flWaveHeight;
	Vector	m_WorldMins;
	Vector	m_WorldMaxs;
	bool	m_bStartDark;
	float	m_flMaxOccludeeArea;
	float	m_flMinOccluderArea;
	float	m_flMinPropScreenSpaceWidth;
	float	m_flMaxPropScreenSpaceWidth;
	bool	m_bColdWorld;

private:
	void	RegisterSharedActivities( void );
	char	m_iszDetailSpriteMaterial[MAX_DETAIL_SPRITE_MATERIAL_NAME_LENGTH];

public:
	BEGIN_INIT_RECV_TABLE(C_World)
	BEGIN_RECV_TABLE(C_World, DT_World, DT_BaseEntity)
		RecvPropFloat(RECVINFO(m_flWaveHeight)),
		RecvPropVector(RECVINFO(m_WorldMins)),
		RecvPropVector(RECVINFO(m_WorldMaxs)),
		RecvPropInt(RECVINFO(m_bStartDark)),
		RecvPropFloat(RECVINFO(m_flMaxOccludeeArea)),
		RecvPropFloat(RECVINFO(m_flMinOccluderArea)),
		RecvPropFloat(RECVINFO(m_flMaxPropScreenSpaceWidth)),
		RecvPropFloat(RECVINFO(m_flMinPropScreenSpaceWidth)),
		RecvPropString(RECVINFO(m_iszDetailSpriteMaterial)),
		RecvPropInt(RECVINFO(m_bColdWorld)),
	END_RECV_TABLE(DT_World)
	END_INIT_RECV_TABLE()
};

inline float C_World::GetWaveHeight() const
{
	return m_flWaveHeight;
}

inline const char *C_World::GetDetailSpriteMaterial() const
{
	return m_iszDetailSpriteMaterial;
}

void ClientWorldFactoryInit();
void ClientWorldFactoryShutdown();
//C_World* GetClientWorldEntity();

#endif // C_WORLD_H