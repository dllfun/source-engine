//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#ifndef GL_MODEL_PRIVATE_H
#define GL_MODEL_PRIVATE_H

#ifdef _WIN32
#pragma once
#endif

#include "mathlib/vector4d.h"
#include "tier0/dbg.h"
#include "tier1/utlsymbol.h"
#include "idispinfo.h"
#include "shadowmgr.h"
#include "vcollide.h"
#include "studio.h"
#include "qlimits.h"
#include "host.h"
#include "gl_model.h"
#include "cmodel.h"
#include "bspfile.h"
#include "Overlay.h"
//#include "datamap.h"
#include "surfacehandle.h"
#include "mathlib/compressed_light_cube.h"
#include "datacache/imdlcache.h"
#include "bitmap/cubemap.h"
#include "bsptreedata.h"
#include "cmodel_private.h"
#include "vphysics\virtualmesh.h"


//-----------------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------------
struct studiomeshdata_t;
struct decal_t;
struct msurface1_t;
struct msurfacelighting_t;
struct msurfacenormal_t;
class ITexture;
class CEngineSprite;

// !!! if this is changed, it must be changed in asm_draw.h too !!!
struct mvertex_t
{
	Vector		position;
};

// !!! if this is changed, it must be changed in asm_draw.h too !!!
struct medge_t
{
	unsigned short	v[2];
	//	unsigned int	cachededgeoffset;
};

class IMaterial;
class IMaterialVar;

// This is here for b/w compatibility with world surfaces that use
// WorldVertexTransition. We can get rid of it when we rev the engine.
#define TEXINFO_USING_BASETEXTURE2	0x0001

struct mtexinfo_t
{
	Vector4D	textureVecsTexelsPerWorldUnits[2];	// [s/t] unit vectors in world space. 
	// [i][3] is the s/t offset relative to the origin.
	Vector4D	lightmapVecsLuxelsPerWorldUnits[2];
	float		luxelsPerWorldUnit;
	float		worldUnitsPerLuxel;
	unsigned short flags;		// SURF_ flags.
	unsigned short texinfoFlags;// TEXINFO_ flags.
	IMaterial	*material;

	mtexinfo_t( mtexinfo_t const& src )
	{
		// copy constructor needed since Vector4D has no copy constructor
		memcpy( this, &src, sizeof(mtexinfo_t) );
	}
};

struct mnode_t
{
	// common with leaf
	int			contents;		// <0 to differentiate from leafs
	// -1 means check the node for visibility
	// -2 means don't check the node for visibility

	int			visframe;		// node needs to be traversed if current

	mnode_t		*parent;
	short		area;			// If all leaves below this node are in the same area, then
	// this is the area index. If not, this is -1.
	short		flags;

	VectorAligned		m_vecCenter;
	VectorAligned		m_vecHalfDiagonal;

	// node specific
	cplane_t	*plane;
	mnode_t		*children[2];

	unsigned short		firstsurface;
	unsigned short		numsurfaces;
};


struct mleaf_t
{
public:

	// common with node
	int			contents;		// contents mask
	int			visframe;		// node needs to be traversed if current

	mnode_t		*parent;

	short		area;
	short		flags;
	VectorAligned		m_vecCenter;
	VectorAligned		m_vecHalfDiagonal;


	// leaf specific
	short		cluster;
	short		leafWaterDataID;

	unsigned short		firstmarksurface;
	unsigned short		nummarksurfaces;

	short		nummarknodesurfaces;
	short		unused;

	unsigned short	dispListStart;			// index into displist of first displacement
	unsigned short	dispCount;				// number of displacements in the list for this leaf
};


struct mleafwaterdata_t
{
	float	surfaceZ;
	float	minZ;
	short	surfaceTexInfoID;
	short	firstLeafIndex;
};


struct mcubemapsample_t
{
	Vector origin;
	ITexture *pTexture;
	unsigned char size; // default (mat_envmaptgasize) if 0, 1<<(size-1) otherwise.
};


typedef struct mportal_s
{
	unsigned short	*vertList;
	int				numportalverts;
	cplane_t		*plane;
	unsigned short	cluster[2];
	//	int				visframe;
} mportal_t;


typedef struct mcluster_s
{
	unsigned short	*portalList;
	int				numportals;
} mcluster_t;


struct mmodel_t
{
	Vector		mins, maxs;
	Vector		origin;		// for sounds or lights
	float		radius;
	int			headnode;
	int			firstface, numfaces;
};

struct mprimitive_t
{
	int	type;
	unsigned short	firstIndex;
	unsigned short	indexCount;
	unsigned short	firstVert;
	unsigned short	vertCount;
};

struct mprimvert_t
{
	Vector		pos;
	float		texCoord[2];
	float		lightCoord[2];
};

typedef dleafambientindex_t mleafambientindex_t;
typedef dleafambientlighting_t mleafambientlighting_t;

struct LightShadowZBufferSample_t
{
	float m_flTraceDistance;								// how far we traced. 0 = invalid
	float m_flHitDistance;									// where we hit
};

#define SHADOW_ZBUF_RES 8									// 6 * 64 * 2 * 4 = 3k bytes per light

typedef CCubeMap< LightShadowZBufferSample_t, SHADOW_ZBUF_RES> lightzbuffer_t;

#include "model_types.h"

#define MODELFLAG_MATERIALPROXY					0x0001	// we've got a material proxy
#define MODELFLAG_TRANSLUCENT					0x0002	// we've got a translucent model
#define MODELFLAG_VERTEXLIT						0x0004	// we've got a vertex-lit model
#define MODELFLAG_TRANSLUCENT_TWOPASS			0x0008	// render opaque part in opaque pass
#define MODELFLAG_FRAMEBUFFER_TEXTURE			0x0010	// we need the framebuffer as a texture
#define MODELFLAG_HAS_DLIGHT					0x0020	// need to check dlights
#define MODELFLAG_STUDIOHDR_USES_FB_TEXTURE		0x0100	// persisted from studiohdr
#define MODELFLAG_STUDIOHDR_USES_BUMPMAPPING	0x0200	// persisted from studiohdr
#define MODELFLAG_STUDIOHDR_USES_ENV_CUBEMAP	0x0400	// persisted from studiohdr
#define MODELFLAG_STUDIOHDR_AMBIENT_BOOST		0x0800	// persisted from studiohdr
#define MODELFLAG_STUDIOHDR_DO_NOT_CAST_SHADOWS	0x1000	// persisted from studiohdr

//-----------------------------------------------------------------------------
// A dictionary used to store where to find game lump data in the .bsp file
//-----------------------------------------------------------------------------

// Extended from the on-disk struct to include uncompressed size and stop propagation of bogus signed values
struct dgamelump_internal_t
{
	dgamelump_internal_t(dgamelump_t& other, unsigned int nCompressedSize)
		: id(other.id)
		, flags(other.flags)
		, version(other.version)
		, offset(Max(other.fileofs, 0))
		, uncompressedSize(Max(other.filelen, 0))
		, compressedSize(nCompressedSize)
	{}
	GameLumpId_t	id;
	unsigned short	flags;
	unsigned short	version;
	unsigned int	offset;
	unsigned int	uncompressedSize;
	unsigned int	compressedSize;
};

// Virtual collision models for terrain
class CVirtualTerrain : public IVirtualMeshEvent
{
public:
	CVirtualTerrain()
	{
		m_pDispHullData = NULL;
	}
	// Fill out the meshlist for this terrain patch
	virtual void GetVirtualMesh(void* userData, virtualmeshlist_t* pList);
	// returns the bounds for the terrain patch
	virtual void GetWorldspaceBounds(void* userData, Vector* pMins, Vector* pMaxs);
	// Query against the AABB tree to find the list of triangles for this patch in a sphere
	virtual void GetTrianglesInSphere(void* userData, const Vector& center, float radius, virtualmeshtrianglelist_t* pList);
	void LevelInit(model_t* mod, dphysdisp_t* pLump, int lumpSize);
	void LevelShutdown();

private:
	model_t* m_mod;
	byte* m_pDispHullData;
	CUtlVector<int> m_dispHullOffset;
};

//struct cnode_t;
//template <class T>
//class CRangeValidatedArray;

struct worldbrushdata_t
{
	int			numsubmodels;

	//int			numplanes;
	//cplane_t	*planes;

	int			numleafs;		// number of visible leafs, not counting 0
	mleaf_t		*leafs;

	int			numleafwaterdata;
	mleafwaterdata_t *leafwaterdata;

	int			numvertexes;
	mvertex_t	*vertexes;

	int			numoccluders;
	doccluderdata_t *occluders;

	int			numoccluderpolys;
	doccluderpolydata_t *occluderpolys;

	int			numoccludervertindices;
	int			*occludervertindices;

	int				numvertnormalindices;	// These index vertnormals.
	unsigned short	*vertnormalindices;

	int			numvertnormals;
	Vector		*vertnormals;

	int			numnodes;
	mnode_t		*nodes;
	unsigned short *m_LeafMinDistToWater;

	int			numtexinfo;
	mtexinfo_t	*texinfo;

	//int			numtexdata;
	//csurface_t	*texdata;

	int         numDispInfos;
	HDISPINFOARRAY	hDispInfos;	// Use DispInfo_Index to get IDispInfos..

	/*
	int         numOrigSurfaces;
	msurface_t  *pOrigSurfaces;
	*/

	int			numsurfaces;
	msurface1_t	*surfaces1;
	msurface2_t	*surfaces2;
	msurfacelighting_t *surfacelighting;
	msurfacenormal_t *surfacenormals;

	bool		unloadedlightmaps;

	int			numvertindices;
	unsigned short *vertindices;

	int nummarksurfaces;
	SurfaceHandle_t *marksurfaces;

	ColorRGBExp32		*lightdata;

	int			numworldlights;
	dworldlight_t *worldlights;

	lightzbuffer_t *shadowzbuffers;

	// non-polygon primitives (strips and lists)
	int			numprimitives;
	mprimitive_t *primitives;

	int			numprimverts;
	mprimvert_t *primverts;

	int			numprimindices;
	unsigned short *primindices;

	int				m_nAreas;
	darea_t			*m_pAreas;

	int				m_nAreaPortals;
	dareaportal_t	*m_pAreaPortals;

	int				m_nClipPortalVerts;
	Vector			*m_pClipPortalVerts;

	mcubemapsample_t  *m_pCubemapSamples;
	int				   m_nCubemapSamples;

	int				m_nDispInfoReferences;
	unsigned short	*m_pDispInfoReferences;

	mleafambientindex_t		*m_pLeafAmbient;
	mleafambientlighting_t	*m_pAmbientSamples;
#if 0
	int			numportals;
	mportal_t	*portals;

	int			numclusters;
	mcluster_t	*clusters;

	int			numportalverts;
	unsigned short *portalverts;

	int			numclusterportals;
	unsigned short *clusterportals;
#endif

	CUtlVector< dgamelump_internal_t > g_GameLumpDict;
	char g_GameLumpFilename[128] = { 0 };

	CUtlVector<model_t*>	m_InlineModels;

	// This is sort of a hack, but it was a little too painful to do this any other way
	// The goal of this dude is to allow us to override the tree with some
	// other tree (or a subtree)
	cnode_t* map_rootnode;

	//char						map_name[MAX_QPATH];
	static csurface_t			nullsurface;

	int									numbrushsides;
	CRangeValidatedArray<cbrushside_t>	map_brushsides;
	int									numboxbrushes;
	CRangeValidatedArray<cboxbrush_t>	map_boxbrushes;
	int									numplanes;
	CRangeValidatedArray<cplane_t>		map_planes;
	int									numcmnodes;
	CRangeValidatedArray<cnode_t>		map_nodes;
	int									numcmleafs;				// allow leaf funcs to be called without a map
	CRangeValidatedArray<cleaf_t>		map_leafs;
	int									emptyleaf, solidleaf;
	int									numleafbrushes;
	CRangeValidatedArray<unsigned short> map_leafbrushes;
	int									numcmodels;
	CRangeValidatedArray<cmodel_t>		map_cmodels;
	int									numbrushes;
	CRangeValidatedArray<cbrush_t>		map_brushes;
	int									numdisplist;
	CRangeValidatedArray<unsigned short> map_dispList;

	// this points to the whole block of memory for vis data, but it is used to
	// reference the header at the top of the block.
	int									numvisibility;
	dvis_t* map_vis;

	int									numentitychars;
	CDiscardableArray<char>				map_entitystring;

	int									numareas;
	CRangeValidatedArray<carea_t>		map_areas;
	int									numareaportals;
	CRangeValidatedArray<dareaportal_t>	map_areaportals;
	int									numclusters;
	char* map_nullname;
	int									numtextures;
	char* map_texturenames;
	CRangeValidatedArray<csurface_t>	map_surfaces;
	int									floodvalid;
	int									numportalopen;
	CRangeValidatedArray<bool>			portalopen;

	int m_DispCollTreeCount = 0;
	CDispCollTree* m_pDispCollTrees = NULL;
	alignedbbox_t* m_pDispBounds = NULL;

	// Singleton to implement the callbacks
	CVirtualTerrain m_VirtualTerrain;
	// List of terrain collision models for the currently loaded level, indexed by terrain patch index
	CUtlVector<CPhysCollide*> m_TerrainList;
};
// only models with type "mod_brush" have this data
struct brushdata_t
{
	worldbrushdata_t	*pShared;
	int			firstmodelsurface, nummodelsurfaces;
	int				inlineModelIndex;
	unsigned short	renderHandle;
	unsigned short	firstnode;
};

// only models with type "mod_sprite" have this data
struct spritedata_t
{
	int				numframes;
	int				width;
	int				height;
	CEngineSprite	*sprite;
};

#define	MAXLIGHTMAPS	4

// drawing surface flags
#define SURFDRAW_NOLIGHT		0x00000001		// no lightmap
#define	SURFDRAW_NODE			0x00000002		// This surface is on a node
#define	SURFDRAW_SKY			0x00000004		// portal to sky
#define SURFDRAW_BUMPLIGHT		0x00000008		// Has multiple lightmaps for bump-mapping
#define SURFDRAW_NODRAW			0x00000010		// don't draw this surface, not really visible
#define SURFDRAW_TRANS			0x00000020		// sort this surface from back to front
#define SURFDRAW_PLANEBACK		0x00000040		// faces away from plane of the node that stores this face
#define SURFDRAW_DYNAMIC		0x00000080		// Don't use a static buffer for this face
#define SURFDRAW_TANGENTSPACE	0x00000100		// This surface needs a tangent space
#define SURFDRAW_NOCULL			0x00000200		// Don't bother backface culling these
#define SURFDRAW_HASLIGHTSYTLES 0x00000400		// has a lightstyle other than 0
#define SURFDRAW_HAS_DISP		0x00000800		// has a dispinfo
#define SURFDRAW_ALPHATEST		0x00001000		// Is alphstested
#define SURFDRAW_NOSHADOWS		0x00002000		// No shadows baby
#define SURFDRAW_NODECALS		0x00004000		// No decals baby
#define SURFDRAW_HAS_PRIMS		0x00008000		// has a list of prims
#define SURFDRAW_WATERSURFACE	0x00010000	// This is a water surface
#define SURFDRAW_UNDERWATER		0x00020000
#define SURFDRAW_ABOVEWATER		0x00040000
#define SURFDRAW_HASDLIGHT		0x00080000	// Has some kind of dynamic light that must be checked
#define SURFDRAW_DLIGHTPASS		0x00100000	// Must be drawn in the dlight pass
#define SURFDRAW_UNUSED2		0x00200000	// unused
#define SURFDRAW_VERTCOUNT_MASK	0xFF000000	// 8 bits of vertex count
#define SURFDRAW_SORTGROUP_MASK	0x00C00000	// 2 bits of sortgroup

#define SURFDRAW_VERTCOUNT_SHIFT	24
#define SURFDRAW_SORTGROUP_SHIFT	22

// NOTE: 16-bytes, preserve size/alignment - we index this alot
struct msurface1_t
{
	// garymct - are these needed? - used by decal projection code
	int		textureMins[2];		// smallest unnormalized s/t position on the surface.
	short	textureExtents[2];	// ?? s/t texture size, 1..512 for all non-sky surfaces

	struct
	{
		unsigned short numPrims;
		unsigned short firstPrimID;			// index into primitive list if numPrims > 0
	} prims;
};

#pragma pack(1)
struct msurfacenormal_t
{
	unsigned int firstvertnormal;
	//	unsigned short	firstvertnormal;
		// FIXME: Should I just point to the leaf here since it has this data?????????????
	//	short fogVolumeID;			// -1 if not in fog  
};
#pragma pack()

// This is a single cache line (32 bytes)
struct msurfacelighting_t
{
	// You read that minus sign right. See the comment below.
	ColorRGBExp32* AvgLightColor(int nLightStyleIndex) { return m_pSamples - (nLightStyleIndex + 1); }

	// Lightmap info
	short m_LightmapMins[2];
	short m_LightmapExtents[2];
	short m_OffsetIntoLightmapPage[2];

	int m_nLastComputedFrame;	// last frame the surface's lightmap was recomputed
	int m_fDLightBits;			// Indicates which dlights illuminates this surface.
	int m_nDLightFrame;			// Indicates the last frame in which dlights illuminated this surface

	unsigned char m_nStyles[MAXLIGHTMAPS];	// index into d_lightstylevalue[] for animated lights 
	// no one surface can be effected by more than 4 
	// animated lights.

// NOTE: This is tricky. To get this to fit in a single cache line,
// and to save the extra memory of not having to store average light colors for
// lightstyles that are not used, I store between 0 and 4 average light colors +before+
// the samples, depending on how many lightstyles there are. Naturally, accessing
// an average color for an undefined lightstyle on the surface results in undefined results.
// 0->4 avg light colors, *in reverse order from m_nStyles* + [numstyles*surfsize]
	ColorRGBExp32* m_pSamples;
};

const int WORLD_DECAL_HANDLE_INVALID = 0xFFFF;
typedef unsigned short WorldDecalHandle_t;

#pragma pack(1)
// NOTE: 32-bytes.  Aligned/indexed often
struct msurface2_t
{
	unsigned int			flags;			// see SURFDRAW_ #defines (only 22-bits right now)
	// These are packed in to flags now
	//unsigned char			vertCount;		// number of verts for this surface
	//unsigned char			sortGroup;		// only uses 2 bits, subdivide?
	cplane_t* plane;			// pointer to shared plane	
	int						firstvertindex;	// look up in model->vertindices[] (only uses 17-18 bits?)
	WorldDecalHandle_t		decals;
	ShadowDecalHandle_t		m_ShadowDecals; // unsigned short
	OverlayFragmentHandle_t m_nFirstOverlayFragment;	// First overlay fragment on the surface (short)
	short					materialSortID;
	unsigned short			vertBufferIndex;

	unsigned short			m_bDynamicShadowsEnabled : 1;	// Can this surface receive dynamic shadows?
	unsigned short			texinfo : 15;

	IDispInfo* pDispInfo;         // displacement map information
	int						visframe;		// should be drawn when node is crossed
};
#pragma pack()

class model_t : public IVModel
{
public:
	void Init();
	void Destory();
	bool Load(CLumpHeaderInfo& header);
	//char* GetMapName();
	int GetCMNodesCount();
	cnode_t* GetNodes(int index);
	int GetLeafsCount();
	cleaf_t* GetLeafs(int index);
	int GetPlanesCount();
	cplane_t* GetPlane(int index);
	int GetBrushesCount();
	cbrush_t* GetBrushes(int index);
	int GetCModelsCount();
	cmodel_t* GetCModels(int index);
	unsigned short GetLeafBrushes(int index);
	cboxbrush_t* GetBoxBrushes(int index);
	cbrushside_t* GetBrushesSide(int index);
	int GetClustersCount();
	int GetVisibilityCount();
	dvis_t* GetVis();
	int GetAreaCount();
	carea_t* GetArea(int index);
	int GetCMAreaPortalsCount();
	dareaportal_t* GetCMAreaPortals(int index);
	int GetFloodvalid();
	void IncFloodvalid();
	void InitPortalOpenState();
	bool GetPortalOpenState(int index);
	void SetPortalOpenState(int index, bool state);
	void SetDispListCount(int count);
	int GetDispListCount();
	CRangeValidatedArray<unsigned short>* GetDispList();
	unsigned short GetDispList(int index);
	unsigned short* GetDispListBase();
	csurface_t GetNullSurface();
	csurface_t* GetSurfaceAtIndex(unsigned short surfaceIndex);
	int GetTexturesCount();
	int GetEntityCharsCount();
	char* GetEntityString();
	void DiscardEntityString();
	int GetDispCollTreesCount();
	CDispCollTree* GetDispCollTrees(int index);
	alignedbbox_t* GetDispBounds(int index);
	bool CollisionBSPData_Init();
	void CollisionBSPData_Destroy();

	void CollisionBSPData_PreLoad();
	bool CollisionBSPData_Load(CLumpHeaderInfo& header);
	//void CollisionBSPData_PostLoad();
	CUtlVector<CPhysCollide*>& GetTerrainList() {
		return brush.pShared->m_TerrainList;
	}
	CVirtualTerrain& GetVirtualTerrain() {
		return brush.pShared->m_VirtualTerrain;
	}
private:


	//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadTextures(CLumpHeaderInfo& header);
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadTexinfo(CLumpHeaderInfo& header,
		CUtlVector<unsigned short>& map_texinfo);
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadLeafs_Version_0(CLumpHeaderInfo& header, CLumpInfo& lh);
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadLeafs_Version_1(CLumpHeaderInfo& header, CLumpInfo& lh);

	void CollisionBSPData_LoadLeafs(CLumpHeaderInfo& header);
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadLeafBrushes(CLumpHeaderInfo& header);
	//----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadPlanes(CLumpHeaderInfo& header);
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadBrushes(CLumpHeaderInfo& header);
	inline bool IsBoxBrush(const cbrush_t& brush, dbrushside_t* pSides, cplane_t* pPlanes);
	inline void ExtractBoxBrush(cboxbrush_t* pBox, const cbrush_t& brush, dbrushside_t* pSides, cplane_t* pPlanes, CUtlVector<unsigned short>& map_texinfo);
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadBrushSides(CLumpHeaderInfo& header, CUtlVector<unsigned short>& map_texinfo);
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadSubmodels(CLumpHeaderInfo& header);
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadNodes(CLumpHeaderInfo& header);
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadAreas(CLumpHeaderInfo& header);
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadAreaPortals(CLumpHeaderInfo& header);
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadVisibility(CLumpHeaderInfo& header);
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadEntityString(CLumpHeaderInfo& header);
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadPhysics(CLumpHeaderInfo& header);
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadDispInfo(CLumpHeaderInfo& header);



	class CDispLeafBuilder
	{
	public:
		CDispLeafBuilder(model_t* mod)
		{
			this->m_mod = mod;
			// don't want this to resize much, so make the backing buffer large
			m_dispList.EnsureCapacity(MAX_MAP_DISPINFO * 2);

			// size both of these to the size of the array since there is exactly one per element
			m_leafCount.SetCount(m_mod->GetDispCollTreesCount());
			m_firstIndex.SetCount(m_mod->GetDispCollTreesCount());
			for (int i = 0; i < m_mod->GetDispCollTreesCount(); i++)
			{
				m_leafCount[i] = 0;
				m_firstIndex[i] = -1;
			}
		}

		void BuildLeafListForDisplacement(int index)
		{
			// get tree and see if it is real (power != 0)
			CDispCollTree* pDispTree = m_mod->GetDispCollTrees(index);
			if (!pDispTree || (pDispTree->GetPower() == 0))
				return;
			m_firstIndex[index] = m_dispList.Count();
			m_leafCount[index] = 0;
			const int MAX_NODES = 1024;
			int nodeList[MAX_NODES];
			int listRead = 0;
			int listWrite = 1;
			nodeList[0] = m_mod->GetCModels(0)->headnode;
			Vector mins, maxs;
			pDispTree->GetBounds(mins, maxs);

			// UNDONE: The rendering code did this, do we need it?
	//		mins -= Vector( 0.5, 0.5, 0.5 );
	//		maxs += Vector( 0.5, 0.5, 0.5 );

			while (listRead != listWrite)
			{
				int nodeIndex = nodeList[listRead];
				listRead = (listRead + 1) % MAX_NODES;

				// if this is a leaf, add it to the array
				if (nodeIndex < 0)
				{
					int leafIndex = -1 - nodeIndex;
					m_dispList.AddToTail(leafIndex);
					m_leafCount[index]++;
				}
				else
				{
					//
					// choose side(s) to traverse
					//
					cnode_t* pNode = m_mod->GetNodes(nodeIndex);
					cplane_t* pPlane = pNode->plane;

					int sideResult = BOX_ON_PLANE_SIDE(mins, maxs, pPlane);

					// front side
					if (sideResult & 1)
					{
						nodeList[listWrite] = pNode->children[0];
						listWrite = (listWrite + 1) % MAX_NODES;
						Assert(listWrite != listRead);
					}
					// back side
					if (sideResult & 2)
					{
						nodeList[listWrite] = pNode->children[1];
						listWrite = (listWrite + 1) % MAX_NODES;
						Assert(listWrite != listRead);
					}
				}
			}
		}
		int GetDispListCount() { return m_dispList.Count(); }
		void WriteLeafList(unsigned short* pLeafList)
		{
			// clear current count if any
			for (int i = 0; i < m_mod->GetLeafsCount(); i++)
			{
				cleaf_t* pLeaf = m_mod->GetLeafs(i);
				pLeaf->dispCount = 0;
			}
			// compute new count per leaf
			for (int i = 0; i < m_dispList.Count(); i++)
			{
				int leafIndex = m_dispList[i];
				cleaf_t* pLeaf = m_mod->GetLeafs(leafIndex);
				pLeaf->dispCount++;
			}
			// point each leaf at the start of it's output range in the output array
			unsigned short firstDispIndex = 0;
			for (int i = 0; i < m_mod->GetLeafsCount(); i++)
			{
				cleaf_t* pLeaf = m_mod->GetLeafs(i);
				pLeaf->dispListStart = firstDispIndex;
				firstDispIndex += pLeaf->dispCount;
				pLeaf->dispCount = 0;
			}
			// now iterate the references in disp order adding to each leaf's (now compact) list
			// for each displacement with leaves
			for (int i = 0; i < m_leafCount.Count(); i++)
			{
				// for each leaf in this disp's list
				int count = m_leafCount[i];
				for (int j = 0; j < count; j++)
				{
					int listIndex = m_firstIndex[i] + j;					// index to per-disp list
					int leafIndex = m_dispList[listIndex];					// this reference is for one leaf
					cleaf_t* pLeaf = m_mod->GetLeafs(leafIndex);
					int outListIndex = pLeaf->dispListStart + pLeaf->dispCount;	// output position for this leaf
					pLeafList[outListIndex] = i;							// write the reference there
					Assert(outListIndex < GetDispListCount());
					pLeaf->dispCount++;										// move this leaf's output pointer
				}
			}
		}

	private:
		model_t* m_mod;
		// this is a list of all of the leaf indices for each displacement
		CUtlVector<unsigned short> m_dispList;
		// this is the first entry into dispList for each displacement
		CUtlVector<int> m_firstIndex;
		// this is the # of leaf entries for each displacement
		CUtlVector<unsigned short> m_leafCount;
	};
	// setup
	void CM_DispTreeLeafnum();


public:
	virtual int ModelFrameCount() const;
	virtual bool IsTranslucent() const;
	virtual int GetModelType() const;
	virtual void GetModelRenderBounds(Vector& mins, Vector& maxs) const;
	virtual studiohdr_t* GetStudiomodel() const;
	virtual bool IsModelVertexLit() const;
	virtual bool ModelHasMaterialProxy() const;
	virtual void Mod_RecomputeTranslucency(int nSkin, int nBody, void /*IClientRenderable*/* pClientRenderable, float fInstanceAlphaModulate = 1.0f);
	virtual void* GetModelExtraData() const;
	virtual const char* GetModelName() const;
	virtual void GetIlluminationPoint(IClientRenderable* pRenderable, Vector const& origin,
		QAngle const& angles, Vector* pLightingCenter) const;
	virtual bool IsTranslucentTwoPass() const;
	virtual MDLHandle_t	GetCacheHandle() const;
	virtual void GetModelMaterialColorAndLighting(IVModel* pWorld, const Vector& origin,
		const QAngle& angles, trace_t* pTrace, Vector& lighting, Vector& matColor) const;
	virtual void GetModelBounds(Vector& mins, Vector& maxs) const;
	virtual int GetModelSpriteWidth() const;
	virtual int GetModelSpriteHeight() const;
	virtual const char* GetModelKeyValueText() const;
	virtual bool IsUsingFBTexture(int nSkin, int nBody, void /*IClientRenderable*/* pClientRenderable) const;
	virtual int GetModelContents() const;
	virtual vcollide_t* GetVCollide() const;
	virtual int R_GetBrushModelPlaneCount() const;
	virtual const cplane_t& R_GetBrushModelPlane(int nIndex, Vector* pOrigin) const;
	virtual bool GetModelKeyValue(CUtlBuffer& buf) const;
	virtual CPhysCollide* GetCollideForVirtualTerrain(int index) const;

	// Returns the number of leaves
	int LeafCount() const;
	// Enumerates the leaves along a ray, box, etc.
	bool EnumerateLeavesAtPoint(const Vector& pt, ISpatialLeafEnumerator* pEnum, intp context);
	bool EnumerateLeavesInBox(const Vector& mins, const Vector& maxs, ISpatialLeafEnumerator* pEnum, intp context);
	bool EnumerateLeavesInSphere(const Vector& center, float radius, ISpatialLeafEnumerator* pEnum, intp context);
	bool EnumerateLeavesAlongRay(Ray_t const& ray, ISpatialLeafEnumerator* pEnum, intp context);
	int LeafToIndex(mleaf_t* pLeaf);

	inline class IDispInfo* MLeaf_Disaplcement(mleaf_t* pLeaf, int index)
	{
		Assert(index < pLeaf->dispCount);
		int dispIndex = brush.pShared->m_pDispInfoReferences[pLeaf->dispListStart + index];
		return DispInfo_IndexArray(brush.pShared->hDispInfos, dispIndex);
	}
	inline int MSurf_Index(SurfaceHandle_t surfID)
	{
		int surfaceIndex = surfID - brush.pShared->surfaces2;
		Assert(surfaceIndex >= 0 && surfaceIndex < brush.pShared->numsurfaces);
		return surfaceIndex;
	}
	inline SurfaceHandle_t SurfaceHandleFromIndex(int surfaceIndex) const
	{
		return &brush.pShared->surfaces2[surfaceIndex];
	}
	inline int& MSurf_DLightBits(SurfaceHandle_t surfID)
	{
		int surfaceIndex = MSurf_Index(surfID);
		//	ASSERT_SURF_VALID( surfID );
		Assert(brush.pShared);
		return brush.pShared->surfacelighting[surfaceIndex].m_fDLightBits;
	}
	inline int* MSurf_TextureMins(SurfaceHandle_t surfID)
	{
		int surfaceIndex = MSurf_Index(surfID);
		//	ASSERT_SURF_VALID( surfID );
		Assert(brush.pShared);
		return brush.pShared->surfaces1[surfaceIndex].textureMins;
	}
	inline short* MSurf_TextureExtents(SurfaceHandle_t surfID)
	{
		int surfaceIndex = MSurf_Index(surfID);
		//	ASSERT_SURF_VALID( surfID );
		Assert(brush.pShared);
		return brush.pShared->surfaces1[surfaceIndex].textureExtents;
	}
	inline short* MSurf_LightmapMins(SurfaceHandle_t surfID)
	{
		int surfaceIndex = MSurf_Index(surfID);
		//	ASSERT_SURF_VALID( surfID );
		Assert(brush.pShared);
		return brush.pShared->surfacelighting[surfaceIndex].m_LightmapMins;
	}
	inline short* MSurf_LightmapExtents(SurfaceHandle_t surfID)
	{
		int surfaceIndex = MSurf_Index(surfID);
		//	ASSERT_SURF_VALID( surfID );
		Assert(brush.pShared);
		return brush.pShared->surfacelighting[surfaceIndex].m_LightmapExtents;
	}
	inline mtexinfo_t* MSurf_TexInfo(SurfaceHandle_t surfID)
	{
		return &brush.pShared->texinfo[surfID->texinfo];
	}
	inline ColorRGBExp32* MSurf_AvgLightColor(SurfaceHandle_t surfID, int nIndex)
	{
		int surfaceIndex = MSurf_Index(surfID);
		//	ASSERT_SURF_VALID( surfID );
		Assert(brush.pShared);
		return brush.pShared->surfacelighting[surfaceIndex].AvgLightColor(nIndex);
	}
	inline byte* MSurf_Styles(SurfaceHandle_t surfID)
	{
		int surfaceIndex = MSurf_Index(surfID);
		//	ASSERT_SURF_VALID( surfID );
		Assert(brush.pShared);
		return brush.pShared->surfacelighting[surfaceIndex].m_nStyles;
	}
	inline bool SurfaceHasDispInfo(SurfaceHandle_t surfID)
	{
		return (MSurf_Flags(surfID) & SURFDRAW_HAS_DISP) ? true : false;
	}
	inline bool SurfaceHasPrims(SurfaceHandle_t surfID)
	{
		return (MSurf_Flags(surfID) & SURFDRAW_HAS_PRIMS) ? true : false;
	}
	inline unsigned short MSurf_NumPrims(SurfaceHandle_t surfID)
	{
		if (SurfaceHasDispInfo(surfID) || !SurfaceHasPrims(surfID))
			return 0;

		int surfaceIndex = MSurf_Index(surfID);
		//	ASSERT_SURF_VALID( surfID );
		Assert(brush.pShared);
		return brush.pShared->surfaces1[surfaceIndex].prims.numPrims;
	}
	inline unsigned short MSurf_FirstPrimID(SurfaceHandle_t surfID)
	{
		if (SurfaceHasDispInfo(surfID))
			return 0;
		int surfaceIndex = MSurf_Index(surfID);
		//	ASSERT_SURF_VALID( surfID );
		Assert(brush.pShared);
		return brush.pShared->surfaces1[surfaceIndex].prims.firstPrimID;
	}
	inline ColorRGBExp32* MSurf_Samples(SurfaceHandle_t surfID)
	{
		int surfaceIndex = MSurf_Index(surfID);
		//	ASSERT_SURF_VALID( surfID );
		Assert(brush.pShared);
		return brush.pShared->surfacelighting[surfaceIndex].m_pSamples;
	}
	inline IDispInfo* MSurf_DispInfo(SurfaceHandle_t surfID)
	{
		return surfID->pDispInfo;
	}
	//inline unsigned short &MSurf_FirstVertNormal( SurfaceHandle_t surfID, worldbrushdata_t *pData = host_state.worldbrush )
	inline unsigned int& MSurf_FirstVertNormal(SurfaceHandle_t surfID)
	{
		int surfaceIndex = MSurf_Index(surfID);
		//	ASSERT_SURF_VALID( surfID );
		Assert(brush.pShared);
		Assert(brush.pShared->surfacenormals[surfaceIndex].firstvertnormal < MAX_MAP_VERTNORMALINDICES);
		return brush.pShared->surfacenormals[surfaceIndex].firstvertnormal;
	}
	inline short& MSurf_MaterialSortID(SurfaceHandle_t surfID)
	{
		return surfID->materialSortID;
	}
	inline short* MSurf_OffsetIntoLightmapPage(SurfaceHandle_t surfID)
	{
		int surfaceIndex = MSurf_Index(surfID);
		//	ASSERT_SURF_VALID( surfID );
		Assert(brush.pShared);
		return brush.pShared->surfacelighting[surfaceIndex].m_OffsetIntoLightmapPage;
	}
	inline cplane_t& MSurf_Plane(SurfaceHandle_t surfID)
	{
		return *surfID->plane;
	}
	inline VPlane MSurf_GetForwardFacingPlane(SurfaceHandle_t surfID)
	{
		//	ASSERT_SURF_VALID( surfID );
		Assert(brush.pShared);
		return VPlane(MSurf_Plane(surfID).normal, MSurf_Plane(surfID).dist);
	}
	inline OverlayFragmentHandle_t& MSurf_OverlayFragmentList(SurfaceHandle_t surfID)
	{
		return surfID->m_nFirstOverlayFragment;
	}
	inline msurfacelighting_t* SurfaceLighting(SurfaceHandle_t surfID)
	{
		int surfaceIndex = MSurf_Index(surfID);
		Assert(brush.pShared);
		return &brush.pShared->surfacelighting[surfaceIndex];
	}
	inline unsigned short MSurf_AreDynamicShadowsEnabled(SurfaceHandle_t surfID)
	{
		return surfID->m_bDynamicShadowsEnabled;
	}
	//inline const SurfaceHandle_t SurfaceHandleFromIndex(int surfaceIndex)
	//{
	//	return &brush.pShared->surfaces2[surfaceIndex];
	//}
#if _DEBUG
#define ASSERT_SURF_VALID(surfID) MSurf_Index(surfID)
#else
#define ASSERT_SURF_VALID(surfID)
#endif

	inline unsigned int& MSurf_Flags(SurfaceHandle_t surfID)
	{
		return surfID->flags;
	}
	inline int& MSurf_VisFrame(SurfaceHandle_t surfID)
	{
		return surfID->visframe;
	}

	inline int MSurf_SortGroup(SurfaceHandle_t surfID)
	{
		return (surfID->flags & SURFDRAW_SORTGROUP_MASK) >> SURFDRAW_SORTGROUP_SHIFT;
	}

	inline void MSurf_SetSortGroup(SurfaceHandle_t surfID, int sortGroup)
	{
		unsigned int flags = (sortGroup << SURFDRAW_SORTGROUP_SHIFT) & SURFDRAW_SORTGROUP_MASK;
		surfID->flags |= flags;
	}
	/*
	inline int& MSurf_DLightFrame( SurfaceHandle_t surfID, worldbrushdata_t *pData = host_state.worldbrush )
	{
	//	ASSERT_SURF_VALID( surfID );
		Assert( pData );
		return pData->surfacelighting[surfID].dlightframe;
	}
	*/
	inline int& MSurf_FirstVertIndex(SurfaceHandle_t surfID)
	{
		return surfID->firstvertindex;
	}

	inline int MSurf_VertCount(SurfaceHandle_t surfID) const
	{
		return (surfID->flags >> SURFDRAW_VERTCOUNT_SHIFT) & 0xFF;
	}

	inline void MSurf_SetVertCount(SurfaceHandle_t surfID, int vertCount)
	{
		int flags = (vertCount << SURFDRAW_VERTCOUNT_SHIFT) & SURFDRAW_VERTCOUNT_MASK;
		surfID->flags |= flags;
	}
	inline short MSurf_MaxLightmapSizeWithBorder(SurfaceHandle_t surfID)
	{
		//	ASSERT_SURF_VALID( surfID );
		return SurfaceHasDispInfo(surfID) ? MAX_DISP_LIGHTMAP_DIM_INCLUDING_BORDER : MAX_BRUSH_LIGHTMAP_DIM_INCLUDING_BORDER;
	}

	inline short MSurf_MaxLightmapSizeWithoutBorder(SurfaceHandle_t surfID)
	{
		//	ASSERT_SURF_VALID( surfID );
		return SurfaceHasDispInfo(surfID) ? MAX_DISP_LIGHTMAP_DIM_WITHOUT_BORDER : MAX_BRUSH_LIGHTMAP_DIM_WITHOUT_BORDER;
	}
	inline WorldDecalHandle_t& MSurf_Decals(SurfaceHandle_t surfID)
	{
		return surfID->decals;
	}
	inline bool SurfaceHasDecals(SurfaceHandle_t surfID)
	{
		return (MSurf_Decals(surfID) != WORLD_DECAL_HANDLE_INVALID) ? true : false;
	}

	inline ShadowDecalHandle_t& MSurf_ShadowDecals(SurfaceHandle_t surfID)
	{
		return surfID->m_ShadowDecals;
	}
	/*
	inline int *MSurf_CachedLight( SurfaceHandle_t surfID, worldbrushdata_t *pData = host_state.worldmodel->brush.pShared )
	{
	//	ASSERT_SURF_VALID( surfID );
		Assert( pData );
		return pData->surfacelighting[surfID].cached_light;
	}

	inline short& MSurf_CachedDLight( SurfaceHandle_t surfID, worldbrushdata_t *pData = host_state.worldmodel->brush.pShared )
	{
	//	ASSERT_SURF_VALID( surfID );
		Assert( pData );
		return pData->surfacelighting[surfID].cached_dlight;
	}
	*/
	inline unsigned short& MSurf_VertBufferIndex(SurfaceHandle_t surfID)
	{
		return surfID->vertBufferIndex;
	}
	Vector& GetMins() {
		return mins;
	}
	Vector& GetMaxs() {
		return maxs;
	}
	int GetPrimitivesCount() {
		return brush.pShared->numprimitives;
	}
	mprimitive_t* GetPrimitives(unsigned short index) {
		return &brush.pShared->primitives[index];
	}
	unsigned short GetPrimindices(unsigned short index) {
		return brush.pShared->primindices[index];
	}
	int GetCubemapSamplesCount() {
		return brush.pShared->m_nCubemapSamples;
	}
	mcubemapsample_t* GetCubemapSamples(int index) {
		return &brush.pShared->m_pCubemapSamples[index];
	}
	SurfaceHandle_t* GetMarkSurface(unsigned short index) {
		return &brush.pShared->marksurfaces[index];
	}
	unsigned short* GetVertindices(int index) {
		return &brush.pShared->vertindices[index];
	}
	mvertex_t* GetVertexes(int index){
		return &brush.pShared->vertexes[index];
	}
	Vector& GetVertnormals(int index) {
		return brush.pShared->vertnormals[index];
	}
	unsigned short GetVertnormalindices(int index) {
		return brush.pShared->vertnormalindices[index];
	}
	int GetLeafCount() {
		return brush.pShared->numleafs;
	}
	mleaf_t* GetMLeafs(int index) {
		return &brush.pShared->leafs[index];
	}
	int GetLeafArea(int index) {
		return brush.pShared->leafs[index].area;
	}
	int GetNodesCount() {
		return brush.pShared->numnodes;
	}
	mnode_t* GetNode(int index) {
		return &brush.pShared->nodes[index];
	}
	int GetFirstNodeNum() {
		return brush.firstnode;
	}
	mnode_t* GetFirstNode() {
		return &brush.pShared->nodes[brush.firstnode];
	}
	HDISPINFOARRAY GetDispInfos() const{
		return brush.pShared->hDispInfos;
	}
	void SetDispInfos(HDISPINFOARRAY hDispInfos) {
		brush.pShared->hDispInfos = hDispInfos;
	}
	int GetPrimvertsCount() {
		return brush.pShared->numprimverts;
	}
	mprimvert_t& GetPrimverts(int index) {
		return brush.pShared->primverts[index];
	}
	ColorRGBExp32* GetLightdata() {
		return brush.pShared->lightdata;
	}
	void SetLightdata(ColorRGBExp32* lightdata) {
		brush.pShared->lightdata = lightdata;
	}
	int GetSurfacesCount() {
		return brush.pShared->numsurfaces;
	}
	msurfacelighting_t* GetSurfacelighting() {
		return brush.pShared->surfacelighting;
	}
	void SetUnloadedlightmaps(bool unloadedlightmaps) {
		brush.pShared->unloadedlightmaps = unloadedlightmaps;
	}
	bool GetUnloadedlightmaps() {
		return brush.pShared->unloadedlightmaps;
	}
	modtype_t GetModelType() {
		return type;
	}
	MDLHandle_t		GetStudio() const{
		return studio;
	}
	mleafambientindex_t* GetLeafAmbient(int index) {
		return &brush.pShared->m_pLeafAmbient[index];
	}
	mleafambientlighting_t& GetAmbientSamples(int index) {
		return brush.pShared->m_pAmbientSamples[index];
	}
	int GetTexinfoCount() {
		return brush.pShared->numtexinfo;
	}
	mtexinfo_t* GetTexinfo(int index) {
		return &brush.pShared->texinfo[index];
	}
	int GetModelsurfacesCount() const{
		return brush.nummodelsurfaces;
	}
	int GetFirstmodelsurface() const{
		return brush.firstmodelsurface;
	}
	int GetLeafwaterdataCount() {
		return brush.pShared->numleafwaterdata;
	}
	mleafwaterdata_t* GetLeafwaterdata(int index) {
		return &brush.pShared->leafwaterdata[index];
	}
	unsigned short* GetLeafMinDistToWater(int index) {
		return &brush.pShared->m_LeafMinDistToWater[index];
	}
	int			GetSubmodelsCount() {
		return brush.pShared->numsubmodels;
	}
	void SetRenderHandle(unsigned short	renderHandle) {
		brush.renderHandle= renderHandle;
	}
	unsigned short	GetRenderHandle() {
		return brush.renderHandle;
	}
	int GetDispInfosCount() {
		return brush.pShared->numDispInfos;
	}
	void SetDispInfosCount(int numDispInfos) {
		brush.pShared->numDispInfos = numDispInfos;
	}
	int& GetModelFlag() {
		return flags;
	}
	int& GetLoadFlags() {
		return nLoadFlags;
	}
	int& GetServerCount() {
		return nServerCount;
	}
	void SetBoundsFromStudioHdr(MDLHandle_t handle);
	void SetModelType(modtype_t	type) {
		this->type = type;
	}
	void SetStudio(MDLHandle_t		studio) {
		this->studio = studio;
	}
	IMaterial** GetMaterials() {
		return ppMaterials;
	}
	int	GetWorldlightsCount() {
		return brush.pShared->numworldlights;
	}
	dworldlight_t* GetWorldlights(int index) {
		return &brush.pShared->worldlights[index];
	}

	int	GetAreaPortalsCount() {
		return brush.pShared->m_nAreaPortals;
	}
	dareaportal_t* GetAreaPortals(int index) {
		return &brush.pShared->m_pAreaPortals[index];
	}
	Vector GetClipPortalVerts(int index) {
		return brush.pShared->m_pClipPortalVerts[index];
	}
	lightzbuffer_t* GetShadowzbuffers(int index) {
		return &brush.pShared->shadowzbuffers[index];
	}
	FileNameHandle_t	GetFnHandle() {
		return fnHandle;
	}
	float				GetRadius() const{
		return radius;
	}
	int* GetOccludervertindices(int index) {
		return &brush.pShared->occludervertindices[index];
	}
	doccluderdata_t* GetOccluders(int index) {
		return &brush.pShared->occluders[index];
	}
	int			GetOccludersCount() {
		return brush.pShared->numoccluders;
	}
	doccluderpolydata_t* GetOccluderpolys(int index) {
		return &brush.pShared->occluderpolys[index];
	}
	cplane_t& GetPlanes(int index) {
		return brush.pShared->map_planes[index];
	}
	int	GetAreasCount() {
		return brush.pShared->m_nAreas;
	}
	darea_t* GetAreas(int index) {
		return &brush.pShared->m_pAreas[index];
	}
	int Mod_GameLumpSize(int lumpId);
	int Mod_GameLumpVersion(int lumpId);
	bool Mod_LoadGameLump(int lumpId, void* pBuffer, int size);
	// returns the material count...
	int Mod_GetMaterialCount();
	// returns the first n materials.
	int Mod_GetModelMaterials(int count, IMaterial** ppMaterial);
	int Mod_GetInlineModelIndex() {
		return brush.inlineModelIndex;
	}
private:
	void Mod_LoadLighting(CLumpHeaderInfo& header, CLumpInfo& lh);
	void Mod_LoadWorldlights(CLumpInfo& lh, bool bIsHDR);
	void Mod_LoadVertices(CLumpHeaderInfo& header);
	void Mod_LoadSubmodels(CLumpHeaderInfo& header,CUtlVector<mmodel_t>& submodelList);
	medge_t* Mod_LoadEdges(CLumpHeaderInfo& header);
	void Mod_LoadOcclusion(CLumpHeaderInfo& header);
	//void Mod_LoadTexdata(CLumpHeaderInfo& header);
	void Mod_LoadTexinfo(CLumpHeaderInfo& header);
	void Mod_LoadVertNormals(CLumpHeaderInfo& header);
	void Mod_LoadVertNormalIndices(CLumpHeaderInfo& header);
	void Mod_LoadPrimitives(CLumpHeaderInfo& header);
	void Mod_LoadPrimVerts(CLumpHeaderInfo& header);
	void Mod_LoadPrimIndices(CLumpHeaderInfo& header);
	void Mod_LoadFaces(CLumpHeaderInfo& header);
	void Mod_LoadNodes(CLumpHeaderInfo& header);
	void Mod_LoadLeafs_Version_0(CLumpHeaderInfo& header, CLumpInfo& lh);
	void Mod_LoadLeafs_Version_1(CLumpHeaderInfo& header, CLumpInfo& lh, CLumpInfo& ambientLightingLump, CLumpInfo& ambientLightingTable);
	void Mod_LoadLeafs(CLumpHeaderInfo& header);
	void Mod_LoadLeafWaterData(CLumpHeaderInfo& header);
	void Mod_LoadCubemapSamples(CLumpHeaderInfo& header);
	void Mod_LoadLeafMinDistToWater(CLumpHeaderInfo& header);
	void Mod_LoadMarksurfaces(CLumpHeaderInfo& header);
	void Mod_LoadSurfedges(CLumpHeaderInfo& header,medge_t* pedges);
	//void Mod_LoadPlanes(CLumpHeaderInfo& header);
	void Mod_LoadGameLumpDict(CLumpHeaderInfo& header);


	FileNameHandle_t	fnHandle;
	CUtlString			strName;

	int					nLoadFlags;		// mark loaded/not loaded
	int					nServerCount;	// marked at load
	IMaterial			**ppMaterials;	// null-terminated runtime material cache; ((intptr_t*)(ppMaterials))[-1] == nMaterials

	modtype_t			type;
	int					flags;			// MODELFLAG_???

	// volume occupied by the model graphics	
	Vector				mins, maxs;
	float				radius;

	union
	{
		brushdata_t		brush;
		MDLHandle_t		studio;
		spritedata_t	sprite;
	};

	friend class CModelLoader;
};

inline short& MSurf_MaterialSortID(SurfaceHandle_t surfID)
{
	return surfID->materialSortID;
}
//-----------------------------------------------------------------------------
// Decals
//-----------------------------------------------------------------------------

struct decallist_t
{
	DECLARE_SIMPLE_DATADESC();

	Vector		position;
	char		name[ 128 ];
	short		entityIndex;
	byte		depth;
	byte		flags;

	// This is the surface plane that we hit so that we can move certain decals across
	//  transitions if they hit similar geometry
	Vector		impactPlaneNormal;
};


#endif // GL_MODEL_PRIVATE_H
