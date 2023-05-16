//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Controls the pose parameters of a model
//
//===========================================================================//


#define MAX_POSE_CONTROLLED_PROPS 4		// Number of entities by the same name that can be controlled


// Type of frequency modulations
enum PoseController_FModType_t
{
	POSECONTROLLER_FMODTYPE_NONE = 0,
	POSECONTROLLER_FMODTYPE_SINE,
	POSECONTROLLER_FMODTYPE_SQUARE,
	POSECONTROLLER_FMODTYPE_TRIANGLE,
	POSECONTROLLER_FMODTYPE_SAWTOOTH,
	POSECONTROLLER_FMODTYPE_NOISE,

	POSECONTROLLER_FMODTYPE_TOTAL,
};


#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// SERVER CLASS
//-----------------------------------------------------------------------------

#include "baseentity.h"

#define MAX_POSE_INTERPOLATION_TIME 10.0f
#define MAX_POSE_CYCLE_FREQUENCY 10.0f
#define MAX_POSE_FMOD_RATE 10.0f
#define MAX_POSE_FMOD_AMPLITUDE 10.0f

class CPoseController : public CBaseEntity
{
public:
	DECLARE_CLASS( CPoseController, CBaseEntity );
	DECLARE_SERVERCLASS();
	DECLARE_SEND_TABLE_ACCESS(DT_PoseController);
	DECLARE_DATADESC();

	virtual void Spawn( void );

	void Think( void );

	void BuildPropList( void );
	void BuildPoseIndexList( void );
	void SetPoseIndex( int i, int iValue );
	void SetCurrentPose( float fCurrentPoseValue );

	float GetPoseValue( void );

	void SetProp( CBaseAnimating *pProp );
	void SetPropName( const char *pName );
	void SetPoseParameterName( const char *pName );
	void SetPoseValue( float fValue );
	void SetInterpolationTime( float fValue );
	void SetInterpolationWrap( bool bWrap );
	void SetCycleFrequency( float fValue );
	void SetFModType( int nType );
	void SetFModTimeOffset( float fValue );
	void SetFModRate( float fValue );
	void SetFModAmplitude( float fValue );
	void RandomizeFMod( float fExtremeness );

	// Input handlers
	void InputSetPoseParameterName( inputdata_t &inputdata );
	void InputSetPoseValue( inputdata_t &inputdata );
	void InputSetInterpolationTime( inputdata_t &inputdata );
	void InputSetCycleFrequency( inputdata_t &inputdata );
	void InputSetFModType( inputdata_t &inputdata );
	void InputSetFModTimeOffset( inputdata_t &inputdata );
	void InputSetFModRate( inputdata_t &inputdata );
	void InputSetFModAmplitude( inputdata_t &inputdata );
	void InputRandomizeFMod( inputdata_t &inputdata );
	void InputGetFMod( inputdata_t &inputdata );

private:

	CNetworkArray( EHANDLE, m_hProps, MAX_POSE_CONTROLLED_PROPS );				// Handles to controlled models
	CNetworkArray( unsigned char, m_chPoseIndex, MAX_POSE_CONTROLLED_PROPS );	// Pose parameter indices for each model

	bool		m_bDisablePropLookup;

	CNetworkVar( bool, m_bPoseValueParity );

	string_t	m_iszPropName;				// Targetname of the models to control
	string_t	m_iszPoseParameterName;		// Pose parameter name to control

	CNetworkVar( float, m_fPoseValue );			// Normalized pose parameter value (maps to each pose parameter's min and max range)
	CNetworkVar( float, m_fInterpolationTime );	// Interpolation speed for client matching absolute pose values
	CNetworkVar( bool, m_bInterpolationWrap );	// Interpolation for the client wraps 0 to 1.

	CNetworkVar( float, m_fCycleFrequency );	// Cycles per second

	// Frequency modulation variables
	CNetworkVar( PoseController_FModType_t, m_nFModType );
	CNetworkVar( float, m_fFModTimeOffset );
	CNetworkVar( float, m_fFModRate );
	CNetworkVar( float, m_fFModAmplitude );

	BEGIN_SEND_TABLE(CPoseController, DT_PoseController, DT_BaseEntity)
		SendPropArray3(SENDINFO_ARRAY3(m_hProps), SendPropEHandle(SENDINFO_ARRAY(m_hProps))),
		SendPropArray3(SENDINFO_ARRAY3(m_chPoseIndex), SendPropInt(SENDINFO_ARRAY(m_chPoseIndex), 5, SPROP_UNSIGNED)),	// bits sent must be enough to represent MAXSTUDIOPOSEPARAM
		SendPropBool(SENDINFO(m_bPoseValueParity)),
		SendPropFloat(SENDINFO(m_fPoseValue), 11, 0, 0.0f, 1.0f),
		SendPropFloat(SENDINFO(m_fInterpolationTime), 11, 0, 0.0f, MAX_POSE_INTERPOLATION_TIME),
		SendPropBool(SENDINFO(m_bInterpolationWrap)),
		SendPropFloat(SENDINFO(m_fCycleFrequency), 11, 0, -MAX_POSE_CYCLE_FREQUENCY, MAX_POSE_CYCLE_FREQUENCY),
		SendPropInt(SENDINFO(m_nFModType), 3, SPROP_UNSIGNED),
		SendPropFloat(SENDINFO(m_fFModTimeOffset), 11, 0, -1.0f, 1.0f),
		SendPropFloat(SENDINFO(m_fFModRate), 11, 0, -MAX_POSE_FMOD_RATE, MAX_POSE_FMOD_RATE),
		SendPropFloat(SENDINFO(m_fFModAmplitude), 11, 0, 0.0f, MAX_POSE_FMOD_AMPLITUDE),
	END_SEND_TABLE(DT_PoseController)
};


#else //#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// CLIENT CLASS
//-----------------------------------------------------------------------------

#include "c_baseentity.h"
#include "fx_interpvalue.h"


class C_PoseController : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_PoseController, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	virtual void	Spawn( void );
	virtual void	OnDataChanged( DataUpdateType_t updateType );

	virtual void	ClientThink( void );

private:

	void	UpdateModulation( void );
	void	UpdatePoseCycle( float fCycleAmount );
	void	SetCurrentPose( float fCurrentPoseValue );


	// Networked variables
	EHANDLE						m_hProps[MAX_POSE_CONTROLLED_PROPS];
	unsigned char				m_chPoseIndex[MAX_POSE_CONTROLLED_PROPS];
	bool						m_bPoseValueParity;
	float						m_fPoseValue;
	float						m_fInterpolationTime;
	bool						m_bInterpolationWrap;
	float						m_fCycleFrequency;
	PoseController_FModType_t	m_nFModType;
	float						m_fFModTimeOffset;
	float						m_fFModRate;
	float						m_fFModAmplitude;
	bool	m_bOldPoseValueParity;

	float	m_fCurrentPoseValue;	// Actual pose value cycled by the frequency and modulation
	float	m_fCurrentFMod;			// The current fequency modulation amount (stored for noise walk)

	CInterpolatedValue	m_PoseTransitionValue;

	BEGIN_RECV_TABLE(C_PoseController, DT_PoseController, DT_BaseEntity)
		RecvPropArray3(RECVINFO_ARRAY(m_hProps), RecvPropEHandle(RECVINFO(m_hProps[0]))),
		RecvPropArray3(RECVINFO_ARRAY(m_chPoseIndex), RecvPropInt(RECVINFO(m_chPoseIndex[0]))),
		RecvPropBool(RECVINFO(m_bPoseValueParity)),
		RecvPropFloat(RECVINFO(m_fPoseValue)),
		RecvPropFloat(RECVINFO(m_fInterpolationTime)),
		RecvPropBool(RECVINFO(m_bInterpolationWrap)),
		RecvPropFloat(RECVINFO(m_fCycleFrequency)),
		RecvPropInt(RECVINFO(m_nFModType)),
		RecvPropFloat(RECVINFO(m_fFModTimeOffset)),
		RecvPropFloat(RECVINFO(m_fFModRate)),
		RecvPropFloat(RECVINFO(m_fFModAmplitude)),
	END_RECV_TABLE(DT_PoseController)
};


#endif //#ifndef CLIENT_DLL