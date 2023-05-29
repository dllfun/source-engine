//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $NoKeywords: $
//===========================================================================//
#include "engine/ivmodelinfo.h"
#include "filesystem.h"
#include "gl_model_private.h"
#include "modelloader.h"
#include "l_studio.h"
#include "cmodel_engine.h"
#include "server.h"
#include "r_local.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterial.h"
#include "lightcache.h"
#include "istudiorender.h"
#include "utldict.h"
#include "filesystem_engine.h"
#include "client.h"
#include "sys_dll.h"
#include "gl_rsurf.h"
#include "utlvector.h"
#include "utlhashtable.h"
#include "utlsymbol.h"
#include "ModelInfo.h"
#include "networkstringtable.h" // for Lock()

#ifndef SWDS
#include "demo.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Gets the lighting center
//-----------------------------------------------------------------------------
static void R_StudioGetLightingCenter( IClientRenderable *pRenderable, studiohdr_t* pStudioHdr, const Vector& origin,
								const QAngle &angles, Vector* pLightingOrigin )
{
	Assert( pLightingOrigin );
	matrix3x4_t matrix;
	AngleMatrix( angles, origin, matrix );
	R_ComputeLightingOrigin( pRenderable, pStudioHdr, matrix, *pLightingOrigin );
}

static int R_StudioBodyVariations( studiohdr_t *pstudiohdr )
{
	mstudiobodyparts_t *pbodypart;
	int i, count;

	if ( !pstudiohdr )
		return 0;

	count = 1;
	pbodypart = pstudiohdr->pBodypart( 0 );

	// Each body part has nummodels variations so there are as many total variations as there
	// are in a matrix of each part by each other part
	for ( i = 0; i < pstudiohdr->numbodyparts; i++ )
	{
		count = count * pbodypart[i].nummodels;
	}
	return count;
}

int model_t::ModelFrameCount() const//model_t *model
{
	int count = 1;

	//if ( !model )
	//	return count;

	if ( type == mod_sprite )//model->
	{
		return sprite.numframes;//model->
	}
	else if ( type == mod_studio )//model->
	{
		count = R_StudioBodyVariations( ( studiohdr_t * )modelloader->GetExtraData( this ) );//model
	}

	if ( count < 1 )
		count = 1;
	
	return count;
}

//-----------------------------------------------------------------------------
// private extension of CNetworkStringTable to correct lack of Lock retval
//-----------------------------------------------------------------------------
class CNetworkStringTable_LockOverride : public CNetworkStringTable
{
private:
//	CNetworkStringTable_LockOverride(); // no impl
//	~CNetworkStringTable_LockOverride(); // no impl
	CNetworkStringTable_LockOverride(const CNetworkStringTable_LockOverride &); // no impl
	CNetworkStringTable_LockOverride& operator=(const CNetworkStringTable_LockOverride &); // no impl
public:
	bool LockWithRetVal( bool bLock ) { bool bWasLocked = m_bLocked; Lock(bLock); return bWasLocked; }
};


//-----------------------------------------------------------------------------
// shared implementation of IVModelInfo
//-----------------------------------------------------------------------------
abstract_class CModelInfo //: virtual public IVModelInfo
{
public:
	
	//virtual int GetModelClientSideIndex( const char *name ) const;

	//virtual bool RegisterModelLoadCallback( int modelindex, IModelLoadCallback* pCallback, bool bCallImmediatelyIfLoaded );
	//virtual void UnregisterModelLoadCallback( int modelindex, IModelLoadCallback* pCallback );
	//virtual bool IsDynamicModelLoading( int modelIndex );
	//virtual void AddRefDynamicModel(int modelIndex);
	//virtual void ReleaseDynamicModel( int modelIndex );

	virtual void OnLevelChange();

	virtual const char* GetModelName(int modelIndex) const;
	virtual void GetModelBounds(int modelIndex, Vector& mins, Vector& maxs) const;
	virtual void GetModelRenderBounds(int modelIndex, Vector& mins, Vector& maxs) const;
	virtual int GetModelFrameCount(int modelIndex) const;
	virtual int GetModelType(int modelIndex) const;
	virtual void* GetModelExtraData(int modelIndex);
	virtual bool IsTranslucent(int modelIndex) const;
	virtual bool IsModelVertexLit(int modelIndex) const;
	virtual bool IsTranslucentTwoPass(int modelIndex) const;
	virtual void RecomputeTranslucency(int modelIndex, int nSkin, int nBody, void /*IClientRenderable*/* pClientRenderable, float fInstanceAlphaModulate);
	virtual int	GetModelMaterialCount(int modelIndex) const;
	virtual void GetModelMaterials(int modelIndex, int count, IMaterial** ppMaterials);
	//virtual void GetIlluminationPoint( int modelIndex, IClientRenderable *pRenderable, const Vector& origin, 
	//	const QAngle& angles, Vector* pLightingOrigin );
	virtual int GetModelContents(int modelIndex);
	vcollide_t* GetVCollide(int modelIndex);
	virtual const char* GetModelKeyValueText(int modelIndex);
	virtual bool GetModelKeyValue(int modelIndex, CUtlBuffer& buf);
	virtual float GetModelRadius(int modelIndex);
	virtual studiohdr_t* GetStudiomodel(int modelIndex);
	virtual int GetModelSpriteWidth(int modelIndex) const;
	virtual int GetModelSpriteHeight(int modelIndex) const;

	virtual const char *GetModelName( const model_t *model ) const;
	virtual void GetModelBounds( const model_t *model, Vector& mins, Vector& maxs ) const;
	virtual void GetModelRenderBounds( const model_t *model, Vector& mins, Vector& maxs ) const;
	virtual int GetModelFrameCount( const model_t *model ) const;
	virtual int GetModelType( const model_t *model ) const;
	virtual void *GetModelExtraData( const model_t *model );
	virtual bool IsTranslucent( const model_t *model ) const;
	virtual bool IsModelVertexLit( const model_t *model ) const;
	virtual bool IsTranslucentTwoPass( const model_t *model ) const;
	virtual void RecomputeTranslucency( const model_t *model, int nSkin, int nBody, void /*IClientRenderable*/ *pClientRenderable, float fInstanceAlphaModulate);
	virtual int	GetModelMaterialCount( const model_t *model ) const;
	virtual void GetModelMaterials( const model_t *model, int count, IMaterial** ppMaterials );
	//virtual void GetIlluminationPoint( const model_t *model, IClientRenderable *pRenderable, const Vector& origin, 
	//	const QAngle& angles, Vector* pLightingOrigin );
	virtual int GetModelContents(const model_t* model);
	vcollide_t *GetVCollide(const IVModel *model );
	virtual const char *GetModelKeyValueText( const model_t *model );
	virtual bool GetModelKeyValue( const model_t *model, CUtlBuffer &buf );
	virtual float GetModelRadius( const model_t *model );
	virtual studiohdr_t *GetStudiomodel( const model_t *mod );
	virtual int GetModelSpriteWidth( const model_t *model ) const;
	virtual int GetModelSpriteHeight( const model_t *model ) const;

	virtual const studiohdr_t *FindModel( const studiohdr_t *pStudioHdr, void **cache, char const *modelname ) const;
	virtual const studiohdr_t *FindModel( void *cache ) const;
	virtual virtualmodel_t *GetVirtualModel( const studiohdr_t *pStudioHdr ) const;
	virtual byte *GetAnimBlock( const studiohdr_t *pStudioHdr, int iBlock ) const;

	//byte *LoadAnimBlock( model_t *model, const studiohdr_t *pStudioHdr, int iBlock, cache_user_t *cache ) const;

	int GetAutoplayList( const studiohdr_t *pStudioHdr, unsigned short **pAutoplayList ) const;
	CPhysCollide *GetCollideForVirtualTerrain(IVModel* world, int index );
	//virtual int GetSurfacepropsForVirtualTerrain( int index ) { return CM_SurfacepropsForDisp(index); }

	virtual MDLHandle_t	GetCacheHandle(int modelIndex) const { return GetCacheHandle(GetModel(modelIndex)); }
	virtual MDLHandle_t	GetCacheHandle(const model_t* model) const { return model->GetCacheHandle(); }

	// Returns planes of non-nodraw brush model surfaces
	virtual int GetBrushModelPlaneCount(int modelIndex) const;
	virtual void GetBrushModelPlane(int modelIndex, int nIndex, cplane_t& plane, Vector* pOrigin) const;
	virtual int GetBrushModelPlaneCount( const model_t *model ) const;
	virtual void GetBrushModelPlane( const model_t *model, int nIndex, cplane_t &plane, Vector *pOrigin ) const;

protected:
	//static int CLIENTSIDE_TO_MODEL( int i ) { return i >= 0 ? (-2 - (i*2 + 1)) : -1; }
	//static int NETDYNAMIC_TO_MODEL( int i ) { return i >= 0 ? (-2 - (i*2)) : -1; }
	//static int MODEL_TO_CLIENTSIDE( int i ) { return ( i <= -2 && (i & 1) ) ? (-2 - i) >> 1 : -1; }
	//static int MODEL_TO_NETDYNAMIC( int i ) { return ( i <= -2 && !(i & 1) ) ? (-2 - i) >> 1 : -1; }

	//model_t *LookupDynamicModel( int i );

	//virtual INetworkStringTable *GetDynamicModelStringTable() const = 0;
	// GetModel, RegisterDynamicModel(name) are in CModelInfoClient/CModelInfoServer
	virtual int GetModelIndex(const char* name) const;
	virtual int LookupPrecachedModelIndex( const char *name ) const = 0;
	virtual const model_t* GetModel(int modelindex) const = 0;
	//void GrowNetworkedDynamicModels( int netidx )
	//{
	//	if ( m_NetworkedDynamicModels.Count() <= netidx )
	//	{
	//		int origCount = m_NetworkedDynamicModels.Count();
	//		m_NetworkedDynamicModels.SetCountNonDestructively( netidx + 1 );
	//		for ( int i = origCount; i <= netidx; ++i )
	//		{
	//			m_NetworkedDynamicModels[i] = NULL;
	//		}
	//	}
	//}

	// Networked dynamic model indices are lookup indices for this vector
	//CUtlVector< model_t* > m_NetworkedDynamicModels;

public:
	struct ModelFileHandleHash
	{
		uint operator()( model_t *p ) const { return PointerHashFunctor()( p->GetFnHandle() ); }
		uint operator()( FileNameHandle_t fn ) const { return PointerHashFunctor()( fn ); }
	};
	struct ModelFileHandleEq
	{
		bool operator()( model_t *a, model_t *b ) const { return a == b; }
		bool operator()( model_t *a, FileNameHandle_t b ) const { return a->GetFnHandle() == b; }
	};
protected:
	// Client-only dynamic model indices are iterators into this struct (only populated by CModelInfoClient subclass)
	//CUtlStableHashtable< model_t*, empty_t, ModelFileHandleHash, ModelFileHandleEq, int16, FileNameHandle_t > m_ClientDynamicModels;
};

int CModelInfo::GetModelIndex( const char *name ) const
{
	if ( !name )
		return -1;

	// Order of preference: precached, networked, client-only.
	int nIndex = LookupPrecachedModelIndex( name );
	if ( nIndex != -1 )
		return nIndex;

//	INetworkStringTable* pTable = GetDynamicModelStringTable();
//	if ( pTable )
//	{
//		int netdyn = pTable->FindStringIndex( name );
//		if ( netdyn != INVALID_STRING_INDEX )
//		{
//			Assert( !m_NetworkedDynamicModels.IsValidIndex( netdyn ) || V_strcmp( m_NetworkedDynamicModels[netdyn]->strName, name ) == 0 );
//			return NETDYNAMIC_TO_MODEL( netdyn );
//		}
//
//#if defined( DEMO_BACKWARDCOMPATABILITY ) && !defined( SWDS )
//		// dynamic model tables in old system did not have a full path with "models/" prefix
//		if ( V_strnicmp( name, "models/", 7 ) == 0 && demoplayer && demoplayer->IsPlayingBack() && demoplayer->GetProtocolVersion() < PROTOCOL_VERSION_20 )
//		{
//			netdyn = pTable->FindStringIndex( name + 7 );
//			if ( netdyn != INVALID_STRING_INDEX )
//			{
//				Assert( !m_NetworkedDynamicModels.IsValidIndex( netdyn ) || V_strcmp( m_NetworkedDynamicModels[netdyn]->strName, name ) == 0 );
//				return NETDYNAMIC_TO_MODEL( netdyn );
//			}
//		}
//#endif
//	}

	return -1;// GetModelClientSideIndex(name);
}

//int CModelInfo::GetModelClientSideIndex( const char *name ) const
//{
	//if ( m_ClientDynamicModels.Count() != 0 )
	//{
	//	FileNameHandle_t file = g_pFullFileSystem->FindFileName( name );
	//	if ( file != FILENAMEHANDLE_INVALID )
	//	{
	//		UtlHashHandle_t h = m_ClientDynamicModels.Find( file );
	//		if ( h != m_ClientDynamicModels.InvalidHandle() )
	//		{
	//			Assert( V_strcmp( m_ClientDynamicModels[h]->strName, name ) == 0 );
	//			return CLIENTSIDE_TO_MODEL( h );
	//		}
	//	}
	//}

//	return -1;
//}

//model_t *CModelInfo::LookupDynamicModel( int i )
//{
//	Assert( IsDynamicModelIndex( i ) );
//	if ( IsClientOnlyModelIndex( i ) )
//	{
//		//UtlHashHandle_t h = (UtlHashHandle_t) MODEL_TO_CLIENTSIDE( i );
//		return NULL;// m_ClientDynamicModels.IsValidHandle(h) ? m_ClientDynamicModels[h] : NULL;
//	}
//	else
//	{
//		//int netidx = MODEL_TO_NETDYNAMIC( i );
//		//if ( m_NetworkedDynamicModels.IsValidIndex( netidx ) && m_NetworkedDynamicModels[ netidx ] )
//		//	return m_NetworkedDynamicModels[ netidx ];
//
////		INetworkStringTable *pTable = GetDynamicModelStringTable();
////		if ( pTable && (uint) netidx < (uint) pTable->GetNumStrings() )
////		{
////			GrowNetworkedDynamicModels( netidx );
////			const char *name = pTable->GetString( netidx );
////
////#if defined( DEMO_BACKWARDCOMPATABILITY ) && !defined( SWDS )
////			// dynamic model tables in old system did not have a full path with "models/" prefix
////			char fixupBuf[MAX_PATH];
////			if ( V_strnicmp( name, "models/", 7 ) != 0 && demoplayer && demoplayer->IsPlayingBack() && demoplayer->GetProtocolVersion() < PROTOCOL_VERSION_20 )
////			{
////				V_snprintf( fixupBuf, MAX_PATH, "models/%s", name );
////				name = fixupBuf;
////			}
////#endif
////
////			model_t *pModel = modelloader->GetDynamicModel( name, false );
////			m_NetworkedDynamicModels[ netidx ] = pModel;
////			return pModel;
////		}
//
//		return NULL;
//	}
//}


//bool CModelInfo::RegisterModelLoadCallback( int modelIndex, IModelLoadCallback* pCallback, bool bCallImmediatelyIfLoaded )
//{
//	const model_t *pModel = GetModel( modelIndex );
//	Assert( pModel );
//	//if ( pModel && IsDynamicModelIndex( modelIndex ) )
//	//{
//	//	return modelloader->RegisterModelLoadCallback( const_cast< model_t *>( pModel ), IsClientOnlyModelIndex( modelIndex ), pCallback, bCallImmediatelyIfLoaded );
//	//}
//	//else 
//	if ( pModel && bCallImmediatelyIfLoaded )
//	{
//		pCallback->OnModelLoadComplete( pModel );
//		return true;
//	}
//	return false;
//}

//void CModelInfo::UnregisterModelLoadCallback( int modelIndex, IModelLoadCallback* pCallback )
//{
//	if ( modelIndex == -1 )
//	{
//		modelloader->UnregisterModelLoadCallback( NULL, false, pCallback );
//		modelloader->UnregisterModelLoadCallback( NULL, true, pCallback );
//	}
//	//else if ( IsDynamicModelIndex( modelIndex ) )
//	//{
//	//	//const model_t *pModel = LookupDynamicModel( modelIndex );
//	//	//Assert( pModel );
//	//	//if ( pModel )
//	//	//{
//	//	//	modelloader->UnregisterModelLoadCallback( const_cast< model_t *>( pModel ), IsClientOnlyModelIndex( modelIndex ), pCallback );
//	//	//
//	//}
//}


//bool CModelInfo::IsDynamicModelLoading( int modelIndex )
//{
//	//model_t *pModel = LookupDynamicModel( modelIndex );
//	return NULL;// pModel&& modelloader->IsDynamicModelLoading(pModel, IsClientOnlyModelIndex(modelIndex));
//}


//void CModelInfo::AddRefDynamicModel( int modelIndex )
//{
//	if ( IsDynamicModelIndex( modelIndex ) )
//	{
//		//model_t *pModel = LookupDynamicModel( modelIndex );
//		//Assert( pModel );
//		//if ( pModel )
//		//{
//		//	modelloader->AddRefDynamicModel( pModel, IsClientOnlyModelIndex( modelIndex ) );
//		//}
//	}
//}

//void CModelInfo::ReleaseDynamicModel( int modelIndex )
//{
//	if ( IsDynamicModelIndex( modelIndex ) )
//	{
//		//model_t *pModel = LookupDynamicModel( modelIndex );
//		//Assert( pModel );
//		//if ( pModel )
//		//{
//		//	modelloader->ReleaseDynamicModel( pModel, IsClientOnlyModelIndex( modelIndex ) );
//		//}
//	}
//}

void CModelInfo::OnLevelChange()
{
	// Network string table has reset
	//m_NetworkedDynamicModels.Purge();

	// Force-unload any server-side models
	modelloader->ForceUnloadNonClientDynamicModels();
}

const char* model_t::GetModelName() const
{
	//if (!pModel)
	//{
	//	return "?";
	//}

	return modelloader->GetName(this);
}

const char* CModelInfo::GetModelName(int modelIndex) const
{
	return GetModelName(GetModel(modelIndex));
}

const char *CModelInfo::GetModelName( const model_t *pModel ) const
{
	if ( !pModel )
	{
		return "?";
	}

	return pModel->GetModelName();
}

void model_t::GetModelBounds(Vector& mins, Vector& maxs) const//const model_t* model, 
{
	VectorCopy(this->mins, mins);
	VectorCopy(this->maxs, maxs);
}

void CModelInfo::GetModelBounds(int modelIndex, Vector& mins, Vector& maxs) const
{
	GetModelBounds(GetModel(modelIndex), mins, maxs);
}

void CModelInfo::GetModelBounds( const model_t *model, Vector& mins, Vector& maxs ) const
{
	model->GetModelBounds(mins, maxs);
}

void model_t::GetModelRenderBounds(Vector& mins, Vector& maxs) const//const model_t* model, 
{
	//if (!model)
	//{
	//	mins.Init(0, 0, 0);
	//	maxs.Init(0, 0, 0);
	//	return;
	//}

	switch (type)//model->
	{
	case mod_studio:
	{
		studiohdr_t* pStudioHdr = (studiohdr_t*)modelloader->GetExtraData((model_t*)this);
		Assert(pStudioHdr);

		// NOTE: We're not looking at the sequence box here, although we could
		if (!VectorCompare(vec3_origin, pStudioHdr->view_bbmin) || !VectorCompare(vec3_origin, pStudioHdr->view_bbmax))
		{
			// clipping bounding box
			VectorCopy(pStudioHdr->view_bbmin, mins);
			VectorCopy(pStudioHdr->view_bbmax, maxs);
		}
		else
		{
			// movement bounding box
			VectorCopy(pStudioHdr->hull_min, mins);
			VectorCopy(pStudioHdr->hull_max, maxs);
		}
	}
	break;

	case mod_brush:
		VectorCopy(mins, mins);//model->
		VectorCopy(maxs, maxs);//model->
		break;

	default:
		mins.Init(0, 0, 0);
		maxs.Init(0, 0, 0);
		break;
	}
}

void CModelInfo::GetModelRenderBounds(int modelIndex, Vector& mins, Vector& maxs) const {
	GetModelRenderBounds(GetModel(modelIndex), mins, maxs);
}

void CModelInfo::GetModelRenderBounds( const model_t *model, Vector& mins, Vector& maxs ) const
{
	if (!model)
	{
		mins.Init(0,0,0);
		maxs.Init(0,0,0);
		return;
	}

	model->GetModelRenderBounds(mins, maxs);
}

int model_t::GetModelSpriteWidth() const//const model_t* model
{
	// We must be a sprite to make this query
	if (type != mod_sprite)
		return 0;

	return sprite.width;
}

int CModelInfo::GetModelSpriteWidth(int modelIndex) const {
	return GetModelSpriteWidth(GetModel(modelIndex));
}

int CModelInfo::GetModelSpriteWidth( const model_t *model ) const
{
	return model->GetModelSpriteWidth();
}

int model_t::GetModelSpriteHeight() const//const model_t* model
{
	// We must be a sprite to make this query
	if (type != mod_sprite)
		return 0;

	return sprite.height;
}

int CModelInfo::GetModelSpriteHeight(int modelIndex) const
{
	return GetModelSpriteHeight(GetModel(modelIndex));
}

int CModelInfo::GetModelSpriteHeight( const model_t *model ) const
{
	return model->GetModelSpriteHeight();
}

int CModelInfo::GetModelFrameCount(int modelIndex) const
{
	return GetModelFrameCount(GetModel(modelIndex));
}

int CModelInfo::GetModelFrameCount( const model_t *model ) const
{
	return model == NULL? 1 : model->ModelFrameCount(  );//(model_t*)
}

int model_t::GetModelType() const//const model_t* model
{
	//if (!model)
	//	return -1;

	if (type == mod_bad)//model->
	{
		//if ( m_ClientDynamicModels.Find( (model_t*) model ) != m_ClientDynamicModels.InvalidHandle() )
		//	return mod_studio;
//		INetworkStringTable* pTable = GetDynamicModelStringTable();
//		if ( pTable && pTable->FindStringIndex( model->strName ) != INVALID_STRING_INDEX )
//			return mod_studio;
//
//#if defined( DEMO_BACKWARDCOMPATABILITY ) && !defined( SWDS )
//		// dynamic model tables in old system did not have a full path with "models/" prefix
//		if ( pTable && demoplayer && demoplayer->IsPlayingBack() && demoplayer->GetProtocolVersion() < PROTOCOL_VERSION_20 &&
//			 V_strnicmp( model->strName, "models/", 7 ) == 0 && pTable->FindStringIndex( model->strName + 7 ) != INVALID_STRING_INDEX )
//		{
//			return mod_studio;
//		}
//#endif
	}

	return type;//model->
}

int CModelInfo::GetModelType(int modelIndex) const
{
	return GetModelType(GetModel(modelIndex));
}

int CModelInfo::GetModelType( const model_t *model ) const
{
	if ( !model )
		return -1;

	return model->GetModelType();
}

void* model_t::GetModelExtraData() const//const model_t* model
{
	return modelloader->GetExtraData((model_t*)this);
}

void* CModelInfo::GetModelExtraData(int modelIndex)
{
	return GetModelExtraData(GetModel(modelIndex));
}

void *CModelInfo::GetModelExtraData( const model_t *model )
{
	return model->GetModelExtraData();
}


//-----------------------------------------------------------------------------
// Purpose: Translate "cache" pointer into model_t, or lookup model by name
//-----------------------------------------------------------------------------
const studiohdr_t *CModelInfo::FindModel( const studiohdr_t *pStudioHdr, void **cache, char const *modelname ) const
{
	const model_t *model = (model_t *)*cache;

	if (!model)
	{
		// FIXME: what do I pass in here?
		model = modelloader->GetModelForName( modelname, IModelLoader::FMODELLOADER_SERVER );
		*cache = (void *)model;
	}

	return (const studiohdr_t *)modelloader->GetExtraData( (model_t *)model );
}


//-----------------------------------------------------------------------------
// Purpose: Translate "cache" pointer into model_t
//-----------------------------------------------------------------------------
const studiohdr_t *CModelInfo::FindModel( void *cache ) const
{
	return g_pMDLCache->GetStudioHdr( VoidPtrToMDLHandle( cache ) );
}


//-----------------------------------------------------------------------------
// Purpose: Return virtualmodel_t block associated with model_t
//-----------------------------------------------------------------------------
virtualmodel_t *CModelInfo::GetVirtualModel( const studiohdr_t *pStudioHdr ) const
{
	//MDLHandle_t handle = VoidPtrToMDLHandle( pStudioHdr->VirtualModel() );
	//return g_pMDLCache->GetVirtualModelFast( pStudioHdr, handle );
	return pStudioHdr->GetVirtualModel();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
byte *CModelInfo::GetAnimBlock( const studiohdr_t *pStudioHdr, int nBlock ) const
{
	MDLHandle_t handle = VoidPtrToMDLHandle( pStudioHdr->VirtualModel() );
	return g_pMDLCache->GetAnimBlock( handle, nBlock );
}

int CModelInfo::GetAutoplayList( const studiohdr_t *pStudioHdr, unsigned short **pAutoplayList ) const
{
	MDLHandle_t handle = VoidPtrToMDLHandle( pStudioHdr->VirtualModel() );
	return g_pMDLCache->GetAutoplayList( handle, pAutoplayList );
}


//-----------------------------------------------------------------------------
// Purpose: bind studiohdr_t support functions to engine
// FIXME: This should be moved into studio.cpp?
//-----------------------------------------------------------------------------
const studiohdr_t *studiohdr_t::FindModel( void **cache, char const *pModelName ) const
{
	MDLHandle_t handle = g_pMDLCache->FindMDL( pModelName );
	*cache = (void*)(uintp)handle;
	return g_pMDLCache->GetStudioHdr( handle );
}

virtualmodel_t *studiohdr_t::GetVirtualModel( void ) const
{
	if ( numincludemodels == 0 )
		return NULL;
	return g_pMDLCache->GetVirtualModelFast( this, VoidPtrToMDLHandle( VirtualModel() ) );
}

byte *studiohdr_t::GetAnimBlock( int i ) const
{
	return g_pMDLCache->GetAnimBlock( VoidPtrToMDLHandle( VirtualModel() ), i );
}

int	studiohdr_t::GetAutoplayList( unsigned short **pOut ) const
{
	return g_pMDLCache->GetAutoplayList( VoidPtrToMDLHandle( VirtualModel() ), pOut );
}

const studiohdr_t *virtualgroup_t::GetStudioHdr( void ) const
{
	return g_pMDLCache->GetStudioHdr( VoidPtrToMDLHandle( cache ) );
}

bool model_t::IsTranslucent() const
{
	return (flags & MODELFLAG_TRANSLUCENT);//model && (model-> )
}

bool CModelInfo::IsTranslucent(int modelIndex) const
{
	return IsTranslucent(GetModel(modelIndex));
}

bool CModelInfo::IsTranslucent( const model_t *model ) const
{
	return (model && (model->IsTranslucent()));
}

bool model_t::IsModelVertexLit() const//const model_t* model
{
	// Should we add skin & model to this function like IsUsingFBTexture()?
	return flags & MODELFLAG_VERTEXLIT;//(model && (model-> ))
}

bool CModelInfo::IsModelVertexLit(int modelIndex) const
{
	// Should we add skin & model to this function like IsUsingFBTexture()?
	return IsModelVertexLit(GetModel(modelIndex));
}

bool CModelInfo::IsModelVertexLit( const model_t *model ) const
{
	// Should we add skin & model to this function like IsUsingFBTexture()?
	return (model && (model->IsModelVertexLit()));
}

bool model_t::IsTranslucentTwoPass() const//const model_t* model
{
	return flags & MODELFLAG_TRANSLUCENT_TWOPASS;//(model && (model-> ))
}

bool CModelInfo::IsTranslucentTwoPass(int modelIndex) const
{
	return IsTranslucentTwoPass(GetModel(modelIndex));
}

bool CModelInfo::IsTranslucentTwoPass( const model_t *model ) const
{
	return (model && (model->IsTranslucentTwoPass()));
}

MDLHandle_t	model_t::GetCacheHandle() const
{ return (this->GetModelType() == mod_studio) ? this->GetStudio() : MDLHANDLE_INVALID; }

void CModelInfo::RecomputeTranslucency(int modelIndex, int nSkin, int nBody, void /*IClientRenderable*/* pClientRenderable, float fInstanceAlphaModulate)
{
	RecomputeTranslucency(GetModel(modelIndex), nSkin, nBody, pClientRenderable, fInstanceAlphaModulate);
}

void CModelInfo::RecomputeTranslucency( const model_t *model, int nSkin, int nBody, void /*IClientRenderable*/ *pClientRenderable, float fInstanceAlphaModulate )
{
	if ( model != NULL )
	{
		((model_t*)model)->Mod_RecomputeTranslucency( nSkin, nBody, pClientRenderable, fInstanceAlphaModulate );
	}
}

int	CModelInfo::GetModelMaterialCount(int modelIndex) const
{
	return GetModelMaterialCount(GetModel(modelIndex));
}

int	CModelInfo::GetModelMaterialCount( const model_t *model ) const
{
	if (!model)
		return 0;
	return ((model_t*)model)->Mod_GetMaterialCount();
}

void CModelInfo::GetModelMaterials(int modelIndex, int count, IMaterial** ppMaterials)
{
	GetModelMaterials(GetModel(modelIndex), count, ppMaterials);
}

void CModelInfo::GetModelMaterials( const model_t *model, int count, IMaterial** ppMaterials )
{
	if (model)
		((model_t*)model)->Mod_GetModelMaterials( count, ppMaterials );
}

int model_t::GetModelContents() const
{
	switch (this->GetModelType())
	{
	case mod_brush:
		return CM_InlineModelContents((model_t*)this);//need check

		// BUGBUG: Studio contents?
	case mod_studio:
		return CONTENTS_SOLID;
	}
	return 0;
}

int CModelInfo::GetModelContents(int modelIndex)
{
	const model_t* pModel = GetModel(modelIndex);
	return GetModelContents(pModel);
}

int CModelInfo::GetModelContents(const model_t* pModel )
{
	if ( pModel )
	{
		return pModel->GetModelContents();
	}
	return 0;
}

#if !defined( _RETAIL )
extern double g_flAccumulatedModelLoadTimeVCollideSync;
#endif

vcollide_t* model_t::GetVCollide() const
{

	if (this->GetModelType() == mod_studio)
	{
#if !defined( _RETAIL )
		double t1 = Plat_FloatTime();
#endif
		vcollide_t* col = g_pMDLCache->GetVCollide(((model_t*)this)->GetStudio());
#if !defined( _RETAIL )
		double t2 = Plat_FloatTime();
		g_flAccumulatedModelLoadTimeVCollideSync += (t2 - t1);
#endif
		return col;
	}
	else if (this->GetModelType() == mod_brush) {
		//int modelIndex = GetModelIndex(pModel->GetModelName());
		//if (modelIndex >= 0)
		//{
		return CM_GetVCollide((model_t*)this);//need check , modelIndex - 1
		//}
	}

	return NULL;
}

vcollide_t* CModelInfo::GetVCollide(int modelIndex)
{
	// First model (index 0 )is is empty
	// Second model( index 1 ) is the world, then brushes/submodels, then players, etc.
	// So, we must subtract 1 from the model index to map modelindex to CM_ index
	// in cmodels, 0 is the world, then brushes, etc.
	if (modelIndex < MAX_MODELS)
	{
		const model_t* pModel = GetModel(modelIndex);
		if (pModel)
		{
			return GetVCollide(pModel);
		}
	//	else
	//	{
	//		// we may have the cmodels loaded and not know the model/mod->type yet
	//		return CM_GetVCollide((model_t*)pModel);//need check , modelIndex - 1
	//	}
	}
	return NULL;
}

vcollide_t *CModelInfo::GetVCollide(const IVModel *pModel )
{
	if ( !pModel )
		return NULL;

	return pModel->GetVCollide();
}

const char* model_t::GetModelKeyValueText() const//const model_t* model
{
	if (type != mod_studio)//!model || model->
		return NULL;

	studiohdr_t* pStudioHdr = g_pMDLCache->GetStudioHdr(this->studio);
	if (!pStudioHdr)
		return NULL;

	return pStudioHdr->KeyValueText();
}

// Client must instantiate a KeyValues, which will be filled by this method
const char* CModelInfo::GetModelKeyValueText(int modelIndex)
{
	return GetModelKeyValueText(GetModel(modelIndex));
}

// Client must instantiate a KeyValues, which will be filled by this method
const char *CModelInfo::GetModelKeyValueText( const model_t *model )
{
	if (!model || model->GetModelType() != mod_studio)
		return NULL;
	return model->GetModelKeyValueText();
}

bool model_t::GetModelKeyValue(CUtlBuffer& buf) const
{
	if (this->GetModelType() != mod_studio)
		return false;

	studiohdr_t* pStudioHdr = g_pMDLCache->GetStudioHdr(this->GetStudio());
	if (!pStudioHdr)
		return false;

	if (pStudioHdr->numincludemodels == 0)
	{
		buf.PutString(pStudioHdr->KeyValueText());
		return true;
	}

	virtualmodel_t* pVM = pStudioHdr->GetVirtualModel();

	if (pVM)
	{
		for (int i = 0; i < pVM->m_group.Count(); i++)
		{
			const studiohdr_t* pSubStudioHdr = pVM->m_group[i].GetStudioHdr();
			if (pSubStudioHdr && pSubStudioHdr->KeyValueText())
			{
				buf.PutString(pSubStudioHdr->KeyValueText());
			}
		}
	}
	return true;
}

bool CModelInfo::GetModelKeyValue(int modelIndex, CUtlBuffer& buf)
{
	return GetModelKeyValue(GetModel(modelIndex),buf);
}

bool CModelInfo::GetModelKeyValue( const model_t *model, CUtlBuffer &buf )
{
	if (!model || model->GetModelType() != mod_studio)
		return false;
	return model->GetModelKeyValue(buf);
}

float CModelInfo::GetModelRadius(int modelIndex)
{
	return GetModelRadius(GetModel(modelIndex));
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *model - 
// Output : float
//-----------------------------------------------------------------------------
float CModelInfo::GetModelRadius( const model_t *model )
{
	if ( !model )
		return 0.0f;
	return model->GetRadius();
}

studiohdr_t* model_t::GetStudiomodel() const//const model_t* model
{
	if (type == mod_studio)//model->
		return g_pMDLCache->GetStudioHdr(studio);//model->

	return NULL;
}

studiohdr_t* CModelInfo::GetStudiomodel(int modelIndex)
{
	return GetStudiomodel(GetModel(modelIndex));
}

//-----------------------------------------------------------------------------
// Lovely studiohdrs
//-----------------------------------------------------------------------------
studiohdr_t *CModelInfo::GetStudiomodel( const model_t *model )
{
	return model->GetStudiomodel();
}

CPhysCollide* model_t::GetCollideForVirtualTerrain(int index) const
{
	return CM_PhysCollideForDisp((model_t*)this, index);
}

CPhysCollide *CModelInfo::GetCollideForVirtualTerrain(IVModel* world, int index )
{
	return CM_PhysCollideForDisp((model_t*)world, index );
}

int CModelInfo::GetBrushModelPlaneCount(int modelIndex) const
{
	return GetBrushModelPlaneCount(GetModel(modelIndex));
}

// Returns planes of non-nodraw brush model surfaces
int CModelInfo::GetBrushModelPlaneCount( const model_t *model ) const
{
	if ( !model || model->GetModelType() != mod_brush )
		return 0;

	return model->R_GetBrushModelPlaneCount();
}

void CModelInfo::GetBrushModelPlane(int modelIndex, int nIndex, cplane_t& plane, Vector* pOrigin) const
{
	GetBrushModelPlane(GetModel(modelIndex), nIndex, plane, pOrigin);
}

void CModelInfo::GetBrushModelPlane( const model_t *model, int nIndex, cplane_t &plane, Vector *pOrigin ) const
{
	if ( !model || model->GetModelType() != mod_brush )
		return;

	plane = ((model_t*)model)->R_GetBrushModelPlane( nIndex, pOrigin );
}




//-----------------------------------------------------------------------------
// implementation of IVModelInfo for server
//-----------------------------------------------------------------------------
class CModelInfoServer : public CModelInfo
{
public:
	//virtual int RegisterDynamicModel( const char *name, bool bClientSideOnly );
	//virtual const model_t *FindOrLoadModel( const char *name );
	//virtual void OnDynamicModelsStringTableChange( int nStringIndex, const char *pString, const void *pData );

	//virtual void GetModelMaterialColorAndLighting( const model_t *model, const Vector& origin,
	//	const QAngle& angles, trace_t* pTrace, Vector& lighting, Vector& matColor );

protected:
	//virtual INetworkStringTable *GetDynamicModelStringTable() const;
	virtual int LookupPrecachedModelIndex( const char *name ) const;

	virtual const model_t* GetModel(int modelindex) const;
};

//INetworkStringTable *CModelInfoServer::GetDynamicModelStringTable() const
//{
//	return sv.GetDynamicModelsTable();
//}

int CModelInfoServer::LookupPrecachedModelIndex( const char *name ) const
{
	return sv.LookupModelIndex( name );
}

//int CModelInfoServer::RegisterDynamicModel( const char *name, bool bClientSide )
//{
//	// Server should not know about client-side dynamic models!
//	Assert( !bClientSide );
//	if ( bClientSide )
//		return -1;
//
//	char buf[256];
//	V_strncpy( buf, name, ARRAYSIZE(buf) );
//	V_RemoveDotSlashes( buf, '/', true );
//	name = buf;
//
//	Assert( V_strnicmp( name, "models/", 7 ) == 0 && V_strstr( name, ".mdl" ) != NULL );
//
//	// Already known? bClientSide should always be false and is asserted above.
//	int index = GetModelIndex( name );
//	if ( index != -1 )
//		return index;
//
//	INetworkStringTable *pTable = GetDynamicModelStringTable();
//	Assert( pTable );
//	if ( !pTable )
//		return -1;
//
//	// Register this model with the dynamic model string table
//	Assert( pTable->FindStringIndex( name ) == INVALID_STRING_INDEX );
//	bool bWasLocked = static_cast<CNetworkStringTable_LockOverride*>( pTable )->LockWithRetVal( false );
//	char nIsLoaded = 0;
//	int netidx = pTable->AddString( true, name, 1, &nIsLoaded );	
//	static_cast<CNetworkStringTable*>( pTable )->Lock( bWasLocked );
//
//	// And also cache the model_t* pointer at this time
//	GrowNetworkedDynamicModels( netidx );
//	m_NetworkedDynamicModels[ netidx ] = modelloader->GetDynamicModel( name, bClientSide );
//
//	Assert( MODEL_TO_NETDYNAMIC( ( short ) NETDYNAMIC_TO_MODEL( netidx ) ) == netidx );
//	return NETDYNAMIC_TO_MODEL( netidx );
//}

const model_t *CModelInfoServer::GetModel( int modelindex ) const
{
	//if (IsDynamicModelIndex(modelindex))
	//	return NULL;// LookupDynamicModel(modelindex);

	return sv.GetModel( modelindex );
}


//void CModelInfoServer::OnDynamicModelsStringTableChange( int nStringIndex, const char *pString, const void *pData )
//{
//	AssertMsg( false, "CModelInfoServer::OnDynamicModelsStringTableChange should never be called" );
//}

//void CModelInfoServer::GetModelMaterialColorAndLighting( const model_t *model, const Vector& origin,
//	const QAngle& angles, trace_t* pTrace, Vector& lighting, Vector& matColor )
//{
//	Msg( "GetModelMaterialColorAndLighting:  Available on client only!\n" );
//}

//const model_t *CModelInfoServer::FindOrLoadModel( const char *name )
//{
//	AssertMsg( false, "CModelInfoServer::FindOrLoadModel should never be called" );
//	return NULL;
//}


//static CModelInfoServer	g_ModelInfoServer;
//EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CModelInfoServer, IVModelInfo003, VMODELINFO_SERVER_INTERFACE_VERSION_3, g_ModelInfoServer );
//EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CModelInfoServer, IVModelInfo, VMODELINFO_SERVER_INTERFACE_VERSION, g_ModelInfoServer );

// Expose IVModelInfo to the engine
//IVModelInfo *modelinfo = &g_ModelInfoServer;


#ifndef SWDS
//-----------------------------------------------------------------------------
// implementation of IVModelInfo for client
//-----------------------------------------------------------------------------
class CModelInfoClient : public CModelInfo//, public IVModelInfoClient
{
public:
	//virtual int RegisterDynamicModel( const char *name, bool bClientSideOnly );
	//virtual const model_t *FindOrLoadModel( const char *name );
	virtual void OnDynamicModelsStringTableChange( int nStringIndex, const char *pString, const void *pData );

	virtual void GetModelMaterialColorAndLighting(IVModel* pWorld, int modelIndex, const Vector& origin,
		const QAngle& angles, trace_t* pTrace, Vector& lighting, Vector& matColor);
	virtual void GetIlluminationPoint(int modelIndex, IClientRenderable* pRenderable, Vector const& origin,
		QAngle const& angles, Vector* pLightingCenter);

	virtual bool IsUsingFBTexture(int modelIndex, int nSkin, int nBody, void /*IClientRenderable*/* pClientRenderable) const;
	virtual bool ModelHasMaterialProxy(int modelIndex) const;

	virtual void GetModelMaterialColorAndLighting(IVModel* pWorld, const model_t* model, const Vector& origin,
		const QAngle& angles, trace_t* pTrace, Vector& lighting, Vector& matColor);
	virtual void GetIlluminationPoint(const model_t* model, IClientRenderable* pRenderable, Vector const& origin,
		QAngle const& angles, Vector* pLightingCenter);

	virtual bool IsUsingFBTexture(const model_t* model, int nSkin, int nBody, void /*IClientRenderable*/* pClientRenderable) const;
	virtual bool ModelHasMaterialProxy(const model_t* model) const;

protected:
	//virtual INetworkStringTable* GetDynamicModelStringTable() const;
	virtual int LookupPrecachedModelIndex(const char* name) const;

	virtual const model_t* GetModel(int modelindex) const;

};

//INetworkStringTable *CModelInfoClient::GetDynamicModelStringTable() const
//{
//	return cl.m_pDynamicModelsTable;
//}

int CModelInfoClient::LookupPrecachedModelIndex( const char *name ) const
{
	return cl.LookupModelIndex( name );
}

//int CModelInfoClient::RegisterDynamicModel( const char *name, bool bClientSide )
//{
//	// Clients cannot register non-client-side dynamic models!
//	Assert( bClientSide );
//	if ( !bClientSide )
//		return -1;
//
//	char buf[256];
//	V_strncpy( buf, name, ARRAYSIZE(buf) );
//	V_RemoveDotSlashes( buf, '/', true );
//	name = buf;
//
//	Assert( V_strstr( name, ".mdl" ) != NULL );
//
//	// Already known? bClientSide should always be true and is asserted above.
//	int index = GetModelClientSideIndex( name );
//	if ( index != -1 )
//		return index;
//
//	// Lookup (or create) model_t* and register it to get a stable iterator index
//	model_t* pModel = modelloader->GetDynamicModel( name, true );
//	Assert( pModel );
//	UtlHashHandle_t localidx = m_ClientDynamicModels.Insert( pModel );
//	Assert( m_ClientDynamicModels.Count() < ((32767 >> 1) - 2) );
//	Assert( MODEL_TO_CLIENTSIDE( (short) CLIENTSIDE_TO_MODEL( localidx ) ) == (int) localidx );
//	return CLIENTSIDE_TO_MODEL( localidx );
//}

const model_t *CModelInfoClient::GetModel( int modelindex ) const
{
	//if (IsDynamicModelIndex(modelindex))
	//	return NULL;// LookupDynamicModel(modelindex);

	return cl.GetModel( modelindex );
}


void CModelInfoClient::OnDynamicModelsStringTableChange( int nStringIndex, const char *pString, const void *pData )
{
	// Do a lookup to force an immediate insertion into our local lookup tables
	//model_t* pModel = LookupDynamicModel( NETDYNAMIC_TO_MODEL( nStringIndex ) );

	//// Notify model loader that the server-side state may have changed
	//bool bServerLoaded = pData ? ( *(char*)pData != 0 ) : ( g_ClientGlobalVariables.network_protocol <= PROTOCOL_VERSION_20 );
	//modelloader->Client_OnServerModelStateChanged( pModel, bServerLoaded );
}

//const model_t *CModelInfoClient::FindOrLoadModel( const char *name )
//{
//	// find the cached model from the server or client
//	const model_t *pModel = GetModel( GetModelIndex( name ) );
//	if ( pModel )
//		return pModel;
//
//	// load the model
//	return modelloader->GetModelForName( name, IModelLoader::FMODELLOADER_CLIENTDLL );
//}





//-----------------------------------------------------------------------------
// A method to get the material color + texture coordinate
//-----------------------------------------------------------------------------
IMaterial* BrushModel_GetLightingAndMaterial(const model_t* model, const Vector &start,
	const Vector &end, Vector &diffuseLightColor, Vector &baseColor)
{
	float textureS, textureT;
	IMaterial *material;

	// TEMP initialize these values until we can find why R_LightVec is not assigning values to them
	textureS = 0;
	textureT = 0;

	SurfaceHandle_t surfID = R_LightVec((model_t*)model, start, end, true, diffuseLightColor, &textureS, &textureT );
	if( !IS_SURF_VALID( surfID ) || !((model_t*)model)->MSurf_TexInfo( surfID ) )//model->brush.pShared
	{
//		ConMsg( "didn't hit anything\n" );
		return 0;
	}
	else
	{
		material = ((model_t*)model)->MSurf_TexInfo( surfID)->material;
		if ( material )
		{
			material->GetLowResColorSample( textureS, textureT, baseColor.Base() );
	//		ConMsg( "%s: diff: %f %f %f base: %f %f %f\n", material->GetName(), diffuseLightColor[0], diffuseLightColor[1], diffuseLightColor[2], baseColor[0], baseColor[1], baseColor[2] );
		}
		else
		{
			baseColor.Init();
		}
		return material;
	}
}

void model_t::GetModelMaterialColorAndLighting(IVModel* pWorld, const Vector& origin,//const model_t* model, 
	const QAngle& angles, trace_t* pTrace, Vector& lighting, Vector& matColor) const
{
	switch (type)//model->
	{
	case mod_brush:
	{
		Vector origin_l, delta, delta_l;
		VectorSubtract(pTrace->endpos, pTrace->startpos, delta);

		// subtract origin offset
		VectorSubtract(pTrace->startpos, origin, origin_l);

		// rotate start and end into the models frame of reference
		if (angles[0] || angles[1] || angles[2])
		{
			Vector forward, right, up;
			AngleVectors(angles, &forward, &right, &up);

			// transform the direction into the local space of this entity
			delta_l[0] = DotProduct(delta, forward);
			delta_l[1] = -DotProduct(delta, right);
			delta_l[2] = DotProduct(delta, up);
		}
		else
		{
			VectorCopy(delta, delta_l);
		}

		Vector end_l;
		VectorMA(origin_l, 1.1f, delta_l, end_l);

		R_LightVecUseModel((model_t*)this);
		BrushModel_GetLightingAndMaterial(this, origin_l, end_l, lighting, matColor);
		R_LightVecUseModel(0);
		return;
	}

	case mod_studio:
	{
		// FIXME: Need some way of getting the material!
		matColor.Init(0.5f, 0.5f, 0.5f);

		// Get the lighting at the point
		LightingState_t lightingState;
		LightcacheGetDynamic_Stats stats;
		LightcacheGetDynamic((model_t*)pWorld, pTrace->endpos, lightingState, stats, LIGHTCACHEFLAGS_STATIC | LIGHTCACHEFLAGS_DYNAMIC | LIGHTCACHEFLAGS_LIGHTSTYLE | LIGHTCACHEFLAGS_ALLOWFAST);
		// Convert the light parameters into something studiorender can digest
		LightDesc_t desc[MAXLOCALLIGHTS];
		int count = 0;
		for (int i = 0; i < lightingState.numlights; ++i)
		{
			if (WorldLightToMaterialLight(lightingState.locallight[i], desc[count]))
			{
				++count;
			}
		}

		// Ask studiorender to figure out the lighting
		g_pStudioRender->ComputeLighting(lightingState.r_boxcolor,
			count, desc, pTrace->endpos, pTrace->plane.normal, lighting);
		return;
	}
	}
}

void CModelInfoClient::GetModelMaterialColorAndLighting(IVModel* pWorld, int modelIndex, const Vector& origin,
	const QAngle& angles, trace_t* pTrace, Vector& lighting, Vector& matColor)
{
	GetModelMaterialColorAndLighting(pWorld, GetModel(modelIndex), origin, angles, pTrace, lighting, matColor);
}

void CModelInfoClient::GetModelMaterialColorAndLighting(IVModel* pWorld, const model_t *model, const Vector & origin,
	const QAngle & angles, trace_t* pTrace, Vector& lighting, Vector& matColor )
{
	model->GetModelMaterialColorAndLighting((model_t*)pWorld, origin, angles, pTrace, lighting, matColor);
}

void model_t::GetIlluminationPoint(IClientRenderable* pRenderable, const Vector& origin,
	const QAngle& angles, Vector* pLightingOrigin) const
{
	Assert(type == mod_studio);
	studiohdr_t* pStudioHdr = (studiohdr_t*)GetModelExtraData();
	if (pStudioHdr)
	{
		R_StudioGetLightingCenter(pRenderable, pStudioHdr, origin, angles, pLightingOrigin);
	}
	else
	{
		*pLightingOrigin = origin;
	}
}

void CModelInfoClient::GetIlluminationPoint(int modelIndex, IClientRenderable* pRenderable, const Vector& origin,
	const QAngle& angles, Vector* pLightingOrigin)
{
	GetIlluminationPoint(GetModel(modelIndex), pRenderable, origin, angles, pLightingOrigin);
}

void CModelInfoClient::GetIlluminationPoint(const model_t* model, IClientRenderable* pRenderable, const Vector& origin,
	const QAngle& angles, Vector* pLightingOrigin)
{
	model->GetIlluminationPoint(pRenderable, origin, angles, pLightingOrigin);
}

bool model_t::IsUsingFBTexture(int nSkin, int nBody, void /*IClientRenderable*/* pClientRenderable) const//const model_t* model, 
{
	bool bMightUseFbTextureThisFrame = flags & MODELFLAG_STUDIOHDR_USES_FB_TEXTURE;//(model && (model-> ))

	if (bMightUseFbTextureThisFrame)
	{
		// Check each material's NeedsPowerOfTwoFrameBufferTexture() virtual func
		switch (type)//model->
		{
		case mod_brush:
		{
			for (int i = 0; i <brush.nummodelsurfaces; ++i)// model->
			{
				SurfaceHandle_t surfID = SurfaceHandleFromIndex(brush.firstmodelsurface + i);//model-> model->
				IMaterial* material = ((model_t*)this)->MSurf_TexInfo(surfID)->material;//model->
				if (material != NULL)
				{
					if (material->NeedsPowerOfTwoFrameBufferTexture())
					{
						return true;
					}
				}
			}
		}
		break;

		case mod_studio:
		{
			IMaterial* pMaterials[128];
			int materialCount = g_pStudioRender->GetMaterialListFromBodyAndSkin(studio, nSkin, nBody, ARRAYSIZE(pMaterials), pMaterials);//model->
			for (int i = 0; i < materialCount; i++)
			{
				if (pMaterials[i] != NULL)
				{
					// Bind material first so all material proxies execute
					CMatRenderContextPtr pRenderContext(g_pMaterialSystem);
					pRenderContext->Bind(pMaterials[i], pClientRenderable);

					if (pMaterials[i]->NeedsPowerOfTwoFrameBufferTexture())
					{
						return true;
					}
				}
			}
		}
		break;
		}
	}

	return false;
}

bool CModelInfoClient::IsUsingFBTexture(int modelIndex, int nSkin, int nBody, void /*IClientRenderable*/* pClientRenderable) const
{
	return IsUsingFBTexture(GetModel(modelIndex), nSkin, nBody, pClientRenderable);
}

bool CModelInfoClient::IsUsingFBTexture(const model_t* model, int nSkin, int nBody, void /*IClientRenderable*/* pClientRenderable) const
{
	bool bMightUseFbTextureThisFrame = (model && (((model_t*)model)->GetModelFlag() & MODELFLAG_STUDIOHDR_USES_FB_TEXTURE));

	if (bMightUseFbTextureThisFrame)
	{
		// Check each material's NeedsPowerOfTwoFrameBufferTexture() virtual func
		switch (model->GetModelType())
		{
		case mod_brush:
		{
			for (int i = 0; i < model->GetModelsurfacesCount(); ++i)
			{
				SurfaceHandle_t surfID = model->SurfaceHandleFromIndex(model->GetFirstmodelsurface() + i);//, model->brush.pShared
				IMaterial* material = ((model_t*)model)->MSurf_TexInfo(surfID )->material;//model->brush.pShared
				if (material != NULL)
				{
					if (material->NeedsPowerOfTwoFrameBufferTexture())
					{
						return true;
					}
				}
			}
		}
		break;

		case mod_studio:
		{
			IMaterial* pMaterials[128];
			int materialCount = g_pStudioRender->GetMaterialListFromBodyAndSkin(model->GetStudio(), nSkin, nBody, ARRAYSIZE(pMaterials), pMaterials);
			for (int i = 0; i < materialCount; i++)
			{
				if (pMaterials[i] != NULL)
				{
					// Bind material first so all material proxies execute
					CMatRenderContextPtr pRenderContext(g_pMaterialSystem);
					pRenderContext->Bind(pMaterials[i], pClientRenderable);

					if (pMaterials[i]->NeedsPowerOfTwoFrameBufferTexture())
					{
						return true;
					}
				}
			}
		}
		break;
		}
	}

	return false;
}

bool model_t::ModelHasMaterialProxy() const//const model_t* model
{
	// Should we add skin & model to this function like IsUsingFBTexture()?
	return flags & MODELFLAG_MATERIALPROXY;//(model && (model-> ))
}

bool CModelInfoClient::ModelHasMaterialProxy(int modelIndex) const
{
	// Should we add skin & model to this function like IsUsingFBTexture()?
	return ModelHasMaterialProxy(GetModel(modelIndex));
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CModelInfoClient::ModelHasMaterialProxy(const model_t* model) const
{
	// Should we add skin & model to this function like IsUsingFBTexture()?
	return (model && (model->ModelHasMaterialProxy()));
}

//CModelInfoClient	g_ModelInfoClient;
//EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CModelInfoClient, IVModelInfoClient, VMODELINFO_CLIENT_INTERFACE_VERSION, g_ModelInfoClient );

// Expose IVModelInfo to the engine
//IVModelInfoClient *modelinfoclient = &g_ModelInfoClient;

#endif
