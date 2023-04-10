//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#if !defined( VIEWRENDER_H )
#define VIEWRENDER_H
#ifdef _WIN32
#pragma once
#endif

#include "shareddefs.h"
#include "tier1/utlstack.h"
#include "view.h"
#include "iviewrender.h"
#include "view_shared.h"
#include "iinput.h"
#include "input.h"
#include "replay/ireplayscreenshotsystem.h"
#include "mathlib/vector.h"
#include "tier0/vprof.h"
#include "renderparm.h"
#include "clientmode_shared.h"
#include "studio_stats.h"
#include "model_types.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class ConVar;
class CClientRenderablesList;
class IClientVehicle;
class C_PointCamera;
class C_EnvProjectedTexture;
class IScreenSpaceEffect;
class CClientViewSetup;
class CViewRender;
struct ClientWorldListInfo_t;
class C_BaseEntity;
struct WriteReplayScreenshotParams_t;
class CReplayScreenshotTaker;
class VMatrix;
class Vector;
class QAngle;
class VPlane;

#ifdef HL2_EPISODIC
class CStunEffect;
#endif // HL2_EPISODIC

extern ConVar cl_demoviewoverride;
extern ConVar cl_leveloverview;
extern ConVar fog_colorskybox;
extern ConVar fog_color;
extern ConVar fog_enable;
extern ConVar fog_endskybox;
extern ConVar fog_end;
extern ConVar fog_maxdensityskybox;
extern ConVar fog_maxdensity;
extern ConVar fog_override;
extern ConVar fog_startskybox;
extern ConVar fog_start;
extern ConVar r_entityclips;

// Robin, make this point at something to get intro mode.
//extern IntroData_t *g_pIntroData;

//-----------------------------------------------------------------------------
// Purpose: Stored pitch drifting variables
//-----------------------------------------------------------------------------
class CPitchDrift
{
public:
	float		pitchvel;
	bool		nodrift;
	float		driftmove;
	double		laststop;
};

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
struct ViewCustomVisibility_t
{
	ViewCustomVisibility_t()
	{
		m_nNumVisOrigins = 0;
		m_VisData.m_fDistToAreaPortalTolerance = FLT_MAX; 
		m_iForceViewLeaf = -1;
	}

	void AddVisOrigin( const Vector& origin )
	{
		// Don't allow them to write past array length
		AssertMsg( m_nNumVisOrigins < MAX_VIS_LEAVES, "Added more origins than will fit in the array!" );

		// If the vis origin count is greater than the size of our array, just fail to add this origin
		if ( m_nNumVisOrigins >= MAX_VIS_LEAVES )
			return;

		m_rgVisOrigins[ m_nNumVisOrigins++ ] = origin;
	}

	void ForceVisOverride( VisOverrideData_t& visData )
	{
		m_VisData = visData;
	}

	void ForceViewLeaf ( int iViewLeaf )
	{
		m_iForceViewLeaf = iViewLeaf;
	}

	// Set to true if you want to use multiple origins for doing client side map vis culling
	// NOTE:  In generaly, you won't want to do this, and by default the 3d origin of the camera, as above,
	//  will be used as the origin for vis, too.
	int				m_nNumVisOrigins;
	// Array of origins
	Vector			m_rgVisOrigins[ MAX_VIS_LEAVES ];

	// The view data overrides for visibility calculations with area portals
	VisOverrideData_t m_VisData;

	// The starting leaf to determing which area to start in when performing area portal culling on the engine
	// Default behavior is to use the leaf the camera position is in.
	int				m_iForceViewLeaf;
};

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
struct WaterRenderInfo_t
{
	bool m_bCheapWater : 1;
	bool m_bReflect : 1;
	bool m_bRefract : 1;
	bool m_bReflectEntities : 1;
	bool m_bDrawWaterSurface : 1;
	bool m_bOpaqueWater : 1;

};

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
class CBase3dView : public CRefCounted<>,
					protected CViewSetup
{
	DECLARE_CLASS_NOBASE( CBase3dView );
public:
	CBase3dView( CViewRender *pMainView );

	VPlane *		GetFrustum();
	virtual int		GetDrawFlags() { return 0; }

#ifdef PORTAL
	virtual	void	EnableWorldFog() {};
#endif

protected:
	// @MULTICORE (toml 8/11/2006): need to have per-view frustum. Change when move view stack to client
	VPlane			*m_Frustum;
	CViewRender *m_pView;
};

//-----------------------------------------------------------------------------
// Base class for 3d views
//-----------------------------------------------------------------------------
class CRendering3dView : public CBase3dView
{
	DECLARE_CLASS( CRendering3dView, CBase3dView );
public:
	CRendering3dView( CViewRender *pMainView );
	virtual ~CRendering3dView() { ReleaseLists(); }

	void Setup( const CViewSetup &setup );

	// What are we currently rendering? Returns a combination of DF_ flags.
	virtual int		GetDrawFlags();

	virtual void	Draw() {};

protected:

	// Fog setup
	void			EnableWorldFog( void );
	void			SetFogVolumeState( const VisibleFogVolumeInfo_t &fogInfo, bool bUseHeightFog );

	// Draw setup
	void			SetupRenderablesList( int viewID );

	void			UpdateRenderablesOpacity();

	// If iForceViewLeaf is not -1, then it uses the specified leaf as your starting area for setting up area portal culling.
	// This is used by water since your reflected view origin is often in solid space, but we still want to treat it as though
	// the first portal we're looking out of is a water portal, so our view effectively originates under the water.
	void			BuildWorldRenderLists( bool bDrawEntities, int iForceViewLeaf = -1, bool bUseCacheIfEnabled = true, bool bShadowDepth = false, float *pReflectionWaterHeight = NULL );

	// Purpose: Builds render lists for renderables. Called once for refraction, once for over water
	void			BuildRenderableRenderLists( int viewID );

	// More concise version of the above BuildRenderableRenderLists().  Called for shadow depth map rendering
	void			BuildShadowDepthRenderableRenderLists();

	void			DrawWorld( float waterZAdjust );

	// Draws all opaque/translucent renderables in leaves that were rendered
	void			DrawOpaqueRenderables( ERenderDepthMode DepthMode );
	void			DrawTranslucentRenderables( bool bInSkybox, bool bShadowDepth );

	// Renders all translucent entities in the render list
	void			DrawTranslucentRenderablesNoWorld( bool bInSkybox );

	// Draws translucent renderables that ignore the Z buffer
	void			DrawNoZBufferTranslucentRenderables( void );

	// Renders all translucent world surfaces in a particular set of leaves
	void			DrawTranslucentWorldInLeaves( bool bShadowDepth );

	// Renders all translucent world + detail objects in a particular set of leaves
	void			DrawTranslucentWorldAndDetailPropsInLeaves( int iCurLeaf, int iFinalLeaf, int nEngineDrawFlags, int &nDetailLeafCount, LeafIndex_t* pDetailLeafList, bool bShadowDepth );

	// Purpose: Computes the actual world list info based on the render flags
	void			PruneWorldListInfo();

#ifdef PORTAL
	virtual bool	ShouldDrawPortals() { return true; }
#endif

	void ReleaseLists();

	//-----------------------------------------------
	// Combination of DF_ flags.
	int m_DrawFlags;
	int m_ClearFlags;

	IWorldRenderList *m_pWorldRenderList;
	CClientRenderablesList *m_pRenderablesList;
	ClientWorldListInfo_t *m_pWorldListInfo;
	ViewCustomVisibility_t *m_pCustomVisibility;
private:

	//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
	static inline void UpdateBrushModelLightmap(IClientRenderable* pEnt)
	{
		IVModel* pModel = (IVModel*)pEnt->GetModel();
		render->UpdateBrushModelLightmap(pModel, pEnt);
	}

	static void CheckAndTransitionColor(float flPercent, float* pColor, float* pLerpToColor)
	{
		if (pLerpToColor[0] != pColor[0] || pLerpToColor[1] != pColor[1] || pLerpToColor[2] != pColor[2])
		{
			float flDestColor[3];

			flDestColor[0] = pLerpToColor[0];
			flDestColor[1] = pLerpToColor[1];
			flDestColor[2] = pLerpToColor[2];

			pColor[0] = FLerp(pColor[0], flDestColor[0], flPercent);
			pColor[1] = FLerp(pColor[1], flDestColor[1], flPercent);
			pColor[2] = FLerp(pColor[2], flDestColor[2], flPercent);
		}
		else
		{
			pColor[0] = pLerpToColor[0];
			pColor[1] = pLerpToColor[1];
			pColor[2] = pLerpToColor[2];
		}
	}

	static void GetFogColorTransition(fogparams_t* pFogParams, float* pColorPrimary, float* pColorSecondary)
	{
		if (!pFogParams)
			return;

		if (pFogParams->lerptime >= gpGlobals->curtime)
		{
			float flPercent = 1.0f - ((pFogParams->lerptime - gpGlobals->curtime) / pFogParams->duration);

			float flPrimaryColorLerp[3] = { (float)pFogParams->colorPrimaryLerpTo.GetR(), (float)pFogParams->colorPrimaryLerpTo.GetG(), (float)pFogParams->colorPrimaryLerpTo.GetB() };
			float flSecondaryColorLerp[3] = { (float)pFogParams->colorSecondaryLerpTo.GetR(), (float)pFogParams->colorSecondaryLerpTo.GetG(), (float)pFogParams->colorSecondaryLerpTo.GetB() };

			CheckAndTransitionColor(flPercent, pColorPrimary, flPrimaryColorLerp);
			CheckAndTransitionColor(flPercent, pColorSecondary, flSecondaryColorLerp);
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Returns the fog color to use in rendering the current frame.
	//-----------------------------------------------------------------------------
	static void GetFogColor(fogparams_t* pFogParams, float* pColor)
	{
		C_BasePlayer* pbp = C_BasePlayer::GetLocalPlayer();
		if (!pbp || !pFogParams)
			return;

		const char* fogColorString = fog_color.GetString();
		if (fog_override.GetInt() && fogColorString)
		{
			sscanf(fogColorString, "%f%f%f", pColor, pColor + 1, pColor + 2);
		}
		else
		{
			float flPrimaryColor[3] = { (float)pFogParams->colorPrimary.GetR(), (float)pFogParams->colorPrimary.GetG(), (float)pFogParams->colorPrimary.GetB() };
			float flSecondaryColor[3] = { (float)pFogParams->colorSecondary.GetR(), (float)pFogParams->colorSecondary.GetG(), (float)pFogParams->colorSecondary.GetB() };

			GetFogColorTransition(pFogParams, flPrimaryColor, flSecondaryColor);

			if (pFogParams->blend)
			{
				//
				// Blend between two fog colors based on viewing angle.
				// The secondary fog color is at 180 degrees to the primary fog color.
				//
				Vector forward;
				pbp->EyeVectors(&forward, NULL, NULL);

				Vector vNormalized = pFogParams->dirPrimary;
				VectorNormalize(vNormalized);
				pFogParams->dirPrimary = vNormalized;

				float flBlendFactor = 0.5 * forward.Dot(pFogParams->dirPrimary) + 0.5;

				// FIXME: convert to linear colorspace
				pColor[0] = flPrimaryColor[0] * flBlendFactor + flSecondaryColor[0] * (1 - flBlendFactor);
				pColor[1] = flPrimaryColor[1] * flBlendFactor + flSecondaryColor[1] * (1 - flBlendFactor);
				pColor[2] = flPrimaryColor[2] * flBlendFactor + flSecondaryColor[2] * (1 - flBlendFactor);
			}
			else
			{
				pColor[0] = flPrimaryColor[0];
				pColor[1] = flPrimaryColor[1];
				pColor[2] = flPrimaryColor[2];
			}
		}

		VectorScale(pColor, 1.0f / 255.0f, pColor);
	}

	static float GetFogStart(fogparams_t* pFogParams)
	{
		if (!pFogParams)
			return 0.0f;

		if (fog_override.GetInt())
		{
			if (fog_start.GetFloat() == -1.0f)
			{
				return pFogParams->start;
			}
			else
			{
				return fog_start.GetFloat();
			}
		}
		else
		{
			if (pFogParams->lerptime > gpGlobals->curtime)
			{
				if (pFogParams->start != pFogParams->startLerpTo)
				{
					if (pFogParams->lerptime > gpGlobals->curtime)
					{
						float flPercent = 1.0f - ((pFogParams->lerptime - gpGlobals->curtime) / pFogParams->duration);

						return FLerp(pFogParams->start, pFogParams->startLerpTo, flPercent);
					}
					else
					{
						if (pFogParams->start != pFogParams->startLerpTo)
						{
							pFogParams->start = pFogParams->startLerpTo;
						}
					}
				}
			}

			return pFogParams->start;
		}
	}

	static float GetFogEnd(fogparams_t* pFogParams)
	{
		if (!pFogParams)
			return 0.0f;

		if (fog_override.GetInt())
		{
			if (fog_end.GetFloat() == -1.0f)
			{
				return pFogParams->end;
			}
			else
			{
				return fog_end.GetFloat();
			}
		}
		else
		{
			if (pFogParams->lerptime > gpGlobals->curtime)
			{
				if (pFogParams->end != pFogParams->endLerpTo)
				{
					if (pFogParams->lerptime > gpGlobals->curtime)
					{
						float flPercent = 1.0f - ((pFogParams->lerptime - gpGlobals->curtime) / pFogParams->duration);

						return FLerp(pFogParams->end, pFogParams->endLerpTo, flPercent);
					}
					else
					{
						if (pFogParams->end != pFogParams->endLerpTo)
						{
							pFogParams->end = pFogParams->endLerpTo;
						}
					}
				}
			}

			return pFogParams->end;
		}
	}

	static bool GetFogEnable(fogparams_t* pFogParams)
	{
		if (cl_leveloverview.GetFloat() > 0)
			return false;

		// Ask the clientmode
		if (g_pClientMode->ShouldDrawFog() == false)
			return false;

		if (fog_override.GetInt())
		{
			if (fog_enable.GetInt())
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			if (pFogParams)
				return pFogParams->enable != false;

			return false;
		}
	}

	static float GetFogMaxDensity(fogparams_t* pFogParams)
	{
		if (!pFogParams)
			return 1.0f;

		if (cl_leveloverview.GetFloat() > 0)
			return 1.0f;

		// Ask the clientmode
		if (!g_pClientMode->ShouldDrawFog())
			return 1.0f;

		if (fog_override.GetInt())
		{
			if (fog_maxdensity.GetFloat() == -1.0f)
				return pFogParams->maxdensity;
			else
				return fog_maxdensity.GetFloat();
		}
		else
			return pFogParams->maxdensity;
	}
};

//-----------------------------------------------------------------------------
// Standard 3d skybox view
//-----------------------------------------------------------------------------
class CSkyboxView : public CRendering3dView
{
	DECLARE_CLASS(CSkyboxView, CRendering3dView);
public:
	CSkyboxView(CViewRender* pMainView) :
		CRendering3dView(pMainView),
		m_pSky3dParams(NULL)
	{
	}

	bool			Setup(const CViewSetup& view, int* pClearFlags, SkyboxVisibility_t* pSkyboxVisible);
	void			Draw();

protected:

#ifdef PORTAL
	virtual bool ShouldDrawPortals() { return false; }
#endif

	//-----------------------------------------------------------------------------
// Purpose: Returns the skybox fog color to use in rendering the current frame.
//-----------------------------------------------------------------------------
	void GetSkyboxFogColor(float* pColor)
	{
		C_BasePlayer* pbp = C_BasePlayer::GetLocalPlayer();
		if (!pbp)
		{
			return;
		}
		CPlayerLocalData* local = &pbp->m_Local;

		const char* fogColorString = fog_colorskybox.GetString();
		if (fog_override.GetInt() && fogColorString)
		{
			sscanf(fogColorString, "%f%f%f", pColor, pColor + 1, pColor + 2);
		}
		else
		{
			if (local->m_skybox3d.fog.blend)
			{
				//
				// Blend between two fog colors based on viewing angle.
				// The secondary fog color is at 180 degrees to the primary fog color.
				//
				Vector forward;
				pbp->EyeVectors(&forward, NULL, NULL);

				Vector vNormalized = local->m_skybox3d.fog.dirPrimary;
				VectorNormalize(vNormalized);
				local->m_skybox3d.fog.dirPrimary = vNormalized;

				float flBlendFactor = 0.5 * forward.Dot(local->m_skybox3d.fog.dirPrimary) + 0.5;

				// FIXME: convert to linear colorspace
				pColor[0] = local->m_skybox3d.fog.colorPrimary.GetR() * flBlendFactor + local->m_skybox3d.fog.colorSecondary.GetR() * (1 - flBlendFactor);
				pColor[1] = local->m_skybox3d.fog.colorPrimary.GetG() * flBlendFactor + local->m_skybox3d.fog.colorSecondary.GetG() * (1 - flBlendFactor);
				pColor[2] = local->m_skybox3d.fog.colorPrimary.GetB() * flBlendFactor + local->m_skybox3d.fog.colorSecondary.GetB() * (1 - flBlendFactor);
			}
			else
			{
				pColor[0] = local->m_skybox3d.fog.colorPrimary.GetR();
				pColor[1] = local->m_skybox3d.fog.colorPrimary.GetG();
				pColor[2] = local->m_skybox3d.fog.colorPrimary.GetB();
			}
		}

		VectorScale(pColor, 1.0f / 255.0f, pColor);
	}


	float GetSkyboxFogStart(void)
	{
		C_BasePlayer* pbp = C_BasePlayer::GetLocalPlayer();
		if (!pbp)
		{
			return 0.0f;
		}
		CPlayerLocalData* local = &pbp->m_Local;

		if (fog_override.GetInt())
		{
			if (fog_startskybox.GetFloat() == -1.0f)
			{
				return local->m_skybox3d.fog.start;
			}
			else
			{
				return fog_startskybox.GetFloat();
			}
		}
		else
		{
			return local->m_skybox3d.fog.start;
		}
	}

	float GetSkyboxFogEnd(void)
	{
		C_BasePlayer* pbp = C_BasePlayer::GetLocalPlayer();
		if (!pbp)
		{
			return 0.0f;
		}
		CPlayerLocalData* local = &pbp->m_Local;

		if (fog_override.GetInt())
		{
			if (fog_endskybox.GetFloat() == -1.0f)
			{
				return local->m_skybox3d.fog.end;
			}
			else
			{
				return fog_endskybox.GetFloat();
			}
		}
		else
		{
			return local->m_skybox3d.fog.end;
		}
	}


	float GetSkyboxFogMaxDensity()
	{
		C_BasePlayer* pbp = C_BasePlayer::GetLocalPlayer();
		if (!pbp)
			return 1.0f;

		CPlayerLocalData* local = &pbp->m_Local;

		if (cl_leveloverview.GetFloat() > 0)
			return 1.0f;

		// Ask the clientmode
		if (!g_pClientMode->ShouldDrawFog())
			return 1.0f;

		if (fog_override.GetInt())
		{
			if (fog_maxdensityskybox.GetFloat() == -1.0f)
				return local->m_skybox3d.fog.maxdensity;
			else
				return fog_maxdensityskybox.GetFloat();
		}
		else
			return local->m_skybox3d.fog.maxdensity;
	}

	virtual SkyboxVisibility_t	ComputeSkyboxVisibility();

	bool			GetSkyboxFogEnable();

	void			Enable3dSkyboxFog(void);
	void			DrawInternal(view_id_t iSkyBoxViewID, bool bInvokePreAndPostRender, ITexture* pRenderTarget, ITexture* pDepthTarget);

	sky3dparams_t* PreRender3dSkyboxWorld(SkyboxVisibility_t nSkyboxVisible);

	sky3dparams_t* m_pSky3dParams;


};

//-----------------------------------------------------------------------------
// 3d skybox view when drawing portals
//-----------------------------------------------------------------------------
#ifdef PORTAL
class CPortalSkyboxView : public CSkyboxView
{
	DECLARE_CLASS(CPortalSkyboxView, CSkyboxView);
public:
	CPortalSkyboxView(CViewRender* pMainView) :
		CSkyboxView(pMainView),
		m_pRenderTarget(NULL)
	{}

	bool			Setup(const CViewSetup& view, int* pClearFlags, SkyboxVisibility_t* pSkyboxVisible, ITexture* pRenderTarget = NULL);

	//Skybox drawing through portals with workarounds to fix area bits, position/scaling, view id's..........
	void			Draw();

private:
	virtual SkyboxVisibility_t	ComputeSkyboxVisibility();

	ITexture* m_pRenderTarget;
};
#endif


//-----------------------------------------------------------------------------
// Shadow depth texture
//-----------------------------------------------------------------------------
class CShadowDepthView : public CRendering3dView
{
	DECLARE_CLASS(CShadowDepthView, CRendering3dView);
public:
	CShadowDepthView(CViewRender* pMainView) : CRendering3dView(pMainView) {}

	void Setup(const CViewSetup& shadowViewIn, ITexture* pRenderTarget, ITexture* pDepthTexture);
	void Draw();

private:
	ITexture* m_pRenderTarget;
	ITexture* m_pDepthTexture;
};

//-----------------------------------------------------------------------------
// Freeze frame. Redraws the frame at which it was enabled.
//-----------------------------------------------------------------------------
class CFreezeFrameView : public CRendering3dView
{
	DECLARE_CLASS(CFreezeFrameView, CRendering3dView);
public:
	CFreezeFrameView(CViewRender* pMainView) : CRendering3dView(pMainView) {}

	void Setup(const CViewSetup& view);
	void Draw();

private:
	CMaterialReference m_pFreezeFrame;
	CMaterialReference m_TranslucentSingleColor;
	float g_flFreezeFlash = 0.0f;//nouse?
};

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
class CBaseWorldView : public CRendering3dView
{
	DECLARE_CLASS(CBaseWorldView, CRendering3dView);
protected:
	CBaseWorldView(CViewRender* pMainView) : CRendering3dView(pMainView) {}

	virtual bool	AdjustView(float waterHeight);

	void			DrawSetup(float waterHeight, int flags, float waterZAdjust, int iForceViewLeaf = -1);
	void			DrawExecute(float waterHeight, view_id_t viewID, float waterZAdjust);

	virtual void	PushView(float waterHeight);
	virtual void	PopView();

	void			SSAO_DepthPass();
	void			DrawDepthOfField();
	void			MaybeInvalidateLocalPlayerAnimation()
	{
		C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
		if ((pPlayer != NULL) && pPlayer->InFirstPersonView())
		{
			// We sometimes need different animation for the main view versus the shadow rendering,
			// so we need to reset the cache to ensure this actually happens.
			pPlayer->InvalidateBoneCache();

			C_BaseCombatWeapon* pWeapon = pPlayer->GetActiveWeapon();
			if (pWeapon != NULL)
			{
				pWeapon->InvalidateBoneCache();
			}

#if defined USES_ECON_ITEMS
			// ...and all the things you're wearing/holding/etc
			int NumWearables = pPlayer->GetNumWearables();
			for (int i = 0; i < NumWearables; ++i)
			{
				CEconWearable* pItem = pPlayer->GetWearable(i);
				if (pItem != NULL)
				{
					pItem->InvalidateBoneCache();
				}
			}
#endif // USES_ECON_ITEMS

		}
	}

	//-----------------------------------------------------------------------------
// Returns true if the view plane intersects the water
//-----------------------------------------------------------------------------
	bool DoesViewPlaneIntersectWater(float waterZ, int leafWaterDataID)
	{
		if (leafWaterDataID == -1)
			return false;

#ifdef PORTAL //when rendering portal views point/plane intersections just don't cut it.
		if (g_pPortalRender->GetViewRecursionLevel() != 0)
			return g_pPortalRender->DoesExitPortalViewIntersectWaterPlane(waterZ, leafWaterDataID);
#endif

		CMatRenderContextPtr pRenderContext(materials);

		VMatrix viewMatrix, projectionMatrix, viewProjectionMatrix, inverseViewProjectionMatrix;
		pRenderContext->GetMatrix(MATERIAL_VIEW, &viewMatrix);
		pRenderContext->GetMatrix(MATERIAL_PROJECTION, &projectionMatrix);
		MatrixMultiply(projectionMatrix, viewMatrix, viewProjectionMatrix);
		MatrixInverseGeneral(viewProjectionMatrix, inverseViewProjectionMatrix);

		Vector mins, maxs;
		ClearBounds(mins, maxs);
		Vector testPoint[4];
		testPoint[0].Init(-1.0f, -1.0f, 0.0f);
		testPoint[1].Init(-1.0f, 1.0f, 0.0f);
		testPoint[2].Init(1.0f, -1.0f, 0.0f);
		testPoint[3].Init(1.0f, 1.0f, 0.0f);
		int i;
		bool bAbove = false;
		bool bBelow = false;
		float fudge = 7.0f;
		for (i = 0; i < 4; i++)
		{
			Vector worldPos;
			Vector3DMultiplyPositionProjective(inverseViewProjectionMatrix, testPoint[i], worldPos);
			AddPointToBounds(worldPos, mins, maxs);
			//		Warning( "viewplanez: %f waterZ: %f\n", worldPos.z, waterZ );
			if (worldPos.z + fudge > waterZ)
			{
				bAbove = true;
			}
			if (worldPos.z - fudge < waterZ)
			{
				bBelow = true;
			}
		}

		// early out if the near plane doesn't cross the z plane of the water.
		if (!(bAbove && bBelow))
			return false;

		Vector vecFudge(fudge, fudge, fudge);
		mins -= vecFudge;
		maxs += vecFudge;

		// the near plane does cross the z value for the visible water volume.  Call into
		// the engine to find out if the near plane intersects the water volume.
		return render->DoesBoxIntersectWaterVolume(mins, maxs, leafWaterDataID);
	}

	//-----------------------------------------------------------------------------
// Pushes a water render target
//-----------------------------------------------------------------------------
	Vector SavedLinearLightMapScale = Vector(-1, -1, -1);			// x<0 = no saved scale

	void SetLightmapScaleForWater(void)
	{
		if (g_pMaterialSystemHardwareConfig->GetHDRType() == HDR_TYPE_INTEGER)
		{
			CMatRenderContextPtr pRenderContext(materials);
			SavedLinearLightMapScale = pRenderContext->GetToneMappingScaleLinear();
			Vector t25 = SavedLinearLightMapScale;
			t25 *= 0.25;
			pRenderContext->SetToneMappingScaleLinear(t25);
		}
	}
};


//-----------------------------------------------------------------------------
// Draws the scene when there's no water or only cheap water
//-----------------------------------------------------------------------------
class CSimpleWorldView : public CBaseWorldView
{
	DECLARE_CLASS(CSimpleWorldView, CBaseWorldView);
public:
	CSimpleWorldView(CViewRender* pMainView) : CBaseWorldView(pMainView) {}

	void			Setup(const CViewSetup& view, int nClearFlags, bool bDrawSkybox, const VisibleFogVolumeInfo_t& fogInfo, const WaterRenderInfo_t& info, ViewCustomVisibility_t* pCustomVisibility = NULL);
	void			Draw();

private:
	VisibleFogVolumeInfo_t m_fogInfo;

};


//-----------------------------------------------------------------------------
// Base class for scenes with water
//-----------------------------------------------------------------------------
class CBaseWaterView : public CBaseWorldView
{
	DECLARE_CLASS(CBaseWaterView, CBaseWorldView);
public:
	CBaseWaterView(CViewRender* pMainView) :
		CBaseWorldView(pMainView),
		m_SoftwareIntersectionView(pMainView)
	{}

	//	void Setup( const CViewSetup &, const WaterRenderInfo_t& info );

protected:
	void			CalcWaterEyeAdjustments(const VisibleFogVolumeInfo_t& fogInfo, float& newWaterHeight, float& waterZAdjust, bool bSoftwareUserClipPlane);

	class CSoftwareIntersectionView : public CBaseWorldView
	{
		DECLARE_CLASS(CSoftwareIntersectionView, CBaseWorldView);
	public:
		CSoftwareIntersectionView(CViewRender* pMainView) : CBaseWorldView(pMainView) {}

		void Setup(bool bAboveWater);
		void Draw();

	private:
		CBaseWaterView* GetOuter() { return GET_OUTER(CBaseWaterView, m_SoftwareIntersectionView); }
	};

	friend class CSoftwareIntersectionView;

	CSoftwareIntersectionView m_SoftwareIntersectionView;

	WaterRenderInfo_t m_waterInfo;
	float m_waterHeight;
	float m_waterZAdjust;
	bool m_bSoftwareUserClipPlane;
	VisibleFogVolumeInfo_t m_fogInfo;
};


//-----------------------------------------------------------------------------
// Scenes above water
//-----------------------------------------------------------------------------
class CAboveWaterView : public CBaseWaterView
{
	DECLARE_CLASS(CAboveWaterView, CBaseWaterView);
public:
	CAboveWaterView(CViewRender* pMainView) :
		CBaseWaterView(pMainView),
		m_ReflectionView(pMainView),
		m_RefractionView(pMainView),
		m_IntersectionView(pMainView)
	{}

	void Setup(const CViewSetup& view, bool bDrawSkybox, const VisibleFogVolumeInfo_t& fogInfo, const WaterRenderInfo_t& waterInfo);
	void			Draw();

	class CReflectionView : public CBaseWorldView
	{
		DECLARE_CLASS(CReflectionView, CBaseWorldView);
	public:
		CReflectionView(CViewRender* pMainView) : CBaseWorldView(pMainView) {}

		void Setup(bool bReflectEntities);
		void Draw();

	private:
		CAboveWaterView* GetOuter() { return GET_OUTER(CAboveWaterView, m_ReflectionView); }
	};

	class CRefractionView : public CBaseWorldView
	{
		DECLARE_CLASS(CRefractionView, CBaseWorldView);
	public:
		CRefractionView(CViewRender* pMainView) : CBaseWorldView(pMainView) {}

		void Setup();
		void Draw();

	private:
		CAboveWaterView* GetOuter() { return GET_OUTER(CAboveWaterView, m_RefractionView); }
	};

	class CIntersectionView : public CBaseWorldView
	{
		DECLARE_CLASS(CIntersectionView, CBaseWorldView);
	public:
		CIntersectionView(CViewRender* pMainView) : CBaseWorldView(pMainView) {}

		void Setup();
		void Draw();

	private:
		CAboveWaterView* GetOuter() { return GET_OUTER(CAboveWaterView, m_IntersectionView); }
	};


	friend class CRefractionView;
	friend class CReflectionView;
	friend class CIntersectionView;

	bool m_bViewIntersectsWater;

	CReflectionView m_ReflectionView;
	CRefractionView m_RefractionView;
	CIntersectionView m_IntersectionView;
};


//-----------------------------------------------------------------------------
// Scenes below water
//-----------------------------------------------------------------------------
class CUnderWaterView : public CBaseWaterView
{
	DECLARE_CLASS(CUnderWaterView, CBaseWaterView);
public:
	CUnderWaterView(CViewRender* pMainView) :
		CBaseWaterView(pMainView),
		m_RefractionView(pMainView)
	{}

	void			Setup(const CViewSetup& view, bool bDrawSkybox, const VisibleFogVolumeInfo_t& fogInfo, const WaterRenderInfo_t& info);
	void			Draw();

	class CRefractionView : public CBaseWorldView
	{
		DECLARE_CLASS(CRefractionView, CBaseWorldView);
	public:
		CRefractionView(CViewRender* pMainView) : CBaseWorldView(pMainView) {}

		void Setup();
		void Draw();

	private:
		CUnderWaterView* GetOuter() { return GET_OUTER(CUnderWaterView, m_RefractionView); }
	};

	friend class CRefractionView;

	bool m_bDrawSkybox; // @MULTICORE (toml 8/17/2006): remove after setup hoisted

	CRefractionView m_RefractionView;
};


//-----------------------------------------------------------------------------
// Scenes containing reflective glass
//-----------------------------------------------------------------------------
class CReflectiveGlassView : public CSimpleWorldView
{
	DECLARE_CLASS(CReflectiveGlassView, CSimpleWorldView);
public:
	CReflectiveGlassView(CViewRender* pMainView) : BaseClass(pMainView)
	{
	}

	virtual bool AdjustView(float flWaterHeight);
	virtual void PushView(float waterHeight);
	virtual void PopView();
	void Setup(const CViewSetup& view, int nClearFlags, bool bDrawSkybox, const VisibleFogVolumeInfo_t& fogInfo, const WaterRenderInfo_t& waterInfo, const cplane_t& reflectionPlane);
	void Draw();

	cplane_t m_ReflectionPlane;
};

class CRefractiveGlassView : public CSimpleWorldView
{
	DECLARE_CLASS(CRefractiveGlassView, CSimpleWorldView);
public:
	CRefractiveGlassView(CViewRender* pMainView) : BaseClass(pMainView)
	{
	}

	virtual bool AdjustView(float flWaterHeight);
	virtual void PushView(float waterHeight);
	virtual void PopView();
	void Setup(const CViewSetup& view, int nClearFlags, bool bDrawSkybox, const VisibleFogVolumeInfo_t& fogInfo, const WaterRenderInfo_t& waterInfo, const cplane_t& reflectionPlane);
	void Draw();

	cplane_t m_ReflectionPlane;
};

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

class CRenderExecutor
{
	DECLARE_CLASS_NOBASE( CRenderExecutor );
public:
	virtual void AddView( CRendering3dView *pView ) = 0;
	virtual void Execute() = 0;

protected:
	CRenderExecutor( CViewRender *pMainView ) : m_pView( pMainView ) {}
	CViewRender *m_pView;
};

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

class CSimpleRenderExecutor : public CRenderExecutor
{
	DECLARE_CLASS( CSimpleRenderExecutor, CRenderExecutor );
public:
	CSimpleRenderExecutor( CViewRender *pMainView ) : CRenderExecutor( pMainView ) {}

	void AddView( CRendering3dView *pView );
	void Execute() {}
};

//-----------------------------------------------------------------------------
// Purpose: Implements the interface to view rendering for the client .dll
//-----------------------------------------------------------------------------

class CViewRender : public IViewRender,
					public IReplayScreenshotSystem
{
	DECLARE_CLASS_NOBASE( CViewRender );
public:
	virtual void	Init( void );
	virtual void	Shutdown( void );

	const CViewSetup *GetPlayerViewSetup( ) const;

	virtual void	StartPitchDrift( void );
	virtual void	StopPitchDrift( void );

	virtual float	GetZNear();
	virtual float	GetZFar();

	virtual void	OnRenderStart();
	void			DriftPitch (void);

	//static CViewRender *	GetMainView() { return assert_cast<CViewRender *>( view ); }

	void			AddViewToScene( CRendering3dView *pView ) { m_SimpleExecutor.AddView( pView ); }
protected:
	// Sets up the view parameters for all views (left, middle and right eyes).
    void            SetUpViews();

	// Sets up the view parameters of map overview mode (cl_leveloverview)
	void			SetUpOverView();

	// generates a low-res screenshot for save games
	virtual void	WriteSaveGameScreenshotOfSize( const char *pFilename, int width, int height, bool bCreatePowerOf2Padded = false, bool bWriteVTF = false );
	void			WriteSaveGameScreenshot( const char *filename );

	virtual IReplayScreenshotSystem *GetReplayScreenshotSystem() { return this; }

	// IReplayScreenshot implementation
	virtual void	WriteReplayScreenshot( WriteReplayScreenshotParams_t &params );
	virtual void	UpdateReplayScreenshotCache();

    StereoEye_t		GetFirstEye() const;
    StereoEye_t		GetLastEye() const;
    CViewSetup &    GetView(StereoEye_t eEye);
    const CViewSetup &    GetView(StereoEye_t eEye) const ;


	// This stores all of the view setup parameters that the engine needs to know about.
    // Best way to pick the right one is with ::GetView(), rather than directly.
	CViewSetup		m_View;         // mono <- in stereo mode, this will be between the two eyes and is the "main" view.
	CViewSetup		m_ViewLeft;     // left (unused for mono)
	CViewSetup		m_ViewRight;    // right (unused for mono)

	// Pitch drifting data
	CPitchDrift		m_PitchDrift;

public:
					CViewRender();
	virtual			~CViewRender( void ) {}

// Implementation of IViewRender interface
public:

	void			SetupVis( const CViewSetup& view, unsigned int &visFlags, ViewCustomVisibility_t *pCustomVisibility = NULL );


	// Render functions
	virtual	void	RenderRect( vrect_t *rect );
	virtual void	RenderView( const CViewSetup &view, int nClearFlags, int whatToDraw );
	virtual void	RenderPlayerSprites();
	virtual void	Render2DEffectsPreHUD( const CViewSetup &view );
	virtual void	Render2DEffectsPostHUD( const CViewSetup &view );


	void			DisableFog( void );

	// Called once per level change
	void			LevelInit( void );
	void			LevelShutdown( void );

	// Add entity to transparent entity queue

	bool			ShouldDrawEntities( void );
	bool			ShouldDrawBrushModels( void );

	const CViewSetup *GetViewSetup( ) const;
	
	void			DisableVis( void );

	// Sets up the view model position relative to the local player
	void			MoveViewModels( );

	// Gets the abs origin + angles of the view models
	void			GetViewModelPosition( int nIndex, Vector *pPos, QAngle *pAngle );

	void			SetCheapWaterStartDistance( float flCheapWaterStartDistance );
	void			SetCheapWaterEndDistance( float flCheapWaterEndDistance );

	void			GetWaterLODParams( float &flCheapWaterStartDistance, float &flCheapWaterEndDistance );

	virtual void	QueueOverlayRenderView( const CViewSetup &view, int nClearFlags, int whatToDraw );

	virtual void	GetScreenFadeDistances( float *min, float *max );

	virtual C_BaseEntity *GetCurrentlyDrawingEntity();
	virtual void		  SetCurrentlyDrawingEntity( C_BaseEntity *pEnt );

	virtual bool		UpdateShadowDepthTexture( ITexture *pRenderTarget, ITexture *pDepthTexture, const CViewSetup &shadowView );

	int				GetBaseDrawFlags() { return m_BaseDrawFlags; }
	virtual bool	ShouldForceNoVis()  { return m_bForceNoVis; }
	int				BuildRenderablesListsNumber() const { return m_BuildRenderableListsNumber; }
	int				IncRenderablesListsNumber()  { return ++m_BuildRenderableListsNumber; }

	int				BuildWorldListsNumber() const;
	int				IncWorldListsNumber() { return ++m_BuildWorldListsNumber; }

	virtual VPlane*	GetFrustum() { return ( m_pActiveRenderer ) ? m_pActiveRenderer->GetFrustum() : m_Frustum; }

	// What are we currently rendering? Returns a combination of DF_ flags.
	virtual int		GetDrawFlags() { return ( m_pActiveRenderer ) ? m_pActiveRenderer->GetDrawFlags() : 0; }

	CBase3dView *	GetActiveRenderer() { return m_pActiveRenderer; }
	CBase3dView *	SetActiveRenderer( CBase3dView *pActiveRenderer ) { CBase3dView *pPrevious = m_pActiveRenderer; m_pActiveRenderer =  pActiveRenderer; return pPrevious; }

	void			FreezeFrame( float flFreezeTime );

	void SetWaterOverlayMaterial( IMaterial *pMaterial )
	{
		m_UnderWaterOverlayMaterial.Init( pMaterial );
	}

	//-----------------------------------------------------------------------------
// Accessors to return the main view (where the player's looking)
//-----------------------------------------------------------------------------
	virtual const Vector& MainViewOrigin()
	{
		return g_vecRenderOrigin;
	}

	virtual const QAngle& MainViewAngles()
	{
		return g_vecRenderAngles;
	}

	virtual const Vector& MainViewForward()
	{
		return g_vecVForward;
	}

	virtual const Vector& MainViewRight()
	{
		return g_vecVRight;
	}

	virtual const Vector& MainViewUp()
	{
		return g_vecVUp;
	}

	virtual const VMatrix& MainWorldToViewMatrix()
	{
		return g_matCamInverse;
	}

	virtual const Vector& PrevMainViewOrigin()
	{
		return g_vecPrevRenderOrigin;
	}

	virtual const QAngle& PrevMainViewAngles()
	{
		return g_vecPrevRenderAngles;
	}

	//-----------------------------------------------------------------------------
	// Accessors to return the current view being rendered
	//-----------------------------------------------------------------------------
	virtual const Vector& CurrentViewOrigin()
	{
		Assert(s_bCanAccessCurrentView);
		return g_vecCurrentRenderOrigin;
	}

	virtual const QAngle& CurrentViewAngles()
	{
		Assert(s_bCanAccessCurrentView);
		return g_vecCurrentRenderAngles;
	}

	virtual const Vector& CurrentViewForward()
	{
		Assert(s_bCanAccessCurrentView);
		return g_vecCurrentVForward;
	}

	virtual const Vector& CurrentViewRight()
	{
		Assert(s_bCanAccessCurrentView);
		return g_vecCurrentVRight;
	}

	virtual const Vector& CurrentViewUp()
	{
		Assert(s_bCanAccessCurrentView);
		return g_vecCurrentVUp;
	}

	virtual const VMatrix& CurrentWorldToViewMatrix()
	{
		Assert(s_bCanAccessCurrentView);
		return g_matCurrentCamInverse;
	}

	//-----------------------------------------------------------------------------
	// Methods to set the current view/guard access to view parameters
	//-----------------------------------------------------------------------------
	virtual void AllowCurrentViewAccess(bool allow)
	{
		s_bCanAccessCurrentView = allow;
	}

	virtual bool IsCurrentViewAccessAllowed()
	{
		return s_bCanAccessCurrentView;
	}

	virtual void SetupCurrentView(view_id_t viewID)
	{
		tmZone(TELEMETRY_LEVEL0, TMZF_NONE, "%s", __FUNCTION__);

		g_CurrentViewID = viewID;
	}

	virtual void SetupCurrentView(const Vector& vecOrigin, const QAngle& angles, view_id_t viewID)
	{
		tmZone(TELEMETRY_LEVEL0, TMZF_NONE, "%s", __FUNCTION__);

		// Store off view origin and angles
		g_vecCurrentRenderOrigin = vecOrigin;
		g_vecCurrentRenderAngles = angles;

		// Compute the world->main camera transform
		ComputeCameraVariables(vecOrigin, angles,
			&g_vecCurrentVForward, &g_vecCurrentVRight, &g_vecCurrentVUp, &g_matCurrentCamInverse);

		g_CurrentViewID = viewID;
		s_bCanAccessCurrentView = true;

		// Cache off fade distances
		float flScreenFadeMinSize, flScreenFadeMaxSize;
		this->GetScreenFadeDistances(&flScreenFadeMinSize, &flScreenFadeMaxSize);
		render->SetViewScreenFadeRange(flScreenFadeMinSize, flScreenFadeMaxSize);

		CMatRenderContextPtr pRenderContext(materials);
#ifdef PORTAL
		if (g_pPortalRender->GetViewRecursionLevel() == 0)
		{
			pRenderContext->SetIntRenderingParameter(INT_RENDERPARM_WRITE_DEPTH_TO_DESTALPHA, ((viewID == VIEW_MAIN) || (viewID == VIEW_3DSKY)) ? 1 : 0);
		}
#else
		pRenderContext->SetIntRenderingParameter(INT_RENDERPARM_WRITE_DEPTH_TO_DESTALPHA, ((viewID == VIEW_MAIN) || (viewID == VIEW_3DSKY)) ? 1 : 0);
#endif
	}

	virtual void FinishCurrentView()
	{
		s_bCanAccessCurrentView = false;
	}

	virtual view_id_t CurrentViewID()
	{
		Assert(g_CurrentViewID != VIEW_ILLEGAL);
		return (view_id_t)g_CurrentViewID;
	}

	//-----------------------------------------------------------------------------
// Purpose: Portal views are considered 'Main' views. This function tests a view id 
//			against all view ids used by portal renderables, as well as the main view.
//-----------------------------------------------------------------------------
	virtual bool IsMainView(view_id_t id)
	{
#if defined(PORTAL)
		return ((id == VIEW_MAIN) || g_pPortalRender->IsPortalViewID(id));
#else
		return (id == VIEW_MAIN);
#endif
	}

	virtual bool DrawingShadowDepthView(void) //for easy externing
	{
		return (this->CurrentViewID() == VIEW_SHADOW_DEPTH_TEXTURE);
	}

	virtual bool DrawingMainView() //for easy externing
	{
		return (this->CurrentViewID() == VIEW_MAIN);
	}

	virtual bool IsRenderingScreenshot()
	{
		return g_bRenderingScreenshot;
	}

	virtual IntroData_t* GetIntroData()
	{
		return g_pIntroData;
	}

	virtual void SetIntroData(IntroData_t* introData) {
		g_pIntroData = introData;
	}

	//-----------------------------------------------------------------------------
// Fakes per-entity clip planes on cards that don't support user clip planes.
//  Achieves the effect by drawing an invisible box that writes to the depth buffer
//  around the clipped area. It's not perfect, but better than nothing.
//-----------------------------------------------------------------------------
	virtual void DrawClippedDepthBox(IClientRenderable* pEnt, float* pClipPlane)
	{
		//#define DEBUG_DRAWCLIPPEDDEPTHBOX //uncomment to draw the depth box as a colorful box

		static const int iQuads[6][5] = { { 0, 4, 6, 2, 0 }, //always an extra copy of first index at end to make some algorithms simpler
											{ 3, 7, 5, 1, 3 },
											{ 1, 5, 4, 0, 1 },
											{ 2, 6, 7, 3, 2 },
											{ 0, 2, 3, 1, 0 },
											{ 5, 7, 6, 4, 5 } };

		static const int iLines[12][2] = { { 0, 1 },
											{ 0, 2 },
											{ 0, 4 },
											{ 1, 3 },
											{ 1, 5 },
											{ 2, 3 },
											{ 2, 6 },
											{ 3, 7 },
											{ 4, 6 },
											{ 4, 5 },
											{ 5, 7 },
											{ 6, 7 } };


#ifdef DEBUG_DRAWCLIPPEDDEPTHBOX
		static const float fColors[6][3] = { { 1.0f, 0.0f, 0.0f },
												{ 0.0f, 1.0f, 1.0f },
												{ 0.0f, 1.0f, 0.0f },
												{ 1.0f, 0.0f, 1.0f },
												{ 0.0f, 0.0f, 1.0f },
												{ 1.0f, 1.0f, 0.0f } };
#endif

		Vector vNormal = *(Vector*)pClipPlane;
		float fPlaneDist = pClipPlane[3];

		Vector vMins, vMaxs;
		pEnt->GetRenderBounds(vMins, vMaxs);

		Vector vOrigin = pEnt->GetRenderOrigin();
		QAngle qAngles = pEnt->GetRenderAngles();

		Vector vForward, vUp, vRight;
		AngleVectors(qAngles, &vForward, &vRight, &vUp);

		Vector vPoints[8];
		vPoints[0] = vOrigin + (vForward * vMins.x) + (vRight * vMins.y) + (vUp * vMins.z);
		vPoints[1] = vOrigin + (vForward * vMaxs.x) + (vRight * vMins.y) + (vUp * vMins.z);
		vPoints[2] = vOrigin + (vForward * vMins.x) + (vRight * vMaxs.y) + (vUp * vMins.z);
		vPoints[3] = vOrigin + (vForward * vMaxs.x) + (vRight * vMaxs.y) + (vUp * vMins.z);
		vPoints[4] = vOrigin + (vForward * vMins.x) + (vRight * vMins.y) + (vUp * vMaxs.z);
		vPoints[5] = vOrigin + (vForward * vMaxs.x) + (vRight * vMins.y) + (vUp * vMaxs.z);
		vPoints[6] = vOrigin + (vForward * vMins.x) + (vRight * vMaxs.y) + (vUp * vMaxs.z);
		vPoints[7] = vOrigin + (vForward * vMaxs.x) + (vRight * vMaxs.y) + (vUp * vMaxs.z);

		int iClipped[8];
		float fDists[8];
		for (int i = 0; i != 8; ++i)
		{
			fDists[i] = vPoints[i].Dot(vNormal) - fPlaneDist;
			iClipped[i] = (fDists[i] > 0.0f) ? 1 : 0;
		}

		Vector vSplitPoints[8][8]; //obviously there are only 12 lines, not 64 lines or 64 split points, but the indexing is way easier like this
		int iLineStates[8][8]; //0 = unclipped, 2 = wholly clipped, 3 = first point clipped, 4 = second point clipped

		//categorize lines and generate split points where needed
		for (int i = 0; i != 12; ++i)
		{
			const int* pPoints = iLines[i];
			int iLineState = (iClipped[pPoints[0]] + iClipped[pPoints[1]]);
			if (iLineState != 1) //either both points are clipped, or neither are clipped
			{
				iLineStates[pPoints[0]][pPoints[1]] =
					iLineStates[pPoints[1]][pPoints[0]] =
					iLineState;
			}
			else
			{
				//one point is clipped, the other is not
				if (iClipped[pPoints[0]] == 1)
				{
					//first point was clipped, index 1 has the negative distance
					float fInvTotalDist = 1.0f / (fDists[pPoints[0]] - fDists[pPoints[1]]);
					vSplitPoints[pPoints[0]][pPoints[1]] =
						vSplitPoints[pPoints[1]][pPoints[0]] =
						(vPoints[pPoints[1]] * (fDists[pPoints[0]] * fInvTotalDist)) - (vPoints[pPoints[0]] * (fDists[pPoints[1]] * fInvTotalDist));

					Assert(fabs(vNormal.Dot(vSplitPoints[pPoints[0]][pPoints[1]]) - fPlaneDist) < 0.01f);

					iLineStates[pPoints[0]][pPoints[1]] = 3;
					iLineStates[pPoints[1]][pPoints[0]] = 4;
				}
				else
				{
					//second point was clipped, index 0 has the negative distance
					float fInvTotalDist = 1.0f / (fDists[pPoints[1]] - fDists[pPoints[0]]);
					vSplitPoints[pPoints[0]][pPoints[1]] =
						vSplitPoints[pPoints[1]][pPoints[0]] =
						(vPoints[pPoints[0]] * (fDists[pPoints[1]] * fInvTotalDist)) - (vPoints[pPoints[1]] * (fDists[pPoints[0]] * fInvTotalDist));

					Assert(fabs(vNormal.Dot(vSplitPoints[pPoints[0]][pPoints[1]]) - fPlaneDist) < 0.01f);

					iLineStates[pPoints[0]][pPoints[1]] = 4;
					iLineStates[pPoints[1]][pPoints[0]] = 3;
				}
			}
		}


		CMatRenderContextPtr pRenderContext(materials);

#ifdef DEBUG_DRAWCLIPPEDDEPTHBOX
		pRenderContext->Bind(materials->FindMaterial("debug/debugvertexcolor", TEXTURE_GROUP_OTHER), NULL);
#else
		pRenderContext->Bind(g_material_WriteZ, NULL);
#endif

		CMeshBuilder meshBuilder;
		IMesh* pMesh = pRenderContext->GetDynamicMesh(false);
		meshBuilder.Begin(pMesh, MATERIAL_TRIANGLES, 18); //6 sides, possible one cut per side. Any side is capable of having 3 tri's. Lots of padding for things that aren't possible

		//going to draw as a collection of triangles, arranged as a triangle fan on each side
		for (int i = 0; i != 6; ++i)
		{
			const int* pPoints = iQuads[i];

			//can't start the fan on a wholly clipped line, so seek to one that isn't
			int j = 0;
			do
			{
				if (iLineStates[pPoints[j]][pPoints[j + 1]] != 2) //at least part of this line will be drawn
					break;

				++j;
			} while (j != 3);

			if (j == 3) //not enough lines to even form a triangle
				continue;

			float* pStartPoint = 0;
			float* pTriangleFanPoints[4]; //at most, one of our fans will have 5 points total, with the first point being stored separately as pStartPoint
			int iTriangleFanPointCount = 1; //the switch below creates the first for sure

			//figure out how to start the fan
			switch (iLineStates[pPoints[j]][pPoints[j + 1]])
			{
			case 0: //uncut
				pStartPoint = &vPoints[pPoints[j]].x;
				pTriangleFanPoints[0] = &vPoints[pPoints[j + 1]].x;
				break;

			case 4: //second index was clipped
				pStartPoint = &vPoints[pPoints[j]].x;
				pTriangleFanPoints[0] = &vSplitPoints[pPoints[j]][pPoints[j + 1]].x;
				break;

			case 3: //first index was clipped
				pStartPoint = &vSplitPoints[pPoints[j]][pPoints[j + 1]].x;
				pTriangleFanPoints[0] = &vPoints[pPoints[j + 1]].x;
				break;

			default:
				Assert(false);
				break;
			};

			for (++j; j != 3; ++j) //add end points for the rest of the indices, we're assembling a triangle fan
			{
				switch (iLineStates[pPoints[j]][pPoints[j + 1]])
				{
				case 0: //uncut line, normal endpoint
					pTriangleFanPoints[iTriangleFanPointCount] = &vPoints[pPoints[j + 1]].x;
					++iTriangleFanPointCount;
					break;

				case 2: //wholly cut line, no endpoint
					break;

				case 3: //first point is clipped, normal endpoint
					//special case, adds start and end point
					pTriangleFanPoints[iTriangleFanPointCount] = &vSplitPoints[pPoints[j]][pPoints[j + 1]].x;
					++iTriangleFanPointCount;

					pTriangleFanPoints[iTriangleFanPointCount] = &vPoints[pPoints[j + 1]].x;
					++iTriangleFanPointCount;
					break;

				case 4: //second point is clipped
					pTriangleFanPoints[iTriangleFanPointCount] = &vSplitPoints[pPoints[j]][pPoints[j + 1]].x;
					++iTriangleFanPointCount;
					break;

				default:
					Assert(false);
					break;
				};
			}

			//special case endpoints, half-clipped lines have a connecting line between them and the next line (first line in this case)
			switch (iLineStates[pPoints[j]][pPoints[j + 1]])
			{
			case 3:
			case 4:
				pTriangleFanPoints[iTriangleFanPointCount] = &vSplitPoints[pPoints[j]][pPoints[j + 1]].x;
				++iTriangleFanPointCount;
				break;
			};

			Assert(iTriangleFanPointCount <= 4);

			//add the fan to the mesh
			int iLoopStop = iTriangleFanPointCount - 1;
			for (int k = 0; k != iLoopStop; ++k)
			{
				meshBuilder.Position3fv(pStartPoint);
#ifdef DEBUG_DRAWCLIPPEDDEPTHBOX
				float fHalfColors[3] = { fColors[i][0] * 0.5f, fColors[i][1] * 0.5f, fColors[i][2] * 0.5f };
				meshBuilder.Color3fv(fHalfColors);
#endif
				meshBuilder.AdvanceVertex();

				meshBuilder.Position3fv(pTriangleFanPoints[k]);
#ifdef DEBUG_DRAWCLIPPEDDEPTHBOX
				meshBuilder.Color3fv(fColors[i]);
#endif
				meshBuilder.AdvanceVertex();

				meshBuilder.Position3fv(pTriangleFanPoints[k + 1]);
#ifdef DEBUG_DRAWCLIPPEDDEPTHBOX
				meshBuilder.Color3fv(fColors[i]);
#endif
				meshBuilder.AdvanceVertex();
			}
		}

		meshBuilder.End();
		pMesh->Draw();
		pRenderContext->Flush(false);
	}
	//-----------------------------------------------------------------------------
// Draws all opaque renderables in leaves that were rendered
//-----------------------------------------------------------------------------
	virtual inline void DrawOpaqueRenderable(IClientRenderable* pEnt, bool bTwoPass, ERenderDepthMode DepthMode, int nDefaultFlags = 0)
	{
		tmZone(TELEMETRY_LEVEL0, TMZF_NONE, "%s", __FUNCTION__);

		float color[3];

		pEnt->GetColorModulation(color);
		render->SetColorModulation(color);

		int flags = nDefaultFlags | STUDIO_RENDER;
		if (bTwoPass)
		{
			flags |= STUDIO_TWOPASS;
		}

		if (DepthMode == DEPTH_MODE_SHADOW)
		{
			flags |= STUDIO_SHADOWDEPTHTEXTURE;
		}
		else if (DepthMode == DEPTH_MODE_SSA0)
		{
			flags |= STUDIO_SSAODEPTHTEXTURE;
		}

		float* pRenderClipPlane = NULL;
		if (r_entityclips.GetBool())
			pRenderClipPlane = pEnt->GetRenderClipPlane();

		if (pRenderClipPlane)
		{
			CMatRenderContextPtr pRenderContext(materials);
			if (!materials->UsingFastClipping()) //do NOT change the fast clip plane mid-scene, depth problems result. Regular user clip planes are fine though
				pRenderContext->PushCustomClipPlane(pRenderClipPlane);
			else
				DrawClippedDepthBox(pEnt, pRenderClipPlane);
			Assert(view->GetCurrentlyDrawingEntity() == NULL);
			this->SetCurrentlyDrawingEntity(pEnt->GetIClientUnknown()->GetBaseEntity());
			pEnt->DrawModel(flags);
			this->SetCurrentlyDrawingEntity(NULL);
			if (pRenderClipPlane && !materials->UsingFastClipping())
				pRenderContext->PopCustomClipPlane();
		}
		else
		{
			Assert(view->GetCurrentlyDrawingEntity() == NULL);
			this->SetCurrentlyDrawingEntity(pEnt->GetIClientUnknown()->GetBaseEntity());
			pEnt->DrawModel(flags);
			this->SetCurrentlyDrawingEntity(NULL);
		}
	}
	//-----------------------------------------------------------------------------
// Renders all translucent entities in the render list
//-----------------------------------------------------------------------------
	virtual inline void DrawTranslucentRenderable(IClientRenderable* pEnt, bool twoPass, bool bShadowDepth, bool bIgnoreDepth)
	{
		// Determine blending amount and tell engine
		float blend = (float)(pEnt->GetFxBlend() / 255.0f);

		// Totally gone
		if (blend <= 0.0f)
			return;

		if (pEnt->IgnoresZBuffer() != bIgnoreDepth)
			return;

		// Tell engine
		render->SetBlend(blend);

		float color[3];
		pEnt->GetColorModulation(color);
		render->SetColorModulation(color);

		int flags = STUDIO_RENDER | STUDIO_TRANSPARENCY;
		if (twoPass)
			flags |= STUDIO_TWOPASS;

		if (bShadowDepth)
			flags |= STUDIO_SHADOWDEPTHTEXTURE;

		float* pRenderClipPlane = NULL;
		if (r_entityclips.GetBool())
			pRenderClipPlane = pEnt->GetRenderClipPlane();

		if (pRenderClipPlane)
		{
			CMatRenderContextPtr pRenderContext(materials);
			if (!materials->UsingFastClipping()) //do NOT change the fast clip plane mid-scene, depth problems result. Regular user clip planes are fine though
				pRenderContext->PushCustomClipPlane(pRenderClipPlane);
			else
				DrawClippedDepthBox(pEnt, pRenderClipPlane);
			Assert(view->GetCurrentlyDrawingEntity() == NULL);
			this->SetCurrentlyDrawingEntity(pEnt->GetIClientUnknown()->GetBaseEntity());
			pEnt->DrawModel(flags);
			this->SetCurrentlyDrawingEntity(NULL);

			if (pRenderClipPlane && !materials->UsingFastClipping())
				pRenderContext->PopCustomClipPlane();
		}
		else
		{
			Assert(view->GetCurrentlyDrawingEntity() == NULL);
			this->SetCurrentlyDrawingEntity(pEnt->GetIClientUnknown()->GetBaseEntity());
			pEnt->DrawModel(flags);
			this->SetCurrentlyDrawingEntity(NULL);
		}
	}

	virtual void DrawOpaqueRenderables_Range(CClientRenderablesList::CEntry* pEntitiesBegin, CClientRenderablesList::CEntry* pEntitiesEnd, ERenderDepthMode DepthMode)
	{
		for (CClientRenderablesList::CEntry* itEntity = pEntitiesBegin; itEntity < pEntitiesEnd; ++itEntity)
		{
			if (itEntity->m_pRenderable)
				DrawOpaqueRenderable(itEntity->m_pRenderable, (itEntity->m_TwoPass != 0), DepthMode);
		}
	}

	virtual void DrawOpaqueRenderables_DrawStaticProps(CClientRenderablesList::CEntry* pEntitiesBegin, CClientRenderablesList::CEntry* pEntitiesEnd, ERenderDepthMode DepthMode)
	{
		if (pEntitiesEnd == pEntitiesBegin)
			return;

		float one[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		render->SetColorModulation(one);
		render->SetBlend(1.0f);

		const int MAX_STATICS_PER_BATCH = 512;
		IClientRenderable* pStatics[MAX_STATICS_PER_BATCH];

		int numScheduled = 0, numAvailable = MAX_STATICS_PER_BATCH;

		for (CClientRenderablesList::CEntry* itEntity = pEntitiesBegin; itEntity < pEntitiesEnd; ++itEntity)
		{
			if (itEntity->m_pRenderable)
				/**/;
			else
				continue;

			if (g_pStudioStatsEntity != NULL && this->CurrentViewID() == VIEW_MAIN && itEntity->m_pRenderable == g_pStudioStatsEntity)
			{
				DrawOpaqueRenderable(itEntity->m_pRenderable, false, DepthMode, STUDIO_GENERATE_STATS);
				continue;
			}

			pStatics[numScheduled++] = itEntity->m_pRenderable;
			if (--numAvailable > 0)
				continue; // place a hint for compiler to predict more common case in the loop

			staticpropmgr->DrawStaticProps(pStatics, numScheduled, DepthMode, vcollide_wireframe.GetBool());
			numScheduled = 0;
			numAvailable = MAX_STATICS_PER_BATCH;
		}

		if (numScheduled)
			staticpropmgr->DrawStaticProps(pStatics, numScheduled, DepthMode, vcollide_wireframe.GetBool());
	}

	virtual void DrawOpaqueRenderables_DrawBrushModels(CClientRenderablesList::CEntry* pEntitiesBegin, CClientRenderablesList::CEntry* pEntitiesEnd, ERenderDepthMode DepthMode)
	{
		for (CClientRenderablesList::CEntry* itEntity = pEntitiesBegin; itEntity < pEntitiesEnd; ++itEntity)
		{
			Assert(!itEntity->m_TwoPass);
			DrawOpaqueRenderable(itEntity->m_pRenderable, false, DepthMode);
		}
	}

	//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
	virtual void SetClearColorToFogColor()
	{
		unsigned char ucFogColor[3];
		CMatRenderContextPtr pRenderContext(materials);
		pRenderContext->GetFogColor(ucFogColor);
		if (g_pMaterialSystemHardwareConfig->GetHDRType() == HDR_TYPE_INTEGER)
		{
			// @MULTICORE (toml 8/16/2006): Find a way to not do this twice in eye above water case
			float scale = LinearToGammaFullRange(pRenderContext->GetToneMappingScaleLinear().x);
			ucFogColor[0] *= scale;
			ucFogColor[1] *= scale;
			ucFogColor[2] *= scale;
		}
		pRenderContext->ClearColor4ub(ucFogColor[0], ucFogColor[1], ucFogColor[2], 255);
	}

private:

	

	int				m_BuildWorldListsNumber;


	// General draw methods
	// baseDrawFlags is a combination of DF_ defines. DF_MONITOR is passed into here while drawing a monitor.
	void			ViewDrawScene( bool bDrew3dSkybox, SkyboxVisibility_t nSkyboxVisible, const CViewSetup &view, int nClearFlags, view_id_t viewID, bool bDrawViewModel = false, int baseDrawFlags = 0, ViewCustomVisibility_t *pCustomVisibility = NULL );

	void			DrawMonitors( const CViewSetup &cameraView );

	bool			DrawOneMonitor( ITexture *pRenderTarget, int cameraNum, C_PointCamera *pCameraEnt, const CViewSetup &cameraView, C_BasePlayer *localPlayer, 
						int x, int y, int width, int height );

	// Drawing primitives
	bool			ShouldDrawViewModel( bool drawViewmodel );
	void			DrawViewModels( const CViewSetup &view, bool drawViewmodel );

	void			PerformScreenSpaceEffects( int x, int y, int w, int h );

	// Overlays
	void			SetScreenOverlayMaterial( IMaterial *pMaterial );
	IMaterial		*GetScreenOverlayMaterial( );
	void			PerformScreenOverlay( int x, int y, int w, int h );

	void DrawUnderwaterOverlay( void );

	// Water-related methods
	void			DrawWorldAndEntities( bool drawSkybox, const CViewSetup &view, int nClearFlags, ViewCustomVisibility_t *pCustomVisibility = NULL );

	virtual void			ViewDrawScene_Intro( const CViewSetup &view, int nClearFlags, const IntroData_t &introData );

#ifdef PORTAL 
	// Intended for use in the middle of another ViewDrawScene call, this allows stencils to be drawn after opaques but before translucents are drawn in the main view.
	void			ViewDrawScene_PortalStencil( const CViewSetup &view, ViewCustomVisibility_t *pCustomVisibility );
	void			Draw3dSkyboxworld_Portal( const CViewSetup &view, int &nClearFlags, bool &bDrew3dSkybox, SkyboxVisibility_t &nSkyboxVisible, ITexture *pRenderTarget = NULL );
#endif // PORTAL

	// Determines what kind of water we're going to use
	void			DetermineWaterRenderInfo( const VisibleFogVolumeInfo_t &fogVolumeInfo, WaterRenderInfo_t &info );

	bool			UpdateRefractIfNeededByList( CUtlVector< IClientRenderable * > &list );
	void			DrawRenderablesInList( CUtlVector< IClientRenderable * > &list, int flags = 0 );

	// Sets up, cleans up the main 3D view
	void			SetupMain3DView( const CViewSetup &view, int &nClearFlags );
	void			CleanupMain3DView( const CViewSetup &view );


	// This stores the current view
 	CViewSetup		m_CurrentView;

	// VIS Overrides
	// Set to true to turn off client side vis ( !!!! rendering will be slow since everything will draw )
	bool			m_bForceNoVis;	

	// Some cvars needed by this system
	const ConVar	*m_pDrawEntities;
	const ConVar	*m_pDrawBrushModels;

	// Some materials used...
	CMaterialReference	m_TranslucentSingleColor;
	CMaterialReference	m_ModulateSingleColor;
	CMaterialReference	m_ScreenOverlayMaterial;
	CMaterialReference m_UnderWaterOverlayMaterial;

	Vector			m_vecLastFacing;
	float			m_flCheapWaterStartDistance;
	float			m_flCheapWaterEndDistance;

	CViewSetup			m_OverlayViewSetup;
	int					m_OverlayClearFlags;
	int					m_OverlayDrawFlags;
	bool				m_bDrawOverlay;

	int					m_BaseDrawFlags;	// Set in ViewDrawScene and OR'd into m_DrawFlags as it goes.
	C_BaseEntity		*m_pCurrentlyDrawingEntity;

#if defined( CSTRIKE_DLL )
	float				m_flLastFOV;
#endif

#ifdef PORTAL
	friend class CPortalRender; //portal drawing needs muck with views in weird ways
	friend class CPortalRenderable;
#endif
	int				m_BuildRenderableListsNumber;

	friend class CBase3dView;
	friend class CRendering3dView;

	Frustum m_Frustum;

	CBase3dView *m_pActiveRenderer;
	CSimpleRenderExecutor m_SimpleExecutor;

	bool			m_rbTakeFreezeFrame[ STEREO_EYE_MAX ];
	float			m_flFreezeFrameUntil;

#if defined( REPLAY_ENABLED )
	CReplayScreenshotTaker	*m_pReplayScreenshotTaker;
#endif

	// These are the vectors for the "main" view - the one the player is looking down.
// For stereo views, they are the vectors for the middle eye.
	Vector g_vecRenderOrigin = Vector(0, 0, 0);
	QAngle g_vecRenderAngles = QAngle(0, 0, 0);
	Vector g_vecPrevRenderOrigin = Vector(0, 0, 0);	// Last frame's render origin
	QAngle g_vecPrevRenderAngles = QAngle(0, 0, 0); // Last frame's render angles
	Vector g_vecVForward = Vector(0, 0, 0), g_vecVRight = Vector(0, 0, 0), g_vecVUp = Vector(0, 0, 0);
	VMatrix g_matCamInverse;
	Vector g_vecCurrentRenderOrigin = Vector(0, 0, 0);
	QAngle g_vecCurrentRenderAngles = QAngle(0, 0, 0);
	Vector g_vecCurrentVForward = Vector(0, 0, 0), g_vecCurrentVRight = Vector(0, 0, 0), g_vecCurrentVUp = Vector(0, 0, 0);
	VMatrix g_matCurrentCamInverse;
	bool s_bCanAccessCurrentView = false;

	IntroData_t* g_pIntroData = NULL;
	bool	g_bRenderingView = false;			// For debugging...
	int g_CurrentViewID = VIEW_NONE;
	bool g_bRenderingScreenshot = false;
	CMaterialReference g_material_WriteZ; //init'ed on by CViewRender::Init()
#ifdef DBGFLAG_ASSERT
	Vector s_DbgSetupOrigin;
	QAngle s_DbgSetupAngles;
#endif
	Vector s_DemoView;
	QAngle s_DemoAngle;

	void CalcDemoViewOverride(Vector& origin, QAngle& angles)
	{
		engineClient->SetViewAngles(s_DemoAngle);

		input->ExtraMouseSample(gpGlobals->absoluteframetime, true);

		engineClient->GetViewAngles(s_DemoAngle);

		Vector forward, right, up;

		AngleVectors(s_DemoAngle, &forward, &right, &up);

		float speed = gpGlobals->absoluteframetime * cl_demoviewoverride.GetFloat() * 320;

		s_DemoView += speed * input->KeyState(&in_forward) * forward;
		s_DemoView -= speed * input->KeyState(&in_back) * forward;

		s_DemoView += speed * input->KeyState(&in_moveright) * right;
		s_DemoView -= speed * input->KeyState(&in_moveleft) * right;

		origin = s_DemoView;
		angles = s_DemoAngle;
	}

	
};

#endif // VIEWRENDER_H
