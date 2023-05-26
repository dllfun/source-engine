//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Model loading / unloading interface
//
// $NoKeywords: $
//=============================================================================//

#ifndef OVERLAY_H
#define OVERLAY_H

#ifdef _WIN32
#pragma once
#endif
#include "gl_model_private.h"

// This is a workaround for the fact that we get massive decal flicker
// when looking at a decal at a glancing angle while standing right next to it.
#define OVERLAY_AVOID_FLICKER_NORMAL_OFFSET	0.1f

struct worldbrushdata_t;

//-----------------------------------------------------------------------------
// Overlay fragments
//-----------------------------------------------------------------------------
typedef unsigned short OverlayFragmentHandle_t;

enum
{
	OVERLAY_FRAGMENT_INVALID = (OverlayFragmentHandle_t)~0
};


//=============================================================================
//
// Overlay Manager Interface
//
class IOverlayMgr
{
public:
	// Memory allocation/de-allocation.
	virtual bool	LoadOverlays(CLumpHeaderInfo& header, model_t* pModel) = 0;
	virtual void	UnloadOverlays( ) = 0;

	virtual void	CreateFragments(model_t* pWorld) = 0;

	virtual void	ReSortMaterials(model_t* pWorld) = 0;

	// Drawing
	// clears all
	virtual void	ClearRenderLists() = 0;
	// clears a particular sort group
	virtual void	ClearRenderLists( int nSortGroup ) = 0;
	virtual void	AddFragmentListToRenderList( int nSortGroup, OverlayFragmentHandle_t iFragment, bool bDisp ) = 0;
	virtual void	RenderOverlays(model_t* pWorld, int nSortGroup ) = 0;

	// Sets the client renderable for an overlay's material proxy to bind to
	virtual void	SetOverlayBindProxy( int iOverlayID, void *pBindProxy ) = 0;
};


//-----------------------------------------------------------------------------
// Overlay manager singleton
//-----------------------------------------------------------------------------
IOverlayMgr *OverlayMgr();

#endif // OVERLAY_H
