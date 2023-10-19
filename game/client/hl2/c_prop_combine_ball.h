//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef CPROPCOMBINEBALL_H_
#define CPROPCOMBINEBALL_H_

#ifdef _WIN32
#pragma once
#endif

class C_PropCombineBall : public C_BaseAnimating
{
	DECLARE_CLASS( C_PropCombineBall, C_BaseAnimating );
	DECLARE_CLIENTCLASS();
public:

	C_PropCombineBall( void );

	virtual RenderGroup_t GetRenderGroup( void );

	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual int		DrawModel(IVModel* pWorld, int flags );

protected:

	void	DrawMotionBlur( void );
	void	DrawFlicker( void );
	virtual bool	InitMaterials( void );

	Vector	m_vecLastOrigin;
	bool	m_bEmit;
	float	m_flRadius;
	bool	m_bHeld;
	bool	m_bLaunched;

	IMaterial	*m_pFlickerMaterial;
	IMaterial	*m_pBodyMaterial;
	IMaterial	*m_pBlurMaterial;

public:
	BEGIN_INIT_RECV_TABLE(C_PropCombineBall)
	BEGIN_RECV_TABLE(C_PropCombineBall, DT_PropCombineBall, DT_BaseAnimating)
		RecvPropBool(RECVINFO(m_bEmit)),
		RecvPropFloat(RECVINFO(m_flRadius)),
		RecvPropBool(RECVINFO(m_bHeld)),
		RecvPropBool(RECVINFO(m_bLaunched)),
	END_RECV_TABLE()
	END_INIT_RECV_TABLE()
};


#endif