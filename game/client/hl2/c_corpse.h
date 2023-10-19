//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#if !defined( C_CORPSE_H )
#define C_CORPSE_H
#ifdef _WIN32
#pragma once
#endif

class C_Corpse : public C_BaseAnimating
{
public:
	DECLARE_CLASS( C_Corpse, C_BaseAnimating );
	DECLARE_CLIENTCLASS();

						C_Corpse( void );

	virtual int			DrawModel(IVModel* pWorld, int flags );

public:
	// The player whom we are copying our data from
	int					m_nReferencePlayer;

private:
						C_Corpse( const C_Corpse & );
public:
	BEGIN_INIT_RECV_TABLE(C_Corpse)
	BEGIN_RECV_TABLE(C_Corpse, DT_Corpse, DT_BaseAnimating)
		RecvPropInt(RECVINFO(m_nReferencePlayer))
	END_RECV_TABLE()
	END_INIT_RECV_TABLE()
};


#endif // C_CORPSE_H