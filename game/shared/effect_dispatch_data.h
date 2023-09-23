//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#ifndef EFFECT_DISPATCH_DATA_H
#define EFFECT_DISPATCH_DATA_H
#ifdef _WIN32
#pragma once
#endif

#include "particle_parse.h"
#include "qlimits.h"

#ifdef CLIENT_DLL

	#include "dt_recv.h"
	#include "client_class.h"

	EXTERN_RECV_TABLE( DT_EffectData );

#else

	#include "dt_send.h"
	#include "server_class.h"

	EXTERN_SEND_TABLE( DT_EffectData );

#endif

// NOTE: These flags are specifically *not* networked; so it's placed above the max effect flag bits
#define EFFECTDATA_NO_RECORD 0x80000000

#define MAX_EFFECT_FLAG_BITS 8

#define CUSTOM_COLOR_CP1		9
#define CUSTOM_COLOR_CP2		10

#define MAX_EFFECT_DISPATCH_STRING_BITS	10
#define MAX_EFFECT_DISPATCH_STRINGS		( 1 << MAX_EFFECT_DISPATCH_STRING_BITS )

#ifdef CLIENT_DLL
void RecvProxy_EntIndex(const CRecvProxyData* pData, void* pStruct, void* pOut);
#endif

// This is the class that holds whatever data we're sending down to the client to make the effect.
class CEffectData
{
public:
	Vector m_vOrigin;
	Vector m_vStart;
	Vector m_vNormal;
	QAngle m_vAngles;
	int		m_fFlags;
#ifdef CLIENT_DLL
	ClientEntityHandle_t m_hEntity;
#else
	int		m_nEntIndex;
#endif
	float	m_flScale;
	float	m_flMagnitude;
	float	m_flRadius;
	int		m_nAttachmentIndex;
	short	m_nSurfaceProp;

	// Some TF2 specific things
	int		m_nMaterial;
	int		m_nDamageType;
	int		m_nHitBox;
	
	unsigned char	m_nColor;

	// Color customizability
	bool							m_bCustomColors;
	te_tf_particle_effects_colors_t	m_CustomColors;

	bool									m_bControlPoint1;
	te_tf_particle_effects_control_point_t	m_ControlPoint1;

// Don't mess with stuff below here. DispatchEffect handles all of this.
public:
	CEffectData()
	{
		m_vOrigin.Init();
		m_vStart.Init();
		m_vNormal.Init();
		m_vAngles.Init();

		m_fFlags = 0;
#ifdef CLIENT_DLL
		m_hEntity = INVALID_EHANDLE_INDEX;
#else
		m_nEntIndex = 0;
#endif
		m_flScale = 1.f;
		m_nAttachmentIndex = 0;
		m_nSurfaceProp = 0;

		m_flMagnitude = 0.0f;
		m_flRadius = 0.0f;

		m_nMaterial = 0;
		m_nDamageType = 0;
		m_nHitBox = 0;

		m_nColor = 0;

		m_bCustomColors = false;
		m_CustomColors.m_vecColor1.Init();
		m_CustomColors.m_vecColor2.Init();

		m_bControlPoint1 = false;
		m_ControlPoint1.m_eParticleAttachment = PATTACH_ABSORIGIN;
		m_ControlPoint1.m_vecOffset.Init();
	}

	int GetEffectNameIndex() { return m_iEffectName; }

#ifdef CLIENT_DLL
	IClientRenderable *GetRenderable() const;
	C_BaseEntity *GetEntity() const;
	int entindex() const;
#endif

//private:

	#ifdef CLIENT_DLL
		DECLARE_CLIENTCLASS_NOBASE()
	#else
	DECLARE_SERVERCLASS_NOBASE()
	DECLARE_SEND_TABLE_ACCESS(DT_EffectData);
#endif

	int m_iEffectName;	// Entry in the EffectDispatch network string table. The is automatically handled by DispatchEffect().

#ifndef CLIENT_DLL

	BEGIN_INIT_SEND_TABLE(CEffectData)
	BEGIN_SEND_TABLE_NOBASE(CEffectData, DT_EffectData)

		// Everything uses _NOCHECK here since this is not an entity and we don't need
		// the functionality of CNetworkVars.

		// Get half-inch precision here.
#ifdef HL2_DLL
		SendPropFloat(SENDINFO_NOCHECK(m_vOrigin[0]), COORD_INTEGER_BITS + SUBINCH_PRECISION, 0, MIN_COORD_INTEGER, MAX_COORD_INTEGER),
		SendPropFloat(SENDINFO_NOCHECK(m_vOrigin[1]), COORD_INTEGER_BITS + SUBINCH_PRECISION, 0, MIN_COORD_INTEGER, MAX_COORD_INTEGER),
		SendPropFloat(SENDINFO_NOCHECK(m_vOrigin[2]), COORD_INTEGER_BITS + SUBINCH_PRECISION, 0, MIN_COORD_INTEGER, MAX_COORD_INTEGER),
		SendPropFloat(SENDINFO_NOCHECK(m_vStart[0]), COORD_INTEGER_BITS + SUBINCH_PRECISION, 0, MIN_COORD_INTEGER, MAX_COORD_INTEGER),
		SendPropFloat(SENDINFO_NOCHECK(m_vStart[1]), COORD_INTEGER_BITS + SUBINCH_PRECISION, 0, MIN_COORD_INTEGER, MAX_COORD_INTEGER),
		SendPropFloat(SENDINFO_NOCHECK(m_vStart[2]), COORD_INTEGER_BITS + SUBINCH_PRECISION, 0, MIN_COORD_INTEGER, MAX_COORD_INTEGER),
#else
		SendPropFloat(SENDINFO_NOCHECK(m_vOrigin[0]), -1, SPROP_COORD_MP_INTEGRAL),
		SendPropFloat(SENDINFO_NOCHECK(m_vOrigin[1]), -1, SPROP_COORD_MP_INTEGRAL),
		SendPropFloat(SENDINFO_NOCHECK(m_vOrigin[2]), -1, SPROP_COORD_MP_INTEGRAL),
		SendPropFloat(SENDINFO_NOCHECK(m_vStart[0]), -1, SPROP_COORD_MP_INTEGRAL),
		SendPropFloat(SENDINFO_NOCHECK(m_vStart[1]), -1, SPROP_COORD_MP_INTEGRAL),
		SendPropFloat(SENDINFO_NOCHECK(m_vStart[2]), -1, SPROP_COORD_MP_INTEGRAL),
#endif
		SendPropQAngles(SENDINFO_NOCHECK(m_vAngles), 7),

#if defined( TF_DLL )
		SendPropVector(SENDINFO_NOCHECK(m_vNormal), 6, 0, -1.0f, 1.0f),
#else
		SendPropVector(SENDINFO_NOCHECK(m_vNormal), 0, SPROP_NORMAL),
#endif

		SendPropInt(SENDINFO_NOCHECK(m_fFlags), MAX_EFFECT_FLAG_BITS, SPROP_UNSIGNED),
		SendPropFloat(SENDINFO_NOCHECK(m_flMagnitude), 12, SPROP_ROUNDDOWN, 0.0f, 1023.0f),
		SendPropFloat(SENDINFO_NOCHECK(m_flScale), 0, SPROP_NOSCALE),
		SendPropInt(SENDINFO_NOCHECK(m_nAttachmentIndex), 5, SPROP_UNSIGNED),
		SendPropIntWithMinusOneFlag(SENDINFO_NOCHECK(m_nSurfaceProp), 8, SendProxy_ShortAddOne),
		SendPropInt(SENDINFO_NOCHECK(m_iEffectName), MAX_EFFECT_DISPATCH_STRING_BITS, SPROP_UNSIGNED),

		SendPropInt(SENDINFO_NOCHECK(m_nMaterial), MAX_MODEL_INDEX_BITS, SPROP_UNSIGNED),
		SendPropInt(SENDINFO_NOCHECK(m_nDamageType), 32, SPROP_UNSIGNED),
		SendPropInt(SENDINFO_NOCHECK(m_nHitBox), 11, SPROP_UNSIGNED),

		SendPropInt(SENDINFO_NAME(m_nEntIndex, entindex), MAX_EDICT_BITS, SPROP_UNSIGNED),

		SendPropInt(SENDINFO_NOCHECK(m_nColor), 8, SPROP_UNSIGNED),

		SendPropFloat(SENDINFO_NOCHECK(m_flRadius), 10, SPROP_ROUNDDOWN, 0.0f, 1023.0f),

		SendPropBool(SENDINFO_NOCHECK(m_bCustomColors)),
		SendPropVector(SENDINFO_NOCHECK(m_CustomColors.m_vecColor1), 8, 0, 0, 1),
		SendPropVector(SENDINFO_NOCHECK(m_CustomColors.m_vecColor2), 8, 0, 0, 1),

		SendPropBool(SENDINFO_NOCHECK(m_bControlPoint1)),
		SendPropInt(SENDINFO_NOCHECK(m_ControlPoint1.m_eParticleAttachment), 5, SPROP_UNSIGNED),
		SendPropFloat(SENDINFO_NOCHECK(m_ControlPoint1.m_vecOffset[0]), -1, SPROP_COORD),
		SendPropFloat(SENDINFO_NOCHECK(m_ControlPoint1.m_vecOffset[1]), -1, SPROP_COORD),
		SendPropFloat(SENDINFO_NOCHECK(m_ControlPoint1.m_vecOffset[2]), -1, SPROP_COORD),

	END_SEND_TABLE(DT_EffectData)
	END_INIT_SEND_TABLE()

#endif // !CLIENT_DLL

#ifdef CLIENT_DLL
	BEGIN_INIT_RECV_TABLE(CEffectData)
	BEGIN_RECV_TABLE_NOBASE(CEffectData, DT_EffectData)

		RecvPropFloat(RECVINFO(m_vOrigin[0])),
		RecvPropFloat(RECVINFO(m_vOrigin[1])),
		RecvPropFloat(RECVINFO(m_vOrigin[2])),

		RecvPropFloat(RECVINFO(m_vStart[0])),
		RecvPropFloat(RECVINFO(m_vStart[1])),
		RecvPropFloat(RECVINFO(m_vStart[2])),

		RecvPropQAngles(RECVINFO(m_vAngles)),

		RecvPropVector(RECVINFO(m_vNormal)),

		RecvPropInt(RECVINFO(m_fFlags)),
		RecvPropFloat(RECVINFO(m_flMagnitude)),
		RecvPropFloat(RECVINFO(m_flScale)),
		RecvPropInt(RECVINFO(m_nAttachmentIndex)),
		RecvPropIntWithMinusOneFlag(RECVINFO(m_nSurfaceProp), RecvProxy_ShortSubOne),
		RecvPropInt(RECVINFO(m_iEffectName)),

		RecvPropInt(RECVINFO(m_nMaterial)),
		RecvPropInt(RECVINFO(m_nDamageType)),
		RecvPropInt(RECVINFO(m_nHitBox)),

		RecvPropInt("entindex", 0, SIZEOF_IGNORE, 0, RecvProxy_EntIndex),

		RecvPropInt(RECVINFO(m_nColor)),

		RecvPropFloat(RECVINFO(m_flRadius)),

		RecvPropBool(RECVINFO(m_bCustomColors)),
		RecvPropVector(RECVINFO(m_CustomColors.m_vecColor1)),
		RecvPropVector(RECVINFO(m_CustomColors.m_vecColor2)),

		RecvPropBool(RECVINFO(m_bControlPoint1)),
		RecvPropInt(RECVINFO(m_ControlPoint1.m_eParticleAttachment)),
		RecvPropFloat(RECVINFO(m_ControlPoint1.m_vecOffset[0])),
		RecvPropFloat(RECVINFO(m_ControlPoint1.m_vecOffset[1])),
		RecvPropFloat(RECVINFO(m_ControlPoint1.m_vecOffset[2])),

	END_RECV_TABLE(DT_EffectData)
	END_INIT_RECV_TABLE()
#endif
};




#ifdef CLIENT_DLL
bool SuppressingParticleEffects();
void SuppressParticleEffects( bool bSuppress );
#endif

#endif // EFFECT_DISPATCH_DATA_H
