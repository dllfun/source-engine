//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_AI_BASENPC_H
#define C_AI_BASENPC_H
#ifdef _WIN32
#pragma once
#endif


#include "c_basecombatcharacter.h"

// NOTE: MOved all controller code into c_basestudiomodel
class C_AI_BaseNPC : public C_BaseCombatCharacter
{
	DECLARE_CLASS( C_AI_BaseNPC, C_BaseCombatCharacter );

public:
	DECLARE_CLIENTCLASS();

	C_AI_BaseNPC();
	virtual unsigned int	PhysicsSolidMaskForEntity( void ) const;
	virtual bool			IsNPC( void ) { return true; }
	bool					IsMoving( void ){ return m_bIsMoving; }
	bool					ShouldAvoidObstacle( void ){ return m_bPerformAvoidance; }
	virtual bool			AddRagdollToFadeQueue( void ) { return m_bFadeCorpse; }

	virtual void			GetRagdollInitBoneArrays( matrix3x4_t *pDeltaBones0, matrix3x4_t *pDeltaBones1, matrix3x4_t *pCurrentBones, float boneDt );

	int						GetDeathPose( void ) { return m_iDeathPose; }

	bool					ShouldModifyPlayerSpeed( void ) { return m_bSpeedModActive;	}
	int						GetSpeedModifyRadius( void ) { return m_iSpeedModRadius; }
	int						GetSpeedModifySpeed( void ) { return m_iSpeedModSpeed;	}

	void					ClientThink( void );
	void					OnDataChanged( DataUpdateType_t type );
	bool					ImportantRagdoll( void ) { return m_bImportanRagdoll;	}

private:
	C_AI_BaseNPC( const C_AI_BaseNPC & ); // not defined, not accessible
	float m_flTimePingEffect;
	int  m_iDeathPose;
	int	 m_iDeathFrame;

	int m_iSpeedModRadius;
	int m_iSpeedModSpeed;

	bool m_bPerformAvoidance;
	bool m_bIsMoving;
	bool m_bFadeCorpse;
	bool m_bSpeedModActive;
	bool m_bImportanRagdoll;

public:
	BEGIN_INIT_RECV_TABLE(C_AI_BaseNPC)
	BEGIN_RECV_TABLE(C_AI_BaseNPC, DT_AI_BaseNPC, DT_BaseCombatCharacter)
		RecvPropInt(RECVINFO(m_lifeState)),
		RecvPropBool(RECVINFO(m_bPerformAvoidance)),
		RecvPropBool(RECVINFO(m_bIsMoving)),
		RecvPropBool(RECVINFO(m_bFadeCorpse)),
		RecvPropInt(RECVINFO(m_iDeathPose)),
		RecvPropInt(RECVINFO(m_iDeathFrame)),
		RecvPropInt(RECVINFO(m_iSpeedModRadius)),
		RecvPropInt(RECVINFO(m_iSpeedModSpeed)),
		RecvPropInt(RECVINFO(m_bSpeedModActive)),
		RecvPropBool(RECVINFO(m_bImportanRagdoll)),
		RecvPropFloat(RECVINFO(m_flTimePingEffect)),
	END_RECV_TABLE(DT_AI_BaseNPC)
	END_INIT_RECV_TABLE()
};


#endif // C_AI_BASENPC_H
