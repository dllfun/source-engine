//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Resource collection entity
//
// $NoKeywords: $
//=============================================================================//

#ifndef SKYCAMERA_H
#define SKYCAMERA_H

#ifdef _WIN32
#pragma once
#endif

class CSkyCamera;

//=============================================================================
//
// Sky Camera Class
//
class CSkyCamera : public CLogicalEntity
{
	DECLARE_CLASS( CSkyCamera, CLogicalEntity );
	DECLARE_SERVERCLASS();
public:

	DECLARE_DATADESC();
	CSkyCamera();
	~CSkyCamera();
	virtual void Spawn( void );
	virtual void Activate();

public:
	sky3dparams_t	m_skyboxData;
	bool			m_bUseAngles;
	CSkyCamera		*m_pNext;
public:
	BEGIN_INIT_SEND_TABLE(CSkyCamera)
	BEGIN_SEND_TABLE(CSkyCamera, DT_SkyCamera, DT_BaseEntity)

	END_SEND_TABLE(DT_SkyCamera)
	END_INIT_SEND_TABLE()
};


//-----------------------------------------------------------------------------
// Retrives the current skycamera
//-----------------------------------------------------------------------------
CSkyCamera*		GetCurrentSkyCamera();
CSkyCamera*		GetSkyCameraList();


#endif // SKYCAMERA_H
