//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_ENTITY_DISSOLVE_H
#define C_ENTITY_DISSOLVE_H

#include "cbase.h"

//-----------------------------------------------------------------------------
// Entity Dissolve, client-side implementation
//-----------------------------------------------------------------------------
class C_EntityDissolve : public C_BaseEntity, public IMotionEvent
{
public:
	DECLARE_CLIENTCLASS();
	DECLARE_CLASS( C_EntityDissolve, C_BaseEntity );

	C_EntityDissolve( void );

	// Inherited from C_BaseEntity
	virtual void	GetRenderBounds( Vector& theMins, Vector& theMaxs );
	virtual int		DrawModel(IVModel* pWorld, int flags );
	virtual bool	ShouldDraw() { return true; }
	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual void	UpdateOnRemove( void );

	virtual Vector	GetEffectColor( void ) { return m_vEffectColor; }
	virtual void	SetEffectColor( Vector v ) { m_vEffectColor = v; }

	// Inherited from IMotionEvent
	virtual simresult_e	Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular );
	
	void			SetupEmitter( void );

	void			ClientThink( void );

	void			SetServerLinkState( bool state ) { m_bLinkedToServerEnt = state; }

	CNetworkVar( float,	m_flStartTime);
	CNetworkVar(float,	m_flFadeOutStart);
	CNetworkVar(float,	m_flFadeOutLength);
	CNetworkVar(float,	m_flFadeOutModelStart);
	CNetworkVar(float,	m_flFadeOutModelLength);
	CNetworkVar(float,	m_flFadeInStart);
	CNetworkVar(float,	m_flFadeInLength);
	CNetworkVar(int,		m_nDissolveType);
	float   m_flNextSparkTime;

	Vector	m_vEffectColor;

	CNetworkVector(	m_vDissolverOrigin);
	CNetworkVar( int,		m_nMagnitude);

	bool	m_bCoreExplode;

protected:

	float GetFadeInPercentage( void );		// Fade in amount (entity fading to black)
	float GetFadeOutPercentage( void );		// Fade out amount (particles fading away)
	float GetModelFadeOutPercentage( void );// Mode fade out amount

	// Compute the bounding box's center, size, and basis
	void ComputeRenderInfo( mstudiobbox_t *pHitBox, const matrix3x4_t &hitboxToWorld, 
								Vector *pVecAbsOrigin, Vector *pXVec, Vector *pYVec );
	void BuildTeslaEffect( mstudiobbox_t *pHitBox, const matrix3x4_t &hitboxToWorld, bool bRandom, float flYawOffset );

	void DoSparks( mstudiohitboxset_t *set, matrix3x4_t *hitboxbones[MAXSTUDIOBONES] );

private:

	CSmartPtr<CSimpleEmitter>	m_pEmitter;

	bool	m_bLinkedToServerEnt;
	IPhysicsMotionController	*m_pController;

public:
	BEGIN_INIT_RECV_TABLE(C_EntityDissolve)
	BEGIN_RECV_TABLE(C_EntityDissolve, DT_EntityDissolve, DT_BaseEntity)
		RecvPropTime(RECVINFO(m_flStartTime)),
		RecvPropFloat(RECVINFO(m_flFadeOutStart)),
		RecvPropFloat(RECVINFO(m_flFadeOutLength)),
		RecvPropFloat(RECVINFO(m_flFadeOutModelStart)),
		RecvPropFloat(RECVINFO(m_flFadeOutModelLength)),
		RecvPropFloat(RECVINFO(m_flFadeInStart)),
		RecvPropFloat(RECVINFO(m_flFadeInLength)),
		RecvPropInt(RECVINFO(m_nDissolveType)),
		RecvPropVector(RECVINFO(m_vDissolverOrigin)),
		RecvPropInt(RECVINFO(m_nMagnitude)),
	END_RECV_TABLE(DT_EntityDissolve)
	END_INIT_RECV_TABLE()
};

#endif // C_ENTITY_DISSOLVE_H

