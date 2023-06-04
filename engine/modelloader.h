//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#if !defined( MOD_LOADER_H )
#define MOD_LOADER_H
#ifdef _WIN32
#pragma once
#endif

//class model_t;
class IMaterial;
class IFileList;
class IModelLoadCallback;
//struct dheader_t;
typedef void* FileHandle_t;

#include "utlmemory.h"
#include "utlbuffer.h"
#include "bspfile.h"
#include "model_types.h"
#include "tier1/UtlStringMap.h"
#include "utlhashtable.h"
#include "utldict.h"
#include "mempool.h"
#include "gl_model_private.h"

bool Model_LessFunc(FileNameHandle_t const& a, FileNameHandle_t const& b);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CModelLoader
{
public:
	enum REFERENCETYPE
	{
		// The name is allocated, but nothing else is in memory or being referenced
		FMODELLOADER_NOTLOADEDORREFERENCED = 0,
		// The model has been loaded into memory
		FMODELLOADER_LOADED	= (1<<0),

		// The model is being referenced by the server code
		FMODELLOADER_SERVER = (1<<1),
		// The model is being referenced by the client code
		FMODELLOADER_CLIENT = (1<<2),
		// The model is being referenced in the client .dll
		FMODELLOADER_CLIENTDLL = (1<<3),
		// The model is being referenced by static props
		FMODELLOADER_STATICPROP	= (1<<4),
		// The model is a detail prop
		FMODELLOADER_DETAILPROP = (1<<5),
		// The model is dynamically loaded
		FMODELLOADER_DYNSERVER = (1<<6),
		FMODELLOADER_DYNCLIENT = (1<<7),
		FMODELLOADER_DYNAMIC = FMODELLOADER_DYNSERVER | FMODELLOADER_DYNCLIENT,

		FMODELLOADER_REFERENCEMASK = (FMODELLOADER_SERVER | FMODELLOADER_CLIENT | FMODELLOADER_CLIENTDLL | FMODELLOADER_STATICPROP | FMODELLOADER_DETAILPROP | FMODELLOADER_DYNAMIC ),

		// The model was touched by the preload method
		FMODELLOADER_TOUCHED_BY_PRELOAD = (1<<15),
		// The model was loaded by the preload method, a postload fixup is required
		FMODELLOADER_LOADED_BY_PRELOAD = (1<<16),
		// The model touched its materials as part of its load
		FMODELLOADER_TOUCHED_MATERIALS = (1<<17),
	};

	enum ReloadType_t
	{
		RELOAD_LOD_CHANGED = 0,
		RELOAD_EVERYTHING,
		RELOAD_REFRESH_MODELS,
	};

	CModelLoader() : m_ModelPool(sizeof(model_t), MAX_KNOWN_MODELS, CUtlMemoryPool::GROW_FAST, "CModelLoader::m_ModelPool"),
		m_Models(0, 0, Model_LessFunc)
	{
	}

	void			Init(void);
	void			Shutdown(void);

	int				GetCount(void);
	model_t* GetModelForIndex(int i);

	// Look up name for model
	//const char		*GetName( model_t const *model );

	// Check cache for data, reload model if needed
	//void			*GetExtraData( const model_t *model );

	int				GetModelFileSize(char const* name);

	// Finds the model, and loads it if it isn't already present.  Updates reference flags
	model_t* GetModelForName(const char* name, REFERENCETYPE referencetype);
	// Mark as referenced by name
	model_t* ReferenceModel(const char* name, REFERENCETYPE referencetype);

	// Unmasks the referencetype field for the model
	void			UnreferenceModel(model_t* model, REFERENCETYPE referencetype);
	// Unmasks the specified reference type across all models
	void			UnreferenceAllModels(REFERENCETYPE referencetype);
	// Set all models to last loaded on server count -1
	void			ResetModelServerCounts();

	// For any models with referencetype blank, frees all memory associated with the model
	//  and frees up the models slot
	void			UnloadUnreferencedModels(void);
	void			PurgeUnusedModels(void);

	bool			Map_GetRenderInfoAllocated(void);
	void			Map_SetRenderInfoAllocated(bool allocated);

	virtual void	Map_LoadDisplacements(model_t* pModel, bool bRestoring);

	// Validate version/header of a .bsp file
	bool			Map_IsValid(char const* mapname, bool bQuiet = false);


	virtual void	Studio_ReloadModels(ReloadType_t reloadType);

	void			Print(void);

	// Is a model loaded?
	virtual bool	IsLoaded(const model_t* mod);

	virtual bool	LastLoadedMapHasHDRLighting(void);

	void			DumpVCollideStats();

	// Returns the map model, otherwise NULL, no load or create
	model_t* FindModelNoCreate(const char* pModelName);

	// Finds the model, builds a model entry if not present
	model_t* FindModel(const char* name);

	modtype_t		GetTypeFromName(const char* pModelName);

	// start with -1, list terminates with -1
	int				FindNext(int iIndex, model_t** ppModel);

	virtual void	UnloadModel(model_t* pModel);

	virtual void	ReloadFilesInList(IFileList* pFilesToReload);

	virtual const char* GetActiveMapName(void);

	// Called by app system once per frame to poll and update dynamic models
	virtual void	UpdateDynamicModels() { InternalUpdateDynamicModels(false); }

	// Called by server and client engine code to flush unreferenced dynamic models
	virtual void	FlushDynamicModels() { InternalUpdateDynamicModels(true); }

	// Called by server and client to force-unload dynamic models regardless of refcount!
	virtual void	ForceUnloadNonClientDynamicModels();

	// Called by client code to load dynamic models, instead of GetModelForName.
	virtual model_t* GetDynamicModel(const char* name, bool bClientOnly);

	// Called by client code to query dynamic model state
	virtual bool	IsDynamicModelLoading(model_t* pModel, bool bClientOnly);

	// Called by client code to refcount dynamic models
	virtual void	AddRefDynamicModel(model_t* pModel, bool bClientSideRef);
	virtual void	ReleaseDynamicModel(model_t* pModel, bool bClientSideRef);

	// Called by client code or GetDynamicModel
	virtual bool	RegisterModelLoadCallback(model_t* pModel, bool bClientOnly, IModelLoadCallback* pCallback, bool bCallImmediatelyIfLoaded);

	// Called by client code or IModelLoadCallback destructor
	virtual void	UnregisterModelLoadCallback(model_t* pModel, bool bClientOnly, IModelLoadCallback* pCallback);

	virtual void	Client_OnServerModelStateChanged(model_t* pModel, bool bServerLoaded);

	void			DebugPrintDynamicModels();

	// Recomputes surface flags
	virtual void	RecomputeSurfaceFlags(model_t* pWorld);
	// Internal types
private:
	// TODO, flag these and allow for UnloadUnreferencedModels to check for allocation type
	//  so we don't have to flush all of the studio models when we free the hunk
	enum
	{
		FALLOC_USESHUNKALLOC = (1 << 31),
		FALLOC_USESCACHEALLOC = (1 << 30),
	};

	// Internal methods
private:

	model_t* AllocModel() {
		void* p = m_ModelPool.Alloc();
		memset(p, 0, sizeof(model_t));
		model_t* pModel = NULL;
		pModel = new(p) model_t();
		Assert(pModel);
		return pModel;
	}

	// Set reference flags and load model if it's not present already
	model_t* LoadModel(model_t* model, REFERENCETYPE* referencetype);
	// Unload models ( won't unload referenced models if checkreferences is true )
	void		UnloadAllModels(bool checkreference);

	// World/map
	void		Map_LoadModel(model_t* mod);
	void		Map_UnloadModel(model_t* mod);
	void		Map_UnloadCubemapSamples(model_t* mod);
	void		SetupSubModels(model_t* pModel, CUtlVector<mmodel_t>& list);//CUtlVector<model_t>&	m_InlineModels,	

	// World loading helper
	//void		SetWorldModel( model_t *mod );
	//void		ClearWorldModel(model_t* mod);
	//bool		IsWorldModelSet( void );
	//int			GetNumWorldSubmodels( void );

	// Sprites
	void		Sprite_LoadModel(model_t* mod);
	void		Sprite_UnloadModel(model_t* mod);

	// Studio models
	void		Studio_LoadModel(model_t* mod, bool bTouchAllData);
	void		Studio_UnloadModel(model_t* mod);

	// Byteswap
	int			UpdateOrCreate(const char* pSourceName, char* pTargetName, int maxLen, bool bForce);

	// Dynamic load queue
	class CDynamicModelInfo;
	void		QueueDynamicModelLoad(CDynamicModelInfo* dyn, model_t* mod);
	bool		CancelDynamicModelLoad(CDynamicModelInfo* dyn, model_t* mod);
	void		UpdateDynamicModelLoadQueue();

	void		FinishDynamicModelLoadIfReady(CDynamicModelInfo* dyn, model_t* mod);

	void		InternalUpdateDynamicModels(bool bIgnoreUpdateTime);

	// Internal data
private:
	enum
	{
		MAX_KNOWN_MODELS = 1024,
	};

	struct ModelEntry_t
	{
		model_t* modelpointer;
	};

	CUtlMap< FileNameHandle_t, ModelEntry_t >	m_Models;

	CUtlMemoryPool			m_ModelPool;

	//CUtlVector<model_t>	m_InlineModels;
	CUtlStringMap<model_t*>	m_InlineModelMap;

	//model_t				*m_pWorldModel;
	//worldbrushdata_t	m_worldBrushData;

public: // HACKHACK

private:
	// local name of current loading model
	char				m_szLoadName[64];

	bool				m_bMapRenderInfoLoaded;
	bool				m_bMapHasHDRLighting;

	char				m_szActiveMapName[64];

	// Dynamic model support:
	class CDynamicModelInfo
	{
	public:
		enum { QUEUED = 0x01, LOADING = 0x02, CLIENTREADY = 0x04, SERVERLOADING = 0x08, ALLREADY = 0x10, INVALIDFLAG = 0x20 }; // flags
		CDynamicModelInfo() : m_iRefCount(0), m_iClientRefCount(0), m_nLoadFlags(INVALIDFLAG), m_uLastTouchedMS_Div256(0) { }
		int16 m_iRefCount;
		int16 m_iClientRefCount; // also doublecounted in m_iRefCount
		uint32 m_nLoadFlags : 8;
		uint32 m_uLastTouchedMS_Div256 : 24;
		CUtlVector< uintptr_t > m_Callbacks; // low bit = client only
	};

	CUtlHashtable< model_t*, CDynamicModelInfo > m_DynamicModels;
	CUtlHashtable< uintptr_t, int > m_RegisteredDynamicCallbacks;

	// Dynamic model load queue
	CUtlVector< model_t* > m_DynamicModelLoadQueue;
	bool m_bDynamicLoadQueueHeadActive;
};

extern CModelLoader* modelloader;



//-----------------------------------------------------------------------------
// Purpose: Loads the lump to temporary memory and automatically cleans up the
//  memory when it goes out of scope.
//-----------------------------------------------------------------------------

class CLumpHeaderInfo
{
public:
	CLumpHeaderInfo(model_t* pMapModel, const char* pLoadname);
	CLumpHeaderInfo(model_t* pMapModel, const void* pData, int nDataSize);

	const char* GetMapName(void);
	//struct worldbrushdata_t	*GetMap( void );

	// Global setup/shutdown
	void			Init(model_t* pMapModel, const char* pLoadname);
	void			InitFromMemory(model_t* pMapModel, const void* pData, int nDataSize);
	void			Shutdown(void);
	int				GetRefCount(void);

	// Free the lighting lump (increases free memory during loading on 360)
	void			FreeLightingLump();

	// Returns the size of a particular lump without loading it
	int				LumpSize(int lumpId);
	int				LumpOffset(int lumpId);

	


	//-----------------------------------------------------------------------------
// Globals used by the CMapLoadHelper
//-----------------------------------------------------------------------------
public:
	dheader_t				m_MapHeader;
	FileHandle_t			m_MapFileHandle;
	char					m_szLoadName[128];
	char					m_szMapName[128];
	//worldbrushdata_t		*s_pMap = NULL;
	int						m_nMapLoadRecursion;
	static CUtlBuffer				s_MapBuffer;
};

class CLumpInfo
{
public:
						CLumpInfo(CLumpHeaderInfo& headerInfo, int lumpToLoad );
						~CLumpInfo( void );

	char*				GetLoadName(void);

	// Get raw memory pointer
	byte				*LumpBase( void );
	int					LumpSize( void );
	int					LumpOffset( void );
	int					LumpVersion() const;

	// Loads one element in a lump.
	void				LoadLumpElement(int nElemIndex, int nElemSize, void* pData);
	void				LoadLumpData(int offset, int size, void* pData);
private:
	CLumpHeaderInfo&	m_LumpHeaderInfo;
	int					m_nLumpSize;
	int					m_nLumpOffset;
	int					m_nLumpVersion;
	byte				*m_pRawData;
	byte				*m_pData;
	byte				*m_pUncompressedData;

	// Handling for lump files
	int					m_nLumpID;
	char				m_szLumpFilename[MAX_PATH];

};

//-----------------------------------------------------------------------------
// Recomputes translucency for the model...
//-----------------------------------------------------------------------------

//void Mod_RecomputeTranslucency( model_t* mod, int nSkin, int nBody, void /*IClientRenderable*/ *pClientRenderable, float fInstanceAlphaModulate );

//-----------------------------------------------------------------------------
// game lumps
//-----------------------------------------------------------------------------







bool Mod_MarkWaterSurfaces( model_t *pModel );

void Mod_SetMaterialVarFlag( model_t *pModel, unsigned int flag, bool on );

//-----------------------------------------------------------------------------
// Hooks the cache notify into the MDL cache system 
//-----------------------------------------------------------------------------
void ConnectMDLCacheNotify( );
void DisconnectMDLCacheNotify( );

//-----------------------------------------------------------------------------
// Initialize studiomdl state
//-----------------------------------------------------------------------------
void InitStudioModelState( model_t *pModel );

extern bool g_bLoadedMapHasBakedPropLighting;
extern bool g_bBakedPropLightingNoSeparateHDR;

#endif // MOD_LOADER_H
