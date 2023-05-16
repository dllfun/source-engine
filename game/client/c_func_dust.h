//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_FUNC_DUST_H
#define C_FUNC_DUST_H
#ifdef _WIN32
#pragma once
#endif


#include "c_baseentity.h"
#include "particles_simple.h"
#include "particle_util.h"
#include "bspflags.h"



// ------------------------------------------------------------------------------------ //
// CDustEffect particle renderer.
// ------------------------------------------------------------------------------------ //

class C_Func_Dust;

class CFuncDustParticle : public Particle
{
public:
	Vector		m_vVelocity;
	float		m_flLifetime;
	float		m_flDieTime;
	float		m_flSize;
	color32		m_Color;
};

class CDustEffect : public CParticleEffect
{
public:
	CDustEffect( const char *pDebugName ) : CParticleEffect( pDebugName ) {}

	virtual void RenderParticles( CParticleRenderIterator *pIterator );
	virtual void SimulateParticles( CParticleSimulateIterator *pIterator );

	C_Func_Dust		*m_pDust;

private:
	CDustEffect( const CDustEffect & ); // not defined, not accessible
};


// ------------------------------------------------------------------------------------ //
// C_Func_Dust class.
// ------------------------------------------------------------------------------------ //

class C_Func_Dust : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_Func_Dust, C_BaseEntity );
	DECLARE_CLIENTCLASS();

						C_Func_Dust();
	virtual				~C_Func_Dust();
	virtual void		OnDataChanged( DataUpdateType_t updateType );
	virtual void		ClientThink();
	virtual bool		ShouldDraw();


private:

	void				AttemptSpawnNewParticle();



// Vars from server.
public:

	color32			m_Color;
	int				m_SpawnRate;
	
	float			m_flSizeMin;
	float			m_flSizeMax;

	int				m_SpeedMax;

	int				m_LifetimeMin;
	int				m_LifetimeMax;

	int				m_DistMax;

	float			m_FallSpeed;	// extra 'gravity'


public:

	int				m_DustFlags;	// Combination of DUSTFLAGS_



public:
	CDustEffect		m_Effect;
	PMaterialHandle		m_hMaterial;
	TimedEvent			m_Spawner;

private:
	C_Func_Dust( const C_Func_Dust & ); // not defined, not accessible

	BEGIN_RECV_TABLE_NOBASE(C_Func_Dust, DT_Func_Dust, CFunc_Dust)

		RecvPropInt(RECVINFO(m_Color)),
		RecvPropInt(RECVINFO(m_SpawnRate)),
		RecvPropFloat(RECVINFO(m_flSizeMin)),
		RecvPropFloat(RECVINFO(m_flSizeMax)),
		RecvPropInt(RECVINFO(m_LifetimeMin)),
		RecvPropInt(RECVINFO(m_LifetimeMax)),
		RecvPropInt(RECVINFO(m_DustFlags)),
		RecvPropInt(RECVINFO(m_SpeedMax)),
		RecvPropInt(RECVINFO(m_DistMax)),
		RecvPropInt(RECVINFO(m_nModelIndex)),
		RecvPropFloat(RECVINFO(m_FallSpeed)),
		RecvPropDataTable(RECVINFO_DT(m_Collision), 0, REFERENCE_RECV_TABLE(DT_CollisionProperty)),
	END_RECV_TABLE(DT_Func_Dust)
};



#endif // C_FUNC_DUST_H
