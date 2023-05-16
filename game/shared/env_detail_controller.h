//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifdef CLIENT_DLL
	#define CEnvDetailController C_EnvDetailController
#endif // CLIENT_DLL

//-----------------------------------------------------------------------------
// Implementation of the class that controls detail prop fade distances
//-----------------------------------------------------------------------------
class CEnvDetailController : public CBaseEntity
{
public:
	DECLARE_CLASS( CEnvDetailController, CBaseEntity );
	DECLARE_NETWORKCLASS();

	CEnvDetailController();
	virtual ~CEnvDetailController();

#ifndef CLIENT_DLL
	virtual bool KeyValue( const char *szKeyName, const char *szValue );
#endif // !CLIENT_DLL

	CNetworkVar( float, m_flFadeStartDist );
	CNetworkVar( float, m_flFadeEndDist );

	// ALWAYS transmit to all clients.
	virtual int UpdateTransmitState( void );

#ifndef CLIENT_DLL
	BEGIN_NETWORK_TABLE_NOBASE(CEnvDetailController, DT_DetailController)
		SendPropFloat(SENDINFO(m_flFadeStartDist)),
		SendPropFloat(SENDINFO(m_flFadeEndDist)),
	END_NETWORK_TABLE(DT_DetailController)
#endif

#ifdef CLIENT_DLL
	BEGIN_NETWORK_TABLE_NOBASE(CEnvDetailController, DT_DetailController)
		RecvPropFloat(RECVINFO(m_flFadeStartDist)),
		RecvPropFloat(RECVINFO(m_flFadeEndDist)),
	END_NETWORK_TABLE(DT_DetailController)
#endif
};

CEnvDetailController * GetDetailController();
