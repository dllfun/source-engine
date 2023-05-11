//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef EFFECTS_H
#define EFFECTS_H

#ifdef _WIN32
#pragma once
#endif


class CBaseEntity;
class Vector;


//-----------------------------------------------------------------------------
// The rotor wash shooter. It emits gibs when pushed by a rotor wash
//-----------------------------------------------------------------------------
abstract_class IRotorWashShooter
{
public:
	virtual CBaseEntity *DoWashPush( float flWashStartTime, const Vector &vecForce ) = 0;
};


//-----------------------------------------------------------------------------
// Gets at the interface if the entity supports it
//-----------------------------------------------------------------------------
IRotorWashShooter *GetRotorWashShooter( CBaseEntity *pEntity );

class CEnvQuadraticBeam : public CPointEntity
{
	DECLARE_CLASS( CEnvQuadraticBeam, CPointEntity );

public:
	void Spawn();
	void SetSpline( const Vector &control, const Vector &target )
	{
		m_targetPosition = target;
		m_controlPosition = control;
	}
	void SetScrollRate( float rate )
	{
		m_scrollRate = rate;
	}

	void SetWidth( float width )
	{
		m_flWidth = width;
	}

private:
	CNetworkVector( m_targetPosition );
	CNetworkVector( m_controlPosition );
	CNetworkVar( float, m_scrollRate );
	CNetworkVar( float, m_flWidth );

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
	DECLARE_SEND_TABLE_ACCESS(DT_QuadraticBeam);

	BEGIN_SEND_TABLE(CEnvQuadraticBeam, DT_QuadraticBeam, DT_BaseEntity)
		SendPropVector(SENDINFO(m_targetPosition), -1, SPROP_COORD),
		SendPropVector(SENDINFO(m_controlPosition), -1, SPROP_COORD),
		SendPropFloat(SENDINFO(m_scrollRate), 8, 0, -4, 4),
		SendPropFloat(SENDINFO(m_flWidth), -1, SPROP_NOSCALE),
	END_SEND_TABLE(DT_QuadraticBeam)
};
CEnvQuadraticBeam *CreateQuadraticBeam( const char *pSpriteName, const Vector &start, const Vector &control, const Vector &end, float width, CBaseEntity *pOwner );


#endif // EFFECTS_H
