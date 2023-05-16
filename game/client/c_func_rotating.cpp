//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#include "cbase.h"
#include <KeyValues.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern void RecvProxy_SimulationTime(const CRecvProxyData* pData, void* pStruct, void* pOut);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_FuncRotating : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_FuncRotating, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	C_FuncRotating();

private:

	BEGIN_RECV_TABLE(C_FuncRotating, DT_FuncRotating, DT_BaseEntity)
		RecvPropVector(RECVINFO_NAME(m_vecNetworkOrigin, m_vecOrigin)),
		RecvPropFloat(RECVINFO_NAME(m_angNetworkAngles[0], m_angRotation[0])),
		RecvPropFloat(RECVINFO_NAME(m_angNetworkAngles[1], m_angRotation[1])),
		RecvPropFloat(RECVINFO_NAME(m_angNetworkAngles[2], m_angRotation[2])),
		RecvPropInt(RECVINFO(m_flSimulationTime), 0, RecvProxy_SimulationTime),
	END_RECV_TABLE(DT_FuncRotating)
};


IMPLEMENT_CLIENTCLASS( C_FuncRotating, DT_FuncRotating, CFuncRotating )



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_FuncRotating::C_FuncRotating()
{
	SetClassname("C_FuncRotating");
}
