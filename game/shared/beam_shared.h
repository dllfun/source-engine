//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef BEAM_H
#define BEAM_H
#ifdef _WIN32
#pragma once
#endif

#include "baseentity_shared.h"
#include "baseplayer_shared.h"
#if !defined( CLIENT_DLL )
#include "entityoutput.h"
#endif

#include "beam_flags.h"

#define MAX_BEAM_WIDTH			102.3f
#define MAX_BEAM_SCROLLSPEED	100.0f
#define MAX_BEAM_NOISEAMPLITUDE		64

#define SF_BEAM_STARTON			0x0001
#define SF_BEAM_TOGGLE			0x0002
#define SF_BEAM_RANDOM			0x0004
#define SF_BEAM_RING			0x0008
#define SF_BEAM_SPARKSTART		0x0010
#define SF_BEAM_SPARKEND		0x0020
#define SF_BEAM_DECALS			0x0040
#define SF_BEAM_SHADEIN			0x0080
#define SF_BEAM_SHADEOUT		0x0100
#define	SF_BEAM_TAPEROUT		0x0200	// Tapers to zero
#define SF_BEAM_TEMPORARY		0x8000

#define ATTACHMENT_INDEX_BITS	5
#define ATTACHMENT_INDEX_MASK	((1 << ATTACHMENT_INDEX_BITS) - 1)

#if defined( CLIENT_DLL )
#define CBeam C_Beam
#include "c_pixel_visibility.h"
#endif

// Beam types, encoded as a byte
enum
{
	BEAM_POINTS = 0,
	BEAM_ENTPOINT,
	BEAM_ENTS,
	BEAM_HOSE,
	BEAM_SPLINE,
	BEAM_LASER,
	NUM_BEAM_TYPES
};

#if !defined( CLIENT_DLL )
void* SendProxy_SendBeamPredictableId(const SendProp* pProp, const void* pStruct, const void* pVarData, CSendProxyRecipients* pRecipients, int objectID);
#endif
#ifdef CLIENT_DLL
void RecvProxy_Beam_ScrollSpeed(const CRecvProxyData* pData, void* pStruct, void* pOut);
#endif

class CBeam : public CBaseEntity
{
	DECLARE_CLASS( CBeam, CBaseEntity );
public:
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();
	DECLARE_SEND_TABLE_ACCESS(DT_Beam);
#endif

	CBeam();

	virtual void SetModel( const char *szModelName );

	void	Spawn( void );
	void	Precache( void );
#if !defined( CLIENT_DLL )
	int		ObjectCaps( void );
	void	SetTransmit( CCheckTransmitInfo *pInfo, bool bAlways );
	int		UpdateTransmitState( void );
	int		ShouldTransmit( const CCheckTransmitInfo *pInfo );
#endif

	virtual int DrawDebugTextOverlays(void);

	// These functions are here to show the way beams are encoded as entities.
	// Encoding beams as entities simplifies their management in the client/server architecture
	void SetType( int type );
	void SetBeamFlags( int flags );
	void SetBeamFlag( int flag );
	
	// NOTE: Start + End Pos are specified in *relative* coordinates 
	void SetStartPos( const Vector &pos );
	void SetEndPos( const Vector &pos );

	// This will change things so the abs position matches the requested spot
	void SetAbsStartPos( const Vector &pos );
	void SetAbsEndPos( const Vector &pos );

	const Vector &GetAbsStartPos( void ) const;
	const Vector &GetAbsEndPos( void ) const;

	void SetStartEntity( CBaseEntity *pEntity );
	void SetEndEntity( CBaseEntity *pEntity );

	void SetStartAttachment( int attachment );
	void SetEndAttachment( int attachment );

	void SetTexture( int spriteIndex );
	void SetHaloTexture( int spriteIndex );
	void SetHaloScale( float haloScale );
	void SetWidth( float width );
	void SetEndWidth( float endWidth );
	void SetFadeLength( float fadeLength );
	void SetNoise( float amplitude );
	void SetColor( int r, int g, int b );
	void SetBrightness( int brightness );
	void SetFrame( float frame );
	void SetScrollRate( int speed );
	void SetFireTime( float flFireTime );
	void SetFrameRate( float flFrameRate ) { m_flFrameRate = flFrameRate; }

	void SetMinDXLevel( int nMinDXLevel ) { m_nMinDXLevel = nMinDXLevel; }

	void TurnOn( void );
	void TurnOff( void );

	int	GetType( void ) const;
	int	GetBeamFlags( void ) const;
	CBaseEntity* GetStartEntityPtr( void ) const;
	int	GetStartEntity( void ) const;
	CBaseEntity* GetEndEntityPtr( void ) const;
	int	GetEndEntity( void ) const;
	int GetStartAttachment() const;
	int GetEndAttachment() const;

	virtual const Vector &WorldSpaceCenter( void ) const;

	int GetTexture( void );
	float GetWidth( void ) const;
	float GetEndWidth( void ) const;
	float GetFadeLength( void ) const;
	float GetNoise( void ) const;
	int GetBrightness( void ) const;
	float GetFrame( void ) const;
	float GetScrollRate( void ) const;
	float GetHDRColorScale( void ) const;
	void SetHDRColorScale( float flScale ) { m_flHDRColorScale = flScale; }


	// Call after you change start/end positions
	void		RelinkBeam( void );

	void		DoSparks( const Vector &start, const Vector &end );
	CBaseEntity *RandomTargetname( const char *szName );
	void		BeamDamage( trace_t *ptr );
	// Init after BeamCreate()
	void		BeamInit( const char *pSpriteName, float width );
	void		PointsInit( const Vector &start, const Vector &end );
	void		PointEntInit( const Vector &start, CBaseEntity *pEndEntity );
	void		EntsInit( CBaseEntity *pStartEntity, CBaseEntity *pEndEntity );
	void		LaserInit( CBaseEntity *pStartEntity, CBaseEntity *pEndEntity );
	void		HoseInit( const Vector &start, const Vector &direction );
	void		SplineInit( int nNumEnts, CBaseEntity** pEntList, int *attachment  );

	// Input handlers

	static CBeam *BeamCreate( const char *pSpriteName, float width );
	static CBeam *BeamCreatePredictable( const char *module, int line, bool persist, const char *pSpriteName, float width, CBasePlayer *pOwner );

	void LiveForTime( float time );
	void BeamDamageInstant( trace_t *ptr, float damage );

// Only supported in TF2 right now
#if defined( INVASION_CLIENT_DLL )
	virtual bool	ShouldPredict( void )
	{
		return true;
	}
#endif

	virtual const char *GetDecalName( void ) { return "BigShot"; }

#if defined( CLIENT_DLL )
// IClientEntity overrides.
public:
	virtual int			DrawModel(IVModel* pWorld, int flags );
	virtual bool		IsTransparent( void );
	virtual bool		ShouldDraw();
	virtual bool		IgnoresZBuffer( void ) const { return true; }
	virtual void		OnDataChanged( DataUpdateType_t updateType );

	virtual bool		OnPredictedEntityRemove( bool isbeingremoved, C_BaseEntity *predicted );

	// Add beam to visible entities list?
	virtual void		AddEntity( void );
	virtual bool		ShouldReceiveProjectedTextures( int flags )
	{
		return false;
	}

// Beam Data Elements
private:
	// Computes the bounding box of a beam local to the origin of the beam
	void ComputeBounds( Vector& mins, Vector& maxs );

	friend void RecvProxy_Beam_ScrollSpeed( const CRecvProxyData *pData, void *pStruct, void *pOut );
	friend class CViewRenderBeams;

#endif

protected:
	CNetworkVar( float, m_flFrameRate );
	CNetworkVar( float, m_flHDRColorScale );
	float		m_flFireTime;
	float		m_flDamage;			// Damage per second to touchers.
	CNetworkVar( int, m_nNumBeamEnts );
#if defined( CLIENT_DLL )
	pixelvis_handle_t	m_queryHandleHalo;
#endif

private:
#if !defined( CLIENT_DLL )
	void InputNoise( inputdata_t &inputdata );
 	void InputWidth( inputdata_t &inputdata );
	void InputColorRedValue( inputdata_t &inputdata );
	void InputColorBlueValue( inputdata_t &inputdata );
	void InputColorGreenValue( inputdata_t &inputdata );
#endif

	// Beam Data Elements
	CNetworkVar( int, m_nHaloIndex );
	CNetworkVar( int, m_nBeamType );
	CNetworkVar( int, m_nBeamFlags );
	CNetworkArray( EHANDLE, m_hAttachEntity, MAX_BEAM_ENTS );
	CNetworkArray( int, m_nAttachIndex, MAX_BEAM_ENTS );
	CNetworkVar( float, m_fWidth );
	CNetworkVar( float, m_fEndWidth );
	CNetworkVar( float, m_fFadeLength );
	CNetworkVar( float, m_fHaloScale );
	CNetworkVar( float, m_fAmplitude );
	CNetworkVar( float, m_fStartFrame );
	CNetworkVar( float, m_fSpeed );
	CNetworkVar( int, m_nMinDXLevel );
	CNetworkVar( float, m_flFrame );

	CNetworkVector( m_vecEndPos );

	EHANDLE		m_hEndEntity;

#if !defined( CLIENT_DLL )
	int			m_nDissolveType;
#endif

public:
#ifdef PORTAL
	CNetworkVar( bool, m_bDrawInMainRender );
	CNetworkVar( bool, m_bDrawInPortalRender );
#endif //#ifdef PORTAL

#if !defined( NO_ENTITY_PREDICTION )
#if !defined( CLIENT_DLL )
	BEGIN_NETWORK_TABLE_NOBASE(CBeam, DT_BeamPredictableId)
		SendPropPredictableId(SENDINFO(m_PredictableID)),
		SendPropInt(SENDINFO(m_bIsPlayerSimulated), 1, SPROP_UNSIGNED),
	END_NETWORK_TABLE(DT_BeamPredictableId)

	BEGIN_NETWORK_TABLE_NOBASE(CBeam, DT_Beam)
		SendPropInt(SENDINFO(m_nBeamType), Q_log2(NUM_BEAM_TYPES) + 1, SPROP_UNSIGNED),
		SendPropInt(SENDINFO(m_nBeamFlags), NUM_BEAM_FLAGS, SPROP_UNSIGNED),
		SendPropInt(SENDINFO(m_nNumBeamEnts), 5, SPROP_UNSIGNED),
		SendPropArray3
		(
			SENDINFO_ARRAY3(m_hAttachEntity),
			SendPropEHandle(SENDINFO_ARRAY(m_hAttachEntity))
		),
		SendPropArray3
		(
			SENDINFO_ARRAY3(m_nAttachIndex),
			SendPropInt(SENDINFO_ARRAY(m_nAttachIndex), ATTACHMENT_INDEX_BITS, SPROP_UNSIGNED)
		),
		SendPropInt(SENDINFO(m_nHaloIndex), 16, SPROP_UNSIGNED),
		SendPropFloat(SENDINFO(m_fHaloScale), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_fWidth), 10, SPROP_ROUNDUP, 0.0f, MAX_BEAM_WIDTH),
		SendPropFloat(SENDINFO(m_fEndWidth), 10, SPROP_ROUNDUP, 0.0f, MAX_BEAM_WIDTH),
		SendPropFloat(SENDINFO(m_fFadeLength), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_fAmplitude), 8, SPROP_ROUNDDOWN, 0.0f, MAX_BEAM_NOISEAMPLITUDE),
		SendPropFloat(SENDINFO(m_fStartFrame), 8, SPROP_ROUNDDOWN, 0.0f, 256.0f),
		SendPropFloat(SENDINFO(m_fSpeed), 8, SPROP_NOSCALE, 0.0f, MAX_BEAM_SCROLLSPEED),
		SendPropInt(SENDINFO(m_nRenderFX), 8, SPROP_UNSIGNED),
		SendPropInt(SENDINFO(m_nRenderMode), 8, SPROP_UNSIGNED),
		SendPropFloat(SENDINFO(m_flFrameRate), 10, SPROP_ROUNDUP, -25.0f, 25.0f),
		SendPropFloat(SENDINFO(m_flHDRColorScale), 0, SPROP_NOSCALE, 0.0f, 100.0f),
		SendPropFloat(SENDINFO(m_flFrame), 20, SPROP_ROUNDDOWN | SPROP_CHANGES_OFTEN, 0.0f, 256.0f),
		SendPropInt(SENDINFO(m_clrRender), 32, SPROP_UNSIGNED | SPROP_CHANGES_OFTEN),
		SendPropVector(SENDINFO(m_vecEndPos), -1, SPROP_COORD),
#ifdef PORTAL
		SendPropBool(SENDINFO(m_bDrawInMainRender)),
		SendPropBool(SENDINFO(m_bDrawInPortalRender)),
#endif
		SendPropModelIndex(SENDINFO(m_nModelIndex)),
		SendPropVector(SENDINFO(m_vecOrigin), 19, SPROP_CHANGES_OFTEN, MIN_COORD_INTEGER, MAX_COORD_INTEGER),
		SendPropEHandle(SENDINFO_NAME(m_hMoveParent, moveparent)),
		SendPropInt(SENDINFO(m_nMinDXLevel), 8, SPROP_UNSIGNED),
#if !defined( NO_ENTITY_PREDICTION )
		SendPropDataTable("beampredictable_id", 0, REFERENCE_SEND_TABLE(DT_BeamPredictableId), SendProxy_SendBeamPredictableId),
#endif
	END_NETWORK_TABLE(DT_Beam)
#endif
#endif

#if !defined( NO_ENTITY_PREDICTION )
#if defined( CLIENT_DLL )
	BEGIN_NETWORK_TABLE_NOBASE(CBeam, DT_BeamPredictableId)
		RecvPropPredictableId(RECVINFO(m_PredictableID)),
		RecvPropInt(RECVINFO(m_bIsPlayerSimulated)),
	END_NETWORK_TABLE(DT_BeamPredictableId)
#endif
#endif

#if defined( CLIENT_DLL )
	BEGIN_NETWORK_TABLE_NOBASE(CBeam, DT_Beam)
		RecvPropInt(RECVINFO(m_nBeamType)),
		RecvPropInt(RECVINFO(m_nBeamFlags)),
		RecvPropInt(RECVINFO(m_nNumBeamEnts)),
		RecvPropArray3
		(
			RECVINFO_ARRAY(m_hAttachEntity),
			RecvPropEHandle(RECVINFO(m_hAttachEntity[0]))
		),
		RecvPropArray3
		(
			RECVINFO_ARRAY(m_nAttachIndex),
			RecvPropInt(RECVINFO(m_nAttachIndex[0]))
		),
		RecvPropInt(RECVINFO(m_nHaloIndex)),
		RecvPropFloat(RECVINFO(m_fHaloScale)),
		RecvPropFloat(RECVINFO(m_fWidth)),
		RecvPropFloat(RECVINFO(m_fEndWidth)),
		RecvPropFloat(RECVINFO(m_fFadeLength)),
		RecvPropFloat(RECVINFO(m_fAmplitude)),
		RecvPropFloat(RECVINFO(m_fStartFrame)),
		RecvPropFloat(RECVINFO(m_fSpeed), 0, RecvProxy_Beam_ScrollSpeed),
		RecvPropFloat(RECVINFO(m_flFrameRate)),
		RecvPropFloat(RECVINFO(m_flHDRColorScale)),
		RecvPropInt(RECVINFO(m_clrRender)),
		RecvPropInt(RECVINFO(m_nRenderFX)),
		RecvPropInt(RECVINFO(m_nRenderMode)),
		RecvPropFloat(RECVINFO(m_flFrame)),
		RecvPropVector(RECVINFO(m_vecEndPos)),
#ifdef PORTAL
		RecvPropBool(RECVINFO(m_bDrawInMainRender)),
		RecvPropBool(RECVINFO(m_bDrawInPortalRender)),
#endif
		RecvPropInt(RECVINFO(m_nModelIndex)),
		RecvPropInt(RECVINFO(m_nMinDXLevel)),

		RecvPropVector(RECVINFO_NAME(m_vecNetworkOrigin, m_vecOrigin)),
		RecvPropInt(RECVINFO_NAME(m_hNetworkMoveParent, moveparent), 0, RecvProxy_IntToMoveParent),
#if !defined( NO_ENTITY_PREDICTION )
		RecvPropDataTable("beampredictable_id", 0, 0, REFERENCE_RECV_TABLE(DT_BeamPredictableId)),
#endif

	END_NETWORK_TABLE(DT_Beam)
#endif
};

#if !defined( CLIENT_DLL )
//-----------------------------------------------------------------------------
// Inline methods 
//-----------------------------------------------------------------------------
inline int CBeam::ObjectCaps( void )
{ 
	int flags = 0;
	if ( HasSpawnFlags( SF_BEAM_TEMPORARY ) )
		flags = FCAP_DONT_SAVE;
	return (BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | flags; 
}
#endif

inline void	CBeam::SetFireTime( float flFireTime )		
{ 
	m_flFireTime = flFireTime; 
}

//-----------------------------------------------------------------------------
// NOTE: Start + End Pos are specified in *relative* coordinates 
//-----------------------------------------------------------------------------
inline void CBeam::SetStartPos( const Vector &pos ) 
{ 
#if defined( CLIENT_DLL )
	SetNetworkOrigin( pos );
#endif
	SetLocalOrigin( pos );
}

inline void CBeam::SetEndPos( const Vector &pos ) 
{ 
	m_vecEndPos = pos; 
}
	 
 // center point of beam
inline const Vector &CBeam::WorldSpaceCenter( void ) const 
{
	Vector &vecResult = AllocTempVector();
	VectorAdd( GetAbsStartPos(), GetAbsEndPos(), vecResult );
	vecResult *= 0.5f;
	return vecResult;
}

inline void CBeam::SetStartAttachment( int attachment )	
{
	Assert( (attachment & ~ATTACHMENT_INDEX_MASK) == 0 );
	m_nAttachIndex.Set( 0, attachment );
}

inline void CBeam::SetEndAttachment( int attachment )		
{ 
	Assert( (attachment & ~ATTACHMENT_INDEX_MASK) == 0 );
	m_nAttachIndex.Set( m_nNumBeamEnts-1, attachment );
}

inline void CBeam::SetTexture( int spriteIndex )		
{ 
	SetModelIndex( spriteIndex ); 
}

inline void CBeam::SetHaloTexture( int spriteIndex )	
{ 
	m_nHaloIndex = spriteIndex; 
}

inline void CBeam::SetHaloScale( float haloScale )		
{ 
	m_fHaloScale = haloScale; 
}

inline void CBeam::SetWidth( float width )				
{
	Assert( width <= MAX_BEAM_WIDTH );
	m_fWidth = MIN( MAX_BEAM_WIDTH, width );
}

inline void CBeam::SetEndWidth( float endWidth )		
{ 
	Assert( endWidth <= MAX_BEAM_WIDTH );
	m_fEndWidth	= MIN( MAX_BEAM_WIDTH, endWidth );
}

inline void CBeam::SetFadeLength( float fadeLength )	
{ 
	m_fFadeLength = fadeLength; 
}

inline void CBeam::SetNoise( float amplitude )			
{ 
	m_fAmplitude = amplitude; 
}

inline void CBeam::SetColor( int r, int g, int b )		
{ 
	SetRenderColor( r, g, b, GetRenderColor().a );
}

inline void CBeam::SetBrightness( int brightness )		
{ 
	SetRenderColorA( brightness ); 
}

inline void CBeam::SetFrame( float frame )				
{ 
	m_fStartFrame = frame; 
}

inline void CBeam::SetScrollRate( int speed )			
{ 
	m_fSpeed = speed; 
}

inline CBaseEntity* CBeam::GetStartEntityPtr( void ) const 
{ 
	return m_hAttachEntity[0].Get(); 
}

inline int CBeam::GetStartEntity( void ) const 
{ 
	CBaseEntity *pEntity = m_hAttachEntity[0].Get();
	return pEntity ? pEntity->NetworkProp()->entindex() : 0;
}

inline CBaseEntity* CBeam::GetEndEntityPtr( void ) const 
{ 
	return m_hAttachEntity[1].Get(); 
}

inline int CBeam::GetEndEntity( void ) const	
{ 
	CBaseEntity *pEntity = m_hAttachEntity[m_nNumBeamEnts-1].Get();
	return pEntity ? pEntity->NetworkProp()->entindex() : 0;
}

inline int CBeam::GetStartAttachment() const
{
	return m_nAttachIndex[0] & ATTACHMENT_INDEX_MASK;
}

inline int CBeam::GetEndAttachment() const
{
	return m_nAttachIndex[m_nNumBeamEnts-1] & ATTACHMENT_INDEX_MASK;
}

inline int CBeam::GetTexture( void )		
{ 
	return GetModelIndex(); 
}

inline float CBeam::GetWidth( void ) const		
{
	return m_fWidth; 
}

inline float CBeam::GetEndWidth( void ) const	
{ 
	return m_fEndWidth; 
}

inline float CBeam::GetFadeLength( void ) const	
{ 
	return m_fFadeLength; 
}

inline float CBeam::GetNoise( void ) const		
{ 
	return m_fAmplitude; 
}

inline int CBeam::GetBrightness( void ) const	
{ 
	return GetRenderColor().a;
}

inline float CBeam::GetFrame( void ) const		
{ 
	return m_fStartFrame; 
}

inline float CBeam::GetScrollRate( void ) const	
{
	return m_fSpeed; 
}

inline float CBeam::GetHDRColorScale( void ) const
{
	return m_flHDRColorScale;
}

inline void CBeam::LiveForTime( float time ) 
{ 
	SetThink(&CBeam::SUB_Remove); 
	SetNextThink( gpGlobals->GetCurTime() + time ); 
}

inline void	CBeam::BeamDamageInstant( trace_t *ptr, float damage ) 
{ 
	m_flDamage = damage; 
	m_flFireTime = gpGlobals->GetCurTime() - 1;
	BeamDamage(ptr); 
}

bool IsStaticPointEntity( CBaseEntity *pEnt );

// Macro to wrap creation
#define BEAM_CREATE_PREDICTABLE( name, width, player ) \
	CBeam::BeamCreatePredictable( __FILE__, __LINE__, false, name, width, player )

#define BEAM_CREATE_PREDICTABLE_PERSIST( name, width, player ) \
	CBeam::BeamCreatePredictable( __FILE__, __LINE__, true, name, width, player )

// Start/End Entity is encoded as 12 bits of entity index, and 4 bits of attachment (4:12)
#define BEAMENT_ENTITY(x)		((x)&0xFFF)
#define BEAMENT_ATTACHMENT(x)	(((x)>>12)&0xF)





#endif // BEAM_H
