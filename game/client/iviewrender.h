//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//===========================================================================//
#if !defined( IVIEWRENDER_H )
#define IVIEWRENDER_H
#ifdef _WIN32
#pragma once
#endif

#include "ivrenderview.h"

// near and far Z it uses to render the world.
#define VIEW_NEARZ	3
//#define VIEW_FARZ	28400

class VMatrix;
class Vector;
class QAngle;
class VPlane;


// These are set as it draws reflections, refractions, etc, so certain effects can avoid 
// drawing themselves in reflections.
enum DrawFlags_t
{
	DF_RENDER_REFRACTION	= 0x1,
	DF_RENDER_REFLECTION	= 0x2,

	DF_CLIP_Z				= 0x4,
	DF_CLIP_BELOW			= 0x8,

	DF_RENDER_UNDERWATER	= 0x10,
	DF_RENDER_ABOVEWATER	= 0x20,
	DF_RENDER_WATER			= 0x40,

	DF_SSAO_DEPTH_PASS		= 0x100,
	DF_WATERHEIGHT			= 0x200,
	DF_DRAW_SSAO			= 0x400,
	DF_DRAWSKYBOX			= 0x800,

	DF_FUDGE_UP				= 0x1000,

	DF_DRAW_ENTITITES		= 0x2000,
	DF_UNUSED3				= 0x4000,

	DF_UNUSED4				= 0x8000,

	DF_UNUSED5				= 0x10000,
	DF_SAVEGAMESCREENSHOT	= 0x20000,
	DF_CLIP_SKYBOX			= 0x40000,

	DF_SHADOW_DEPTH_MAP		= 0x100000	// Currently rendering a shadow depth map
};

//-----------------------------------------------------------------------------
// Computes draw flags for the engine to build its world surface lists
//-----------------------------------------------------------------------------
static inline unsigned long BuildEngineDrawWorldListFlags(unsigned nDrawFlags)
{
	unsigned long nEngineFlags = 0;

	if (nDrawFlags & DF_DRAWSKYBOX)
	{
		nEngineFlags |= DRAWWORLDLISTS_DRAW_SKYBOX;
	}

	if (nDrawFlags & DF_RENDER_ABOVEWATER)
	{
		nEngineFlags |= DRAWWORLDLISTS_DRAW_STRICTLYABOVEWATER;
		nEngineFlags |= DRAWWORLDLISTS_DRAW_INTERSECTSWATER;
	}

	if (nDrawFlags & DF_RENDER_UNDERWATER)
	{
		nEngineFlags |= DRAWWORLDLISTS_DRAW_STRICTLYUNDERWATER;
		nEngineFlags |= DRAWWORLDLISTS_DRAW_INTERSECTSWATER;
	}

	if (nDrawFlags & DF_RENDER_WATER)
	{
		nEngineFlags |= DRAWWORLDLISTS_DRAW_WATERSURFACE;
	}

	if (nDrawFlags & DF_CLIP_SKYBOX)
	{
		nEngineFlags |= DRAWWORLDLISTS_DRAW_CLIPSKYBOX;
	}

	if (nDrawFlags & DF_SHADOW_DEPTH_MAP)
	{
		nEngineFlags |= DRAWWORLDLISTS_DRAW_SHADOWDEPTH;
	}

	if (nDrawFlags & DF_RENDER_REFRACTION)
	{
		nEngineFlags |= DRAWWORLDLISTS_DRAW_REFRACTION;
	}

	if (nDrawFlags & DF_RENDER_REFLECTION)
	{
		nEngineFlags |= DRAWWORLDLISTS_DRAW_REFLECTION;
	}

	if (nDrawFlags & DF_SSAO_DEPTH_PASS)
	{
		nEngineFlags |= DRAWWORLDLISTS_DRAW_SSAO | DRAWWORLDLISTS_DRAW_STRICTLYUNDERWATER | DRAWWORLDLISTS_DRAW_INTERSECTSWATER | DRAWWORLDLISTS_DRAW_STRICTLYABOVEWATER;
		nEngineFlags &= ~(DRAWWORLDLISTS_DRAW_WATERSURFACE | DRAWWORLDLISTS_DRAW_REFRACTION | DRAWWORLDLISTS_DRAW_REFLECTION);
	}

	return nEngineFlags;
}

// This identifies the view for certain systems that are unique per view (e.g. pixel visibility)
// NOTE: This is identifying which logical part of the scene an entity is being redered in
// This is not identifying a particular render target necessarily.  This is mostly needed for entities that
// can be rendered more than once per frame (pixel vis queries need to be identified per-render call)
enum view_id_t
{
	VIEW_ILLEGAL = -2,
	VIEW_NONE = -1,
	VIEW_MAIN = 0,
	VIEW_3DSKY = 1,
	VIEW_MONITOR = 2,
	VIEW_REFLECTION = 3,
	VIEW_REFRACTION = 4,
	VIEW_INTRO_PLAYER = 5,
	VIEW_INTRO_CAMERA = 6,
	VIEW_SHADOW_DEPTH_TEXTURE = 7,
	VIEW_SSAO = 8,
	VIEW_ID_COUNT
};

//-----------------------------------------------------------------------------
// Purpose: View setup and rendering
//-----------------------------------------------------------------------------
class CViewSetup;
class C_BaseEntity;
struct vrect_t;
class C_BaseViewModel;
struct WriteReplayScreenshotParams_t;
class IReplayScreenshotSystem;

//-----------------------------------------------------------------------------
// Data specific to intro mode to control rendering.
//-----------------------------------------------------------------------------
struct IntroDataBlendPass_t
{
	int m_BlendMode;
	float m_Alpha; // in [0.0f,1.0f]  This needs to add up to 1.0 for all passes, unless you are fading out.
};

struct IntroData_t
{
	bool	m_bDrawPrimary;
	Vector	m_vecCameraView;
	QAngle	m_vecCameraViewAngles;
	float	m_playerViewFOV;
	CUtlVector<IntroDataBlendPass_t> m_Passes;

	// Fade overriding for the intro
	float	m_flCurrentFadeColor[4];
};

abstract_class IViewRender
{
public:
	// SETUP
	// Initialize view renderer
	virtual void		Init( void ) = 0;

	// Clear any systems between levels
	virtual void		LevelInit( void ) = 0;
	virtual void		LevelShutdown( void ) = 0;

	// Shutdown
	virtual void		Shutdown( void ) = 0;

	// RENDERING
	// Called right before simulation. It must setup the view model origins and angles here so 
	// the correct attachment points can be used during simulation.	
	virtual void		OnRenderStart() = 0;

	// Called to render the entire scene
	virtual	void		RenderRect( vrect_t *rect ) = 0;

	// Called to render just a particular setup ( for timerefresh and envmap creation )
	virtual void		RenderView( const CViewSetup &view, int nClearFlags, int whatToDraw ) = 0;

	// What are we currently rendering? Returns a combination of DF_ flags.
	virtual int GetDrawFlags() = 0;

	// MISC
	// Start and stop pitch drifting logic
	virtual void		StartPitchDrift( void ) = 0;
	virtual void		StopPitchDrift( void ) = 0;

	// This can only be called during rendering (while within RenderView).
	virtual VPlane*		GetFrustum() = 0;

	virtual bool		ShouldDrawBrushModels( void ) = 0;

	virtual const CViewSetup *GetPlayerViewSetup( void ) const = 0;
	virtual const CViewSetup *GetViewSetup( void ) const = 0;

	virtual void		DisableVis( void ) = 0;

	virtual int			BuildWorldListsNumber() const = 0;

	virtual void		SetCheapWaterStartDistance( float flCheapWaterStartDistance ) = 0;
	virtual void		SetCheapWaterEndDistance( float flCheapWaterEndDistance ) = 0;

	virtual void		GetWaterLODParams( float &flCheapWaterStartDistance, float &flCheapWaterEndDistance ) = 0;

	virtual void		DriftPitch (void) = 0;

	virtual void		SetScreenOverlayMaterial( IMaterial *pMaterial ) = 0;
	virtual IMaterial	*GetScreenOverlayMaterial( ) = 0;

#if !defined( _X360 )
#define SAVEGAME_SCREENSHOT_WIDTH	180
#define SAVEGAME_SCREENSHOT_HEIGHT	100
#else
#define SAVEGAME_SCREENSHOT_WIDTH	128
#define SAVEGAME_SCREENSHOT_HEIGHT	128
#endif

	virtual void		WriteSaveGameScreenshot( const char *pFilename ) = 0;
	virtual void		WriteSaveGameScreenshotOfSize( const char *pFilename, int width, int height, bool bCreatePowerOf2Padded = false, bool bWriteVTF = false ) = 0;

	virtual void		WriteReplayScreenshot( WriteReplayScreenshotParams_t &params ) = 0;
	virtual void		UpdateReplayScreenshotCache() = 0;

	// Draws another rendering over the top of the screen
	virtual void		QueueOverlayRenderView( const CViewSetup &view, int nClearFlags, int whatToDraw ) = 0;

	// Returns znear and zfar
	virtual float		GetZNear() = 0;
	virtual float		GetZFar() = 0;

	virtual void		GetScreenFadeDistances( float *min, float *max ) = 0;

	virtual C_BaseEntity *GetCurrentlyDrawingEntity() = 0;
	//virtual void		SetCurrentlyDrawingEntity( C_BaseEntity *pEnt ) = 0;

	virtual bool		UpdateShadowDepthTexture( ITexture *pRenderTarget, ITexture *pDepthTexture, const CViewSetup &shadowView ) = 0;

	virtual void		FreezeFrame( float flFreezeTime ) = 0;

	virtual IReplayScreenshotSystem *GetReplayScreenshotSystem() = 0;

	//-----------------------------------------------------------------------------
// There's a difference between the 'current view' and the 'main view'
// The 'main view' is where the player is sitting. Current view is just
// what's currently being rendered, which, owing to monitors or water,
// could be just about anywhere.
//-----------------------------------------------------------------------------
	virtual const Vector& MainViewOrigin() = 0;
	virtual const QAngle& MainViewAngles() = 0;
	virtual const Vector& PrevMainViewOrigin() = 0;
	virtual const QAngle& PrevMainViewAngles() = 0;
	virtual const VMatrix& MainWorldToViewMatrix() = 0;
	virtual const Vector& MainViewForward() = 0;
	virtual const Vector& MainViewRight() = 0;
	virtual const Vector& MainViewUp() = 0;

	virtual const Vector& CurrentViewOrigin() = 0;
	virtual const QAngle& CurrentViewAngles() = 0;
	virtual const VMatrix& CurrentWorldToViewMatrix() = 0;
	virtual const Vector& CurrentViewForward() = 0;
	virtual const Vector& CurrentViewRight() = 0;
	virtual const Vector& CurrentViewUp() = 0;

	virtual void AllowCurrentViewAccess(bool allow) = 0;
	virtual bool IsCurrentViewAccessAllowed() = 0;
	virtual view_id_t CurrentViewID() = 0;
	virtual bool DrawingMainView() = 0;
	virtual bool DrawingShadowDepthView(void) = 0;
	virtual bool IsRenderingScreenshot() = 0;
	virtual IntroData_t* GetIntroData() = 0;
	virtual void SetIntroData(IntroData_t* introData) = 0;
	//virtual void SetupCurrentView(const Vector& vecOrigin, const QAngle& angles, view_id_t viewID) = 0;
	//virtual void SetupCurrentView(view_id_t viewID) = 0;
	//virtual void FinishCurrentView() = 0;
	//virtual bool IsMainView(view_id_t id) = 0;
	//virtual void DrawClippedDepthBox(IClientRenderable* pEnt, float* pClipPlane) = 0;
	//virtual inline void DrawOpaqueRenderable(IClientRenderable* pEnt, bool bTwoPass, ERenderDepthMode DepthMode, int nDefaultFlags = 0) = 0;
	//virtual inline void DrawTranslucentRenderable(IClientRenderable* pEnt, bool twoPass, bool bShadowDepth, bool bIgnoreDepth) = 0;
	//virtual void DrawOpaqueRenderables_Range(CClientRenderablesList::CEntry* pEntitiesBegin, CClientRenderablesList::CEntry* pEntitiesEnd, ERenderDepthMode DepthMode) = 0;
	//virtual void DrawOpaqueRenderables_DrawStaticProps(CClientRenderablesList::CEntry* pEntitiesBegin, CClientRenderablesList::CEntry* pEntitiesEnd, ERenderDepthMode DepthMode) = 0;
	//virtual void DrawOpaqueRenderables_DrawBrushModels(CClientRenderablesList::CEntry* pEntitiesBegin, CClientRenderablesList::CEntry* pEntitiesEnd, ERenderDepthMode DepthMode) = 0;
	//virtual void SetClearColorToFogColor() = 0;
};

extern IViewRender *g_pView;

#endif // IVIEWRENDER_H
