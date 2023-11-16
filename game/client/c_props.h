//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#ifndef C_PROPS_H
#define C_PROPS_H
#ifdef _WIN32
#pragma once
#endif

#include "c_breakableprop.h"
#include "props_shared.h"

#define CDynamicProp C_DynamicProp

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CDynamicProp : public C_BreakableProp
{
	DECLARE_CLASS(CDynamicProp, C_BreakableProp );
public:
	DECLARE_NETWORKCLASS();

	// constructor, destructor
	CDynamicProp( void );
	~CDynamicProp( void );

	void GetRenderBounds( Vector& theMins, Vector& theMaxs );
	unsigned int ComputeClientSideAnimationFlags();
	bool TestBoneFollowers( const Ray_t &ray, unsigned int fContentsMask, trace_t& tr );
	bool TestCollision( const Ray_t &ray, unsigned int fContentsMask, trace_t& tr );

private:
	CDynamicProp( const CDynamicProp& );

protected:
	CNetworkVar( bool,	m_bUseHitboxesForRenderBox);
private:
	int		m_iCachedFrameCount;
	Vector	m_vecCachedRenderMins;
	Vector	m_vecCachedRenderMaxs;

public:
	BEGIN_INIT_RECV_TABLE(CDynamicProp)
	BEGIN_RECV_TABLE(CDynamicProp, DT_DynamicProp, DT_BreakableProp)
		RecvPropBool(RECVINFO(m_bUseHitboxesForRenderBox)),
	END_RECV_TABLE(DT_DynamicProp)
	END_INIT_RECV_TABLE()
};

#endif // C_PROPS_H
