//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#ifndef IVMODELINFO_H
#define IVMODELINFO_H

#ifdef _WIN32
#pragma once
#endif

#include "platform.h"
#include "dbg.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class IMaterial;
class KeyValues;
struct vcollide_t;
class IVModel;
class Vector;
class QAngle;
class CGameTrace;
struct cplane_t;
typedef CGameTrace trace_t;
struct studiohdr_t;
struct virtualmodel_t;
typedef unsigned char byte;
struct virtualterrainparams_t;
class CPhysCollide;
typedef unsigned short MDLHandle_t;
class CUtlBuffer;
class IClientRenderable;


//-----------------------------------------------------------------------------
// Purpose: a callback class that is notified when a model has finished loading
//-----------------------------------------------------------------------------
abstract_class IModelLoadCallback
{
public:
	virtual void OnModelLoadComplete( const IVModel* pModel ) = 0;

protected:
	// Protected destructor so that nobody tries to delete via this interface.
	// Automatically unregisters if the callback is destroyed while still pending.
	~IModelLoadCallback();
};


//-----------------------------------------------------------------------------
// Purpose: Automate refcount tracking on a model index
//-----------------------------------------------------------------------------
class CRefCountedModelIndex
{
private:
	int m_nIndex;
public:
	CRefCountedModelIndex() : m_nIndex( -1 ) { }
	~CRefCountedModelIndex() { Set( -1 ); }

	CRefCountedModelIndex( const CRefCountedModelIndex& src ) : m_nIndex( -1 ) { Set( src.m_nIndex ); }
	CRefCountedModelIndex& operator=( const CRefCountedModelIndex& src ) { Set( src.m_nIndex ); return *this; }

	explicit CRefCountedModelIndex( int i ) : m_nIndex( -1 ) { Set( i ); }
	CRefCountedModelIndex& operator=( int i ) { Set( i ); return *this; }

	int Get() const { return m_nIndex; }
	void Set( int i );
	void Clear() { Set( -1 ); }

	operator int () const { return m_nIndex; }
};


//-----------------------------------------------------------------------------
// Model info interface
//-----------------------------------------------------------------------------

// change this when the new version is incompatable with the old
#define VMODELINFO_CLIENT_INTERFACE_VERSION		"VModelInfoClient006"
#define VMODELINFO_SERVER_INTERFACE_VERSION_3	"VModelInfoServer003"
#define VMODELINFO_SERVER_INTERFACE_VERSION		"VModelInfoServer004"

// MODEL INDEX RULES
// If index >= 0, then index references the precached model string table
// If index == -1, then the model is invalid
// If index < -1, then the model is DYNAMIC and has a DYNAMIC INDEX of (-2 - index)
// - if the dynamic index is ODD, then the model is CLIENT ONLY
//   and has a m_LocalDynamicModels lookup index of (dynamic index)>>1
// - if the dynamic index is EVEN, then the model is NETWORKED
//   and has a dynamic model string table index of (dynamic index)>>1

//inline bool IsDynamicModelIndex( int modelindex ) { return modelindex < -1; }
//inline bool IsClientOnlyModelIndex( int modelindex ) { return modelindex < -1 && (modelindex & 1); }

abstract_class IVModelInfo
{
public:
	virtual							~IVModelInfo( void ) { }

	// Returns IVModel* pointer for a model given a precached or dynamic model index.
	virtual const IVModel			*GetModel( int modelindex ) const = 0;

	// Returns index of model by name for precached or known dynamic models.
	// Does not adjust reference count for dynamic models.
	virtual int						GetModelIndex( const char *name ) const = 0;

	// Returns name of model
	virtual const char*				GetModelName(int modelindex) const = 0;
	virtual vcollide_t*				GetVCollide(int modelindex) = 0;
	virtual void					GetModelBounds(int modelindex, Vector& mins, Vector& maxs) const = 0;
	virtual	void					GetModelRenderBounds(int modelindex, Vector& mins, Vector& maxs) const = 0;
	virtual int						GetModelFrameCount(int modelindex) const = 0;
	virtual int						GetModelType(int modelindex) const = 0;
	virtual void*					GetModelExtraData(int modelindex) = 0;
	virtual bool					IsTranslucent(int modelindex) const = 0;
	virtual bool					IsTranslucentTwoPass(int modelindex) const = 0;
	virtual void					RecomputeTranslucency(int modelindex, int nSkin, int nBody, void /*IClientRenderable*/* pClientRenderable, float fInstanceAlphaModulate = 1.0f) = 0;
	virtual int						GetModelMaterialCount(int modelindex) const = 0;
	virtual void					GetModelMaterials(int modelindex, int count, IMaterial** ppMaterial) = 0;
	virtual bool					IsModelVertexLit(int modelindex) const = 0;
	virtual const char*				GetModelKeyValueText(int modelindex) = 0;
	virtual bool					GetModelKeyValue(int modelindex, CUtlBuffer& buf) = 0; // supports keyvalue blocks in submodels
	virtual float					GetModelRadius(int modelindex) = 0;

	// Returns name of model
	//virtual const char				*GetModelName( const IVModel *model ) const = 0;
	//virtual vcollide_t				*GetVCollide( const IVModel *model ) = 0;
	//virtual void					GetModelBounds( const IVModel *model, Vector& mins, Vector& maxs ) const = 0;
	//virtual	void					GetModelRenderBounds( const IVModel *model, Vector& mins, Vector& maxs ) const = 0;
	//virtual int						GetModelFrameCount( const IVModel *model ) const = 0;
	//virtual int						GetModelType( const IVModel *model ) const = 0;
	//virtual void					*GetModelExtraData( const IVModel *model ) = 0;
	//virtual bool					IsTranslucent( IVModel const* model ) const = 0;
	//virtual bool					IsTranslucentTwoPass( const IVModel *model ) const = 0;
	//virtual void					RecomputeTranslucency( const IVModel *model, int nSkin, int nBody, void /*IClientRenderable*/ *pClientRenderable, float fInstanceAlphaModulate=1.0f) = 0;
	//virtual int						GetModelMaterialCount( const IVModel* model ) const = 0;
	//virtual void					GetModelMaterials( const IVModel *model, int count, IMaterial** ppMaterial ) = 0;
	//virtual bool					IsModelVertexLit( const IVModel *model ) const = 0;
	//virtual const char				*GetModelKeyValueText( const IVModel *model ) = 0;
	//virtual bool					GetModelKeyValue( const IVModel *model, CUtlBuffer &buf ) = 0; // supports keyvalue blocks in submodels
	//virtual float					GetModelRadius( const IVModel *model ) = 0;

	virtual const studiohdr_t		*FindModel( const studiohdr_t *pStudioHdr, void **cache, const char *modelname ) const = 0;
	virtual const studiohdr_t		*FindModel( void *cache ) const = 0;
	virtual	virtualmodel_t			*GetVirtualModel( const studiohdr_t *pStudioHdr ) const = 0;
	virtual byte					*GetAnimBlock( const studiohdr_t *pStudioHdr, int iBlock ) const = 0;

	// Available on client only!!!
	//virtual void					GetModelMaterialColorAndLighting( const IVModel *model, Vector const& origin,
	//									QAngle const& angles, trace_t* pTrace, 
	//									Vector& lighting, Vector& matColor ) = 0;
	//virtual void					GetIlluminationPoint( const IVModel *model, IClientRenderable *pRenderable, Vector const& origin, 
	//									QAngle const& angles, Vector* pLightingCenter ) = 0;

	virtual int						GetModelContents( int modelIndex ) = 0;
	virtual studiohdr_t*			GetStudiomodel(int modelIndex) = 0;
	virtual int						GetModelSpriteWidth(int modelIndex) const = 0;
	virtual int						GetModelSpriteHeight(int modelIndex) const = 0;

	//virtual studiohdr_t				*GetStudiomodel( const IVModel *mod ) = 0;
	//virtual int						GetModelSpriteWidth( const IVModel *model ) const = 0;
	//virtual int						GetModelSpriteHeight( const IVModel *model ) const = 0;

	

	// both client and server
	virtual int						GetAutoplayList( const studiohdr_t *pStudioHdr, unsigned short **pAutoplayList ) const = 0;

	// Gets a virtual terrain collision model (creates if necessary)
	// NOTE: This may return NULL if the terrain model cannot be virtualized
	virtual CPhysCollide			*GetCollideForVirtualTerrain(IVModel* world, int index ) = 0;


	// Obsolete methods. These are left in to maintain binary compatibility with clients using the IVModelInfo old version.
	//virtual const IVModel			*FindOrLoadModel( const char *name ) { Warning( "IVModelInfo::FindOrLoadModel is now obsolte.\n" ); return NULL; }
	//virtual void					InitDynamicModels( ) { Warning( "IVModelInfo::InitDynamicModels is now obsolte.\n" ); }
	//virtual void					ShutdownDynamicModels( ) { Warning( "IVModelInfo::ShutdownDynamicModels is now obsolte.\n" ); }
	//virtual void					AddDynamicModel( const char *name, int nModelIndex = -1 ) { Warning( "IVModelInfo::AddDynamicModel is now obsolte.\n" ); }
	//virtual void					ReferenceModel( int modelindex ) { Warning( "IVModelInfo::ReferenceModel is now obsolte.\n" ); }
	//virtual void					UnreferenceModel( int modelindex ) { Warning( "IVModelInfo::UnreferenceModel is now obsolte.\n" ); }
	//virtual void					CleanupDynamicModels( bool bForce = false ) { Warning( "IVModelInfo::CleanupDynamicModels is now obsolte.\n" ); }

	virtual MDLHandle_t				GetCacheHandle(int modelIndex) const = 0;
	//virtual MDLHandle_t				GetCacheHandle( const IVModel *model ) const = 0;

	// Returns planes of non-nodraw brush model surfaces
	virtual int						GetBrushModelPlaneCount(int modelIndex) const = 0;
	virtual void					GetBrushModelPlane(int modelIndex, int nIndex, cplane_t& plane, Vector* pOrigin) const = 0;
	//virtual int						GetBrushModelPlaneCount( const IVModel *model ) const = 0;
	//virtual void					GetBrushModelPlane( const IVModel *model, int nIndex, cplane_t &plane, Vector *pOrigin ) const = 0;
	//virtual int						GetSurfacepropsForVirtualTerrain( int index ) = 0;

	// Poked by engine host system
	virtual void					OnLevelChange() = 0;

	//virtual int						GetModelClientSideIndex( const char *name ) const = 0;

	// Returns index of model by name, dynamically registered if not already known.
	//virtual int						RegisterDynamicModel( const char *name, bool bClientSide ) = 0;

	//virtual bool					IsDynamicModelLoading( int modelIndex ) = 0;

	//virtual void					AddRefDynamicModel( int modelIndex ) = 0;
	//virtual void					ReleaseDynamicModel( int modelIndex ) = 0;

	// Registers callback for when dynamic model has finished loading.
	// Automatically adds reference, pair with ReleaseDynamicModel.
	//virtual bool					RegisterModelLoadCallback( int modelindex, IModelLoadCallback* pCallback, bool bCallImmediatelyIfLoaded = true ) = 0;
	//virtual void					UnregisterModelLoadCallback( int modelindex, IModelLoadCallback* pCallback ) = 0;
};

typedef IVModelInfo IVModelInfo003;


abstract_class IVModelInfoClient : virtual public IVModelInfo
{
public:
	virtual void OnDynamicModelsStringTableChange( int nStringIndex, const char *pString, const void *pData ) = 0;

	// For tools only!
	//virtual const IVModel *FindOrLoadModel( const char *name ) = 0;

	virtual void GetModelMaterialColorAndLighting(IVModel* pWorld, int modelIndex, const Vector& origin,
		const QAngle& angles, trace_t* pTrace, Vector& lighting, Vector& matColor) = 0;
	virtual void GetIlluminationPoint(int modelIndex, IClientRenderable* pRenderable, Vector const& origin,
		QAngle const& angles, Vector* pLightingCenter) = 0;
	virtual bool IsUsingFBTexture(int modelIndex, int nSkin, int nBody, void /*IClientRenderable*/* pClientRenderable) const = 0;
	virtual bool					ModelHasMaterialProxy(int modelIndex) const = 0;

	//virtual void GetModelMaterialColorAndLighting(const IVModel* model, const Vector& origin,
	//	const QAngle& angles, trace_t* pTrace, Vector& lighting, Vector& matColor) = 0;
	//virtual void GetIlluminationPoint(const IVModel* model, IClientRenderable* pRenderable, Vector const& origin,
	//	QAngle const& angles, Vector* pLightingCenter) = 0;
	//virtual bool IsUsingFBTexture(const IVModel* model, int nSkin, int nBody, void /*IClientRenderable*/* pClientRenderable) const = 0;
	//virtual bool					ModelHasMaterialProxy(const IVModel* model) const = 0;

};


struct virtualterrainparams_t
{
	// UNDONE: Add grouping here, specified in BSP file? (test grouping to see if this is necessary)
	int index;
};


//-----------------------------------------------------------------------------
// Purpose: Force removal from callback list on destruction to avoid crashes.
//-----------------------------------------------------------------------------
inline IModelLoadCallback::~IModelLoadCallback()
{
#ifdef CLIENT_DLL
	extern IVModelInfoClient *modelinfo;
#else
	extern IVModelInfo *modelinfo;
#endif
	if ( modelinfo )
	{
		//modelinfo->UnregisterModelLoadCallback( -1, this );
	}
}



//-----------------------------------------------------------------------------
// Purpose: Automate refcount tracking on a model index
//-----------------------------------------------------------------------------
inline void CRefCountedModelIndex::Set( int i )
{
#ifdef CLIENT_DLL
	extern IVModelInfoClient *modelinfo;
#else
	extern IVModelInfo *modelinfo;
#endif
	if ( i == m_nIndex )
		return;
	//modelinfo->AddRefDynamicModel( i );
	//modelinfo->ReleaseDynamicModel( m_nIndex );
	m_nIndex = i;
}


#endif // IVMODELINFO_H
