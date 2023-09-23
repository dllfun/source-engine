//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

// #include "BaseAnimating.h"
#include "dt_utlvector_send.h"

#ifndef BASE_ANIMATING_OVERLAY_H
#define BASE_ANIMATING_OVERLAY_H
#ifdef _WIN32
#pragma once
#endif

#define ORDER_BITS			4
#define WEIGHT_BITS			8

class CBaseAnimatingOverlay;

class CAnimationLayer
{
public:	
	DECLARE_CLASS_NOBASE( CAnimationLayer );

	DECLARE_SEND_TABLE_ACCESS(DT_Animationlayer);

	CAnimationLayer( void );
	void	Init( CBaseAnimatingOverlay *pOverlay );

	// float	SetBlending( int iBlender, float flValue, CBaseAnimating *pOwner );
	void	StudioFrameAdvance( float flInterval, CBaseAnimating *pOwner );
	void	DispatchAnimEvents( CBaseAnimating *eventHandler, CBaseAnimating *pOwner );
	void	SetOrder( int nOrder );

	float GetFadeout( float flCurTime );

	// For CNetworkVars.
	void NetworkStateChanged();
	void NetworkStateChanged( void *pVar );

public:	

#define ANIM_LAYER_ACTIVE		0x0001
#define ANIM_LAYER_AUTOKILL		0x0002
#define ANIM_LAYER_KILLME		0x0004
#define ANIM_LAYER_DONTRESTORE	0x0008
#define ANIM_LAYER_CHECKACCESS	0x0010
#define ANIM_LAYER_DYING		0x0020

	int		m_fFlags;

	bool	m_bSequenceFinished;
	bool	m_bLooping;
	
	CNetworkVar( int, m_nSequence );
	CNetworkVar( float, m_flCycle );
	CNetworkVar( float, m_flPrevCycle );
	CNetworkVar( float, m_flWeight );
	
	float	m_flPlaybackRate;

	float	m_flBlendIn; // start and end blend frac (0.0 for now blend)
	float	m_flBlendOut; 

	float	m_flKillRate;
	float	m_flKillDelay;

	float	m_flLayerAnimtime;
	float	m_flLayerFadeOuttime;

	// For checking for duplicates
	Activity	m_nActivity;

	// order of layering on client
	int		m_nPriority;
	CNetworkVar( int, m_nOrder );

	bool	IsActive( void ) { return ((m_fFlags & ANIM_LAYER_ACTIVE) != 0); }
	bool	IsAutokill( void ) { return ((m_fFlags & ANIM_LAYER_AUTOKILL) != 0); }
	bool	IsKillMe( void ) { return ((m_fFlags & ANIM_LAYER_KILLME) != 0); }
	bool	IsAutoramp( void ) { return (m_flBlendIn != 0.0 || m_flBlendOut != 0.0); }
	void	KillMe( void ) { m_fFlags |= ANIM_LAYER_KILLME; }
	void	Dying( void ) { m_fFlags |= ANIM_LAYER_DYING; }
	bool	IsDying( void ) { return ((m_fFlags & ANIM_LAYER_DYING) != 0); }
	void	Dead( void ) { m_fFlags &= ~ANIM_LAYER_DYING; }

	bool	IsAbandoned( void );
	void	MarkActive( void );

	float	m_flLastEventCheck;

	float	m_flLastAccess;

	// Network state changes get forwarded here.
	CBaseAnimatingOverlay *m_pOwnerEntity;
	
	DECLARE_SIMPLE_DATADESC();

	BEGIN_INIT_SEND_TABLE(CAnimationLayer)
	BEGIN_SEND_TABLE_NOBASE(CAnimationLayer, DT_Animationlayer)
		SendPropInt(SENDINFO(m_nSequence), ANIMATION_SEQUENCE_BITS, SPROP_UNSIGNED),
		SendPropFloat(SENDINFO(m_flCycle), ANIMATION_CYCLE_BITS, SPROP_ROUNDDOWN, 0.0f, 1.0f),
		SendPropFloat(SENDINFO(m_flPrevCycle), ANIMATION_CYCLE_BITS, SPROP_ROUNDDOWN, 0.0f, 1.0f),
		SendPropFloat(SENDINFO(m_flWeight), WEIGHT_BITS, 0, 0.0f, 1.0f),
		SendPropInt(SENDINFO(m_nOrder), ORDER_BITS, SPROP_UNSIGNED),
	END_SEND_TABLE(DT_Animationlayer)
	END_INIT_SEND_TABLE()
};

inline float CAnimationLayer::GetFadeout( float flCurTime )
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



class CBaseAnimatingOverlay : public CBaseAnimating
{
	DECLARE_CLASS( CBaseAnimatingOverlay, CBaseAnimating );
	DECLARE_SEND_TABLE_ACCESS(DT_OverlayVars);
	DECLARE_SEND_TABLE_ACCESS(DT_BaseAnimatingOverlay);

public:
	enum 
	{
		MAX_OVERLAYS = 15,
	};

private:
	CUtlVector< CAnimationLayer	> m_AnimOverlay;
	//int				m_nActiveLayers;
	//int				m_nActiveBaseLayers;

public:
	
	virtual void	OnRestore();

	virtual void	StudioFrameAdvance();
	virtual	void	DispatchAnimEvents ( CBaseAnimating *eventHandler );
	virtual void	GetSkeleton( CStudioHdr *pStudioHdr, Vector pos[], Quaternion q[], int boneMask );

	int		AddGestureSequence( int sequence, bool autokill = true );
	int		AddGestureSequence( int sequence, float flDuration, bool autokill = true );
	int		AddGesture( Activity activity, bool autokill = true );
	int		AddGesture( Activity activity, float flDuration, bool autokill = true );
	bool	IsPlayingGesture( Activity activity );
	void	RestartGesture( Activity activity, bool addifmissing = true, bool autokill = true );
	void	RemoveGesture( Activity activity );
	void	RemoveAllGestures( void );

	int		AddLayeredSequence( int sequence, int iPriority );

	void	SetLayerPriority( int iLayer, int iPriority );

	bool	IsValidLayer( int iLayer );

	void	SetLayerDuration( int iLayer, float flDuration );
	float	GetLayerDuration( int iLayer );

	void	SetLayerCycle( int iLayer, float flCycle );
	void	SetLayerCycle( int iLayer, float flCycle, float flPrevCycle );
	float	GetLayerCycle( int iLayer );

	void	SetLayerPlaybackRate( int iLayer, float flPlaybackRate );
	void	SetLayerWeight( int iLayer, float flWeight );
	float	GetLayerWeight( int iLayer );
	void	SetLayerBlendIn( int iLayer, float flBlendIn );
	void	SetLayerBlendOut( int iLayer, float flBlendOut );
	void	SetLayerAutokill( int iLayer, bool bAutokill );
	void	SetLayerLooping( int iLayer, bool bLooping );
	void	SetLayerNoRestore( int iLayer, bool bNoRestore );

	Activity	GetLayerActivity( int iLayer );
	int			GetLayerSequence( int iLayer );

	int		FindGestureLayer( Activity activity );

	void	RemoveLayer( int iLayer, float flKillRate = 0.2, float flKillDelay = 0.0 );
	void	FastRemoveLayer( int iLayer );

	CAnimationLayer *GetAnimOverlay( int iIndex );
	int GetNumAnimOverlays() const;
	void SetNumAnimOverlays( int num );

	void VerifyOrder( void );

	bool	HasActiveLayer( void );

private:
	int		AllocateLayer( int iPriority = 0 ); // lower priorities are processed first

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DECLARE_PREDICTABLE();

	BEGIN_INIT_SEND_TABLE(CBaseAnimatingOverlay)
	INIT_REFERENCE_SEND_TABLE(CAnimationLayer)
	BEGIN_SEND_TABLE_NOBASE(CBaseAnimatingOverlay, DT_OverlayVars)
		SendPropUtlVector(
		(char*)SENDINFO_UTLVECTOR(m_AnimOverlay),
		CBaseAnimatingOverlay::MAX_OVERLAYS, // max elements
		SendPropDataTable(NULL, 0, REFERENCE_SEND_TABLE(DT_Animationlayer)))
	END_SEND_TABLE(DT_OverlayVars)

	BEGIN_SEND_TABLE(CBaseAnimatingOverlay, DT_BaseAnimatingOverlay, DT_BaseAnimating)
		// These are in their own separate data table so CCSPlayer can exclude all of these.
		SendPropDataTable("overlay_vars", 0, REFERENCE_SEND_TABLE(DT_OverlayVars))
	END_SEND_TABLE(DT_BaseAnimatingOverlay)
	END_INIT_SEND_TABLE()
};

EXTERN_SEND_TABLE(DT_BaseAnimatingOverlay);

inline int CBaseAnimatingOverlay::GetNumAnimOverlays() const
{
	return m_AnimOverlay.Count();
}

// ------------------------------------------------------------------------------------------ //
// CAnimationLayer inlines.
// ------------------------------------------------------------------------------------------ //

inline void CAnimationLayer::SetOrder( int nOrder )
{
	m_nOrder = nOrder;
}

inline void CAnimationLayer::NetworkStateChanged()
{
	if ( m_pOwnerEntity )
		m_pOwnerEntity->NetworkStateChanged();
}

inline void CAnimationLayer::NetworkStateChanged( void *pVar )
{
	if ( m_pOwnerEntity )
		m_pOwnerEntity->NetworkStateChanged();
}

#endif // BASE_ANIMATING_OVERLAY_H
