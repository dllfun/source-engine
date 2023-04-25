//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef IOCCLUSIONSYSTEM_H
#define	IOCCLUSIONSYSTEM_H

#ifdef _WIN32
#pragma once
#endif


class Vector;
class VMatrix;
class model_t;
class VPlane;
class CUtlBuffer;


//-----------------------------------------------------------------------------
// Occlusion system interface
//-----------------------------------------------------------------------------
class IOcclusionSystem
{
public:
	// Activate/deactivate an occluder brush model
	virtual void ActivateOccluder(model_t* pWorld, int nOccluderIndex, bool bActive ) = 0;

	// Sets the view transform
	virtual void SetView( const Vector &vecCameraPos, float flFOV, const VMatrix &worldToCamera, 
		const VMatrix &cameraToProjection, const VPlane &nearClipPlane ) = 0;

	// Test for occlusion (bounds specified in abs space)
	virtual bool IsOccluded(model_t* pWorld, const Vector &vecAbsMins, const Vector &vecAbsMaxs ) = 0;

	// Sets global occlusion parameters
	virtual void SetOcclusionParameters( float flMaxOccludeeArea, float flMinOccluderArea ) = 0;
	virtual float MinOccluderArea() const = 0;

	// Render debugging overlay
	virtual void DrawDebugOverlays() = 0;
};


//-----------------------------------------------------------------------------
// Singleton accessor
//-----------------------------------------------------------------------------
IOcclusionSystem *OcclusionSystem();


#endif // IOCCLUSIONSYSTEM_H
