//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef ANIMATIONLAYER_H
#define ANIMATIONLAYER_H
#ifdef _WIN32
#pragma once
#endif


#include "rangecheckedvar.h"
#include "lerp_functions.h"
#include "networkvar.h"

#ifdef CLIENT_DLL
#define CAnimationLayer C_AnimationLayer
#endif

typedef CRangeCheckedVar<int, -1, 65535, 0> SequenceType;
typedef CRangeCheckedVar<float, -2, 2, 0> PrevCycleType;
typedef CRangeCheckedVar<float, -5, 5, 0> WeightType;

typedef CRangeCheckedVar<float, -2, 2, 0> CycleType;


class C_AnimationLayer
{
public:
	DECLARE_CLASS_NOBASE(C_AnimationLayer);
	// This allows the datatables to access private members.
	ALLOW_DATATABLES_PRIVATE_ACCESS();

	C_AnimationLayer();
	void Reset();

	void SetOrder( int order );

public:

	bool IsActive( void );

	CNetworkVar(SequenceType,	m_nSequence);
	CNetworkVar(PrevCycleType,	m_flPrevCycle);
	CNetworkVar(WeightType,		m_flWeight);
	CNetworkVar( int,		m_nOrder);

	// used for automatic crossfades between sequence changes
	CRangeCheckedVar<float, -50, 50, 1>	m_flPlaybackRate;
	CNetworkVar(CycleType,				m_flCycle);

	float GetFadeout( float flCurTime );

	void BlendWeight();

	float	m_flLayerAnimtime;
	float	m_flLayerFadeOuttime;

	float   m_flBlendIn;
	float   m_flBlendOut;

	bool    m_bClientBlend;


	// For CNetworkVars.
	void NetworkStateChanged() {}
	void NetworkStateChanged(void* pVar) {}
public:
	BEGIN_INIT_RECV_TABLE(CAnimationLayer)
	BEGIN_RECV_TABLE_NOBASE(CAnimationLayer, DT_Animationlayer)
		RecvPropInt(RECVINFO_NAME(m_nSequence, m_nSequence)),
		RecvPropFloat(RECVINFO_NAME(m_flCycle, m_flCycle)),
		RecvPropFloat(RECVINFO_NAME(m_flPrevCycle, m_flPrevCycle)),
		RecvPropFloat(RECVINFO_NAME(m_flWeight, m_flWeight)),
		RecvPropInt(RECVINFO_NAME(m_nOrder, m_nOrder))
	END_RECV_TABLE(DT_Animationlayer)
	END_INIT_RECV_TABLE()
};


inline C_AnimationLayer::C_AnimationLayer()
{
	Reset();
}

inline void C_AnimationLayer::Reset()
{
	m_nSequence = 0;
	m_flPrevCycle = 0;
	m_flWeight = 0;
	m_flPlaybackRate = 0;
	m_flCycle = 0;
	m_flLayerAnimtime = 0;
	m_flLayerFadeOuttime = 0;
	m_flBlendIn = 0;
	m_flBlendOut = 0;
	m_bClientBlend = false;
}


inline void C_AnimationLayer::SetOrder( int order )
{
	m_nOrder = order;
}

inline float C_AnimationLayer::GetFadeout( float flCurTime )
{
	float s;

    if (m_flLayerFadeOuttime <= 0.0f)
	{
		s = 0;
	}
	else
	{
		// blend in over 0.2 seconds
		s = 1.0 - (flCurTime - m_flLayerAnimtime) / m_flLayerFadeOuttime;
		if (s > 0 && s <= 1.0)
		{
			// do a nice spline curve
			s = 3 * s * s - 2 * s * s * s;
		}
		else if ( s > 1.0f )
		{
			// Shouldn't happen, but maybe curtime is behind animtime?
			s = 1.0f;
		}
	}
	return s;
}


inline C_AnimationLayer LoopingLerp( float flPercent, C_AnimationLayer& from, C_AnimationLayer& to )
{
	C_AnimationLayer output;

	output.m_nSequence = to.m_nSequence;
	output.m_flCycle = LoopingLerp( flPercent, (float)(CycleType)from.m_flCycle, (float)(CycleType)to.m_flCycle );
	output.m_flPrevCycle = to.m_flPrevCycle;
	output.m_flWeight = Lerp( flPercent, (WeightType)from.m_flWeight, (WeightType)to.m_flWeight );
	output.m_nOrder = to.m_nOrder;

	output.m_flLayerAnimtime = to.m_flLayerAnimtime;
	output.m_flLayerFadeOuttime = to.m_flLayerFadeOuttime;
	return output;
}

inline C_AnimationLayer Lerp( float flPercent, const C_AnimationLayer& from, const C_AnimationLayer& to )
{
	C_AnimationLayer output;

	output.m_nSequence = to.m_nSequence;
	output.m_flCycle = Lerp( flPercent, (float)(CycleType)from.m_flCycle, (float)(CycleType)to.m_flCycle );
	output.m_flPrevCycle = to.m_flPrevCycle;
	output.m_flWeight = Lerp( flPercent, (WeightType)from.m_flWeight, (WeightType)to.m_flWeight );
	output.m_nOrder = to.m_nOrder;

	output.m_flLayerAnimtime = to.m_flLayerAnimtime;
	output.m_flLayerFadeOuttime = to.m_flLayerFadeOuttime;
	return output;
}

inline C_AnimationLayer LoopingLerp_Hermite( float flPercent, C_AnimationLayer& prev, C_AnimationLayer& from, C_AnimationLayer& to )
{
	C_AnimationLayer output;

	output.m_nSequence = to.m_nSequence;
	output.m_flCycle = LoopingLerp_Hermite( flPercent, (float)(CycleType)prev.m_flCycle, (float)(CycleType)from.m_flCycle, (float)(CycleType)to.m_flCycle );
	output.m_flPrevCycle = to.m_flPrevCycle;
	output.m_flWeight = Lerp( flPercent, (WeightType)from.m_flWeight, (WeightType)to.m_flWeight );
	output.m_nOrder = to.m_nOrder;

	output.m_flLayerAnimtime = to.m_flLayerAnimtime;
	output.m_flLayerFadeOuttime = to.m_flLayerFadeOuttime;
	return output;
}

// YWB:  Specialization for interpolating euler angles via quaternions...
inline C_AnimationLayer Lerp_Hermite( float flPercent, const C_AnimationLayer& prev, const C_AnimationLayer& from, const C_AnimationLayer& to )
{
	C_AnimationLayer output;

	output.m_nSequence = to.m_nSequence;
	output.m_flCycle = Lerp_Hermite( flPercent, (float)(CycleType)prev.m_flCycle, (float)(CycleType)from.m_flCycle, (float)(CycleType)to.m_flCycle );
	output.m_flPrevCycle = to.m_flPrevCycle;
	output.m_flWeight = Lerp( flPercent, (WeightType)from.m_flWeight, (WeightType)to.m_flWeight );
	output.m_nOrder = to.m_nOrder;

	output.m_flLayerAnimtime = to.m_flLayerAnimtime;
	output.m_flLayerFadeOuttime = to.m_flLayerFadeOuttime;
	return output;
}

inline void Lerp_Clamp( C_AnimationLayer &val )
{
	Lerp_Clamp( (SequenceType)val.m_nSequence );
	Lerp_Clamp((CycleType)val.m_flCycle );
	Lerp_Clamp( (PrevCycleType)val.m_flPrevCycle );
	Lerp_Clamp( (WeightType)val.m_flWeight );
	Lerp_Clamp( val.m_nOrder );
	Lerp_Clamp( val.m_flLayerAnimtime );
	Lerp_Clamp( val.m_flLayerFadeOuttime );
}

inline void C_AnimationLayer::BlendWeight()
{
	if ( !m_bClientBlend )
		return;

	m_flWeight = 1;

	// blend in?
	if ( m_flBlendIn != 0.0f )
	{
		if ((CycleType)m_flCycle < m_flBlendIn)
		{
			m_flWeight = (CycleType)m_flCycle / m_flBlendIn;
		}
	}

	// blend out?
	if ( m_flBlendOut != 0.0f )
	{
		if ((CycleType)m_flCycle > 1.0 - m_flBlendOut)
		{
			m_flWeight = (1.0 - (CycleType)m_flCycle) / m_flBlendOut;
		}
	}

	m_flWeight = 3.0 * (WeightType)m_flWeight * (WeightType)m_flWeight - 2.0 * (WeightType)m_flWeight * (WeightType)m_flWeight * (WeightType)m_flWeight;
	if ((SequenceType)m_nSequence == 0)
		m_flWeight = 0;
}

#endif // ANIMATIONLAYER_H
