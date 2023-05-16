//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef C_BASEANIMATINGOVERLAY_H
#define C_BASEANIMATINGOVERLAY_H
#pragma once

#include "c_baseanimating.h"
#include "dt_utlvector_recv.h"

// For shared code.
#define CBaseAnimatingOverlay C_BaseAnimatingOverlay

void ResizeAnimationLayerCallback(void* pStruct, int offsetToUtlVector, int len);

class C_BaseAnimatingOverlay : public C_BaseAnimating
{
public:
	DECLARE_CLASS( C_BaseAnimatingOverlay, C_BaseAnimating );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();


	C_BaseAnimatingOverlay();

	virtual CStudioHdr *OnNewModel();

	C_AnimationLayer* GetAnimOverlay( int i );
	void SetNumAnimOverlays( int num );	// This makes sure there is space for this # of layers.
	int GetNumAnimOverlays() const;

	virtual void	GetRenderBounds( Vector& theMins, Vector& theMaxs );

	void			CheckForLayerChanges( CStudioHdr *hdr, float currentTime );

	// model specific
	virtual void	AccumulateLayers( IBoneSetup &boneSetup, Vector pos[], Quaternion q[], float currentTime );
	virtual void DoAnimationEvents( CStudioHdr *pStudioHdr );

	enum
	{
		MAX_OVERLAYS = 15,
	};

	CUtlVector < C_AnimationLayer >	m_AnimOverlay;

	CUtlVector < CInterpolatedVar< C_AnimationLayer > >	m_iv_AnimOverlay;

	float m_flOverlayPrevEventCycle[ MAX_OVERLAYS ];

private:
	C_BaseAnimatingOverlay( const C_BaseAnimatingOverlay & ); // not defined, not accessible

	BEGIN_RECV_TABLE_NOBASE(C_BaseAnimatingOverlay, DT_OverlayVars)
		RecvPropUtlVector(
			RECVINFO_UTLVECTOR_SIZEFN(m_AnimOverlay, ResizeAnimationLayerCallback),
			C_BaseAnimatingOverlay::MAX_OVERLAYS,
			RecvPropDataTable(NULL, 0, 0, REFERENCE_RECV_TABLE(DT_Animationlayer)))
	END_RECV_TABLE(DT_OverlayVars)

	BEGIN_RECV_TABLE(C_BaseAnimatingOverlay, DT_BaseAnimatingOverlay, DT_BaseAnimating)
		RecvPropDataTable("overlay_vars", 0, 0, REFERENCE_RECV_TABLE(DT_OverlayVars))
	END_RECV_TABLE(DT_BaseAnimatingOverlay)
};


EXTERN_RECV_TABLE(DT_BaseAnimatingOverlay);


#endif // C_BASEANIMATINGOVERLAY_H




