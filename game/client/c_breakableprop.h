//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_BREAKABLEPROP_H
#define C_BREAKABLEPROP_H
#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_BreakableProp : public C_BaseAnimating
{
	typedef C_BaseAnimating BaseClass;
public:
	DECLARE_CLIENTCLASS();

	C_BreakableProp();
	
	virtual void SetFadeMinMax( float fademin, float fademax );

	// Copy fade from another breakable prop
	void CopyFadeFrom( C_BreakableProp *pSource );

public:
	BEGIN_INIT_RECV_TABLE(C_BreakableProp)
	BEGIN_RECV_TABLE(C_BreakableProp, DT_BreakableProp, DT_BaseAnimating)

	END_RECV_TABLE(DT_BreakableProp)
	END_INIT_RECV_TABLE()
};

#endif // C_BREAKABLEPROP_H
