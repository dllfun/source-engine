//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "view.h"
#include "iviewrender.h"
#include "model_types.h"
#include "ivrenderview.h"
#include "engine/ivmodelinfo.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define VIEWER_PADDING	80.0f

class C_AreaPortal : public C_BaseEntity{
public:
	DECLARE_CLIENTCLASS();
	C_AreaPortal() {}

public:
	BEGIN_INIT_RECV_TABLE(C_AreaPortal)
	BEGIN_RECV_TABLE(C_AreaPortal, DT_AreaPortal, DT_BaseEntity)
		
	END_RECV_TABLE(DT_AreaPortal)
	END_INIT_RECV_TABLE()
};

IMPLEMENT_CLIENTCLASS(C_AreaPortal, DT_AreaPortal, CAreaPortal)

class C_FuncAreaPortalWindow : public C_BaseEntity
{
public:
	DECLARE_CLIENTCLASS();
	DECLARE_CLASS( C_FuncAreaPortalWindow, C_BaseEntity );


// Overrides.
public:

	virtual void	ComputeFxBlend();
	virtual bool	IsTransparent();
	virtual int		DrawModel(IVModel* pWorld, int flags );
	virtual bool	ShouldReceiveProjectedTextures( int flags );

private:

	float			GetDistanceBlend();



public:
	CNetworkVar( float,			m_flFadeStartDist);	// Distance at which it starts fading (when <= this, alpha=m_flTranslucencyLimit).
	CNetworkVar( float,			m_flFadeDist);		// Distance at which it becomes solid.

	// 0-1 value - minimum translucency it's allowed to get to.
	CNetworkVar( float,			m_flTranslucencyLimit);

	CNetworkVar( int,				m_iBackgroundModelIndex);

public:
	BEGIN_INIT_RECV_TABLE(C_FuncAreaPortalWindow)
	BEGIN_RECV_TABLE(C_FuncAreaPortalWindow, DT_FuncAreaPortalWindow, DT_BaseEntity)
		RecvPropFloat(RECVINFO(m_flFadeStartDist)),
		RecvPropFloat(RECVINFO(m_flFadeDist)),
		RecvPropFloat(RECVINFO(m_flTranslucencyLimit)),
		RecvPropInt(RECVINFO(m_iBackgroundModelIndex))
	END_RECV_TABLE(DT_FuncAreaPortalWindow)
	END_INIT_RECV_TABLE()
};



IMPLEMENT_CLIENTCLASS( C_FuncAreaPortalWindow, DT_FuncAreaPortalWindow, CFuncAreaPortalWindow )




void C_FuncAreaPortalWindow::ComputeFxBlend()
{
	// We reset our blend down below so pass anything except 0 to the renderer.
	m_nRenderFXBlend = 255;

#ifdef _DEBUG
	m_nFXComputeFrame = gpGlobals->GetFrameCount();
#endif

}


bool C_FuncAreaPortalWindow::IsTransparent()
{
	return true;
}


int C_FuncAreaPortalWindow::DrawModel(IVModel* pWorld, int flags )
{
	if ( !m_bReadyToDraw )
		return 0;

	if( !GetModel() )
		return 0;

	// Make sure we're a brush model.
	int modelType = GetModel()?GetModel()->GetModelType():mod_bad;//GetModel()
	if( modelType != mod_brush )
		return 0;

	// Draw the fading version.
	render->SetBlend( GetDistanceBlend() );

	DrawBrushModelMode_t mode = DBM_DRAW_ALL;
	if ( flags & STUDIO_TWOPASS )
	{
		mode = ( flags & STUDIO_TRANSPARENCY ) ? DBM_DRAW_TRANSLUCENT_ONLY : DBM_DRAW_OPAQUE_ONLY;
	}

	render->DrawBrushModelEx( 
		this, 
		(IVModel *)GetModel(), 
		GetAbsOrigin(), 
		GetAbsAngles(), 
		mode );

	// Draw the optional foreground model next.
	// Only use the alpha in the texture from the thing in the front.
	if (m_iBackgroundModelIndex >= 0)
	{
		render->SetBlend( 1 );
		IVModel *pBackground = ( IVModel * )engineClient->GetModel( m_iBackgroundModelIndex );
		if( pBackground && pBackground->GetModelType() == mod_brush )//pBackground
		{
			render->DrawBrushModelEx( 
				this, 
				pBackground, 
				GetAbsOrigin(), 
				GetAbsAngles(), 
				mode );
		}
	}

	return 1;
}


float C_FuncAreaPortalWindow::GetDistanceBlend()
{
	// Get the viewer's distance to us.
	float flDist = CollisionProp()->CalcDistanceFromPoint(g_pView->CurrentViewOrigin() );
	C_BasePlayer *local = C_BasePlayer::GetLocalPlayer();
	if ( local )
	{
		flDist *= local->GetFOVDistanceAdjustFactor();
	}
	
	return RemapValClamped( flDist, m_flFadeStartDist, m_flFadeDist, m_flTranslucencyLimit, 1 );
}

bool C_FuncAreaPortalWindow::ShouldReceiveProjectedTextures( int flags )
{
	return false;
}

class C_FuncBrush : public C_BaseEntity {
public:
	DECLARE_CLIENTCLASS();
	C_FuncBrush() {}

public:
	BEGIN_INIT_RECV_TABLE(C_FuncBrush)
	BEGIN_RECV_TABLE(C_FuncBrush, DT_FuncBrush, DT_BaseEntity)

	END_RECV_TABLE(DT_FuncBrush)
	END_INIT_RECV_TABLE()
};

IMPLEMENT_CLIENTCLASS(C_FuncBrush, DT_FuncBrush, CFuncBrush)

