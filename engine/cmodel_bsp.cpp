//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "tier0/platform.h"
#include "sysexternal.h"
#include "cmodel_engine.h"
#include "dispcoll_common.h"
#include "modelloader.h"
#include "common.h"
#include "zone.h"
#include "gl_model_private.h"

// UNDONE: Abstract the texture/material lookup stuff and all of this goes away
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/materialsystem_config.h"
extern IMaterialSystem *materials;

#include "vphysics_interface.h"
#include "sys_dll.h"
#include "tier2/tier2.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern int g_iServerGameDLLVersion;
IPhysicsSurfaceProps *physprop = NULL;
IPhysicsCollision	 *physcollision = NULL;

//CCollisionBSPData g_BSPData;								// the global collision bsp
//#define g_BSPData dont_use_g_BSPData_directly

// local forward declarations
//void CollisionBSPData_LoadTextures( CCollisionBSPData *pBSPData );
//void CollisionBSPData_LoadTexinfo( CCollisionBSPData *pBSPData, CUtlVector<unsigned short> &map_texinfo );
//void CollisionBSPData_LoadLeafs_Version_0( CCollisionBSPData *pBSPData );
//void CollisionBSPData_LoadLeafs_Version_1( CCollisionBSPData *pBSPData );
//void CollisionBSPData_LoadLeafs( CCollisionBSPData *pBSPData );
//void CollisionBSPData_LoadLeafBrushes( CCollisionBSPData *pBSPData );
//void CollisionBSPData_LoadPlanes( CCollisionBSPData *pBSPData );
//void CollisionBSPData_LoadBrushes( CCollisionBSPData *pBSPData );
//void CollisionBSPData_LoadBrushSides( CCollisionBSPData *pBSPData, CUtlVector<unsigned short> &map_texinfo );
//void CollisionBSPData_LoadSubmodels( CCollisionBSPData *pBSPData );
//void CollisionBSPData_LoadNodes( CCollisionBSPData *pBSPData );
//void CollisionBSPData_LoadAreas( CCollisionBSPData *pBSPData );
//void CollisionBSPData_LoadAreaPortals( CCollisionBSPData *pBSPData );
//void CollisionBSPData_LoadVisibility( CCollisionBSPData *pBSPData );
//void CollisionBSPData_LoadEntityString( CCollisionBSPData *pBSPData );
//void CollisionBSPData_LoadPhysics( CCollisionBSPData *pBSPData );
//void CollisionBSPData_LoadDispInfo( CCollisionBSPData *pBSPData );

csurface_t worldbrushdata_t::nullsurface = { "**empty**", 0, 0 };				// generic null collision model surface
csurface_t* model_t::GetSurfaceAtIndex(unsigned short surfaceIndex)
{
	if (surfaceIndex == SURFACE_INDEX_INVALID)
	{
		return &worldbrushdata_t::nullsurface;
	}
	return &brush.pShared->map_surfaces[surfaceIndex];
}
int model_t::GetBrushesCount() {
	return brush.pShared->numbrushes;
}
int model_t::GetCModelsCount() {
	return brush.pShared->numcmodels;
}
cmodel_t* model_t::GetCModels(int index) {
	return &brush.pShared->map_cmodels[index];
}
cleaf_t* model_t::GetLeafs(int index) {
	return &brush.pShared->map_leafs[index];
}
unsigned short model_t::GetLeafBrushes(int index) {
	return brush.pShared->map_leafbrushes[index];
}
cbrush_t* model_t::GetBrushes(int index) {
	return &brush.pShared->map_brushes[index];
}
cnode_t* model_t::GetNodes(int index) {
	return &brush.pShared->map_rootnode[index];
}
int model_t::GetClustersCount() {
	return brush.pShared->numclusters;
}
char* model_t::GetEntityString() {
	return brush.pShared->map_entitystring.Get();
}
void model_t::DiscardEntityString() {
	brush.pShared->map_entitystring.Discard();
}
void model_t::InitPortalOpenState() {
	for (int i = 0; i < brush.pShared->numportalopen; i++)
	{
		brush.pShared->portalopen[i] = false;
	}
}
char* model_t::GetMapName() {
	return brush.pShared->map_name;
}
int model_t::GetPlanesCount() {
	return brush.pShared->numplanes;
}
cboxbrush_t* model_t::GetBoxBrushes(int index) {
	return &brush.pShared->map_boxbrushes[index];
}
cbrushside_t* model_t::GetBrushesSide(int index) {
	return &brush.pShared->map_brushsides[index];
}
int model_t::GetCMNodesCount() {
	return brush.pShared->numcmnodes;
}
csurface_t model_t::GetNullSurface() {
	return brush.pShared->nullsurface;
}
int model_t::GetVisibilityCount() {
	return brush.pShared->numvisibility;
}
dvis_t* model_t::GetVis() {
	return brush.pShared->map_vis;
}
int model_t::GetFloodvalid() {
	return brush.pShared->floodvalid;
}
dareaportal_t* model_t::GetCMAreaPortals(int index) {
	return &brush.pShared->map_areaportals[index];
}
bool model_t::GetPortalOpenState(int index) {
	return brush.pShared->portalopen[index];
}
void model_t::SetPortalOpenState(int index, bool state) {
	brush.pShared->portalopen[index] = state;
}
carea_t* model_t::GetArea(int index) {
	return &brush.pShared->map_areas[index];
}
void model_t::IncFloodvalid() {
	brush.pShared->floodvalid++;
}
int model_t::GetAreaCount() {
	return brush.pShared->numareas;
}
int model_t::GetCMAreaPortalsCount() {
	return brush.pShared->numareaportals;
}
int model_t::GetLeafsCount() {
	return brush.pShared->numcmleafs;
}
cplane_t* model_t::GetPlane(int index) {
	return &brush.pShared->map_planes[index];
}
void model_t::Init() {
	brush.pShared->numcmleafs = 1;
	brush.pShared->map_vis = NULL;
	brush.pShared->numareas = 1;
	brush.pShared->numclusters = 1;
	brush.pShared->map_nullname = "**empty**";
	brush.pShared->numtextures = 0;
}
void model_t::Destory() {
	if (brush.pShared->map_planes.Base())
	{
		brush.pShared->map_planes.Detach();
	}

	if (brush.pShared->map_texturenames)
	{
		brush.pShared->map_texturenames = NULL;
	}

	if (brush.pShared->map_surfaces.Base())
	{
		brush.pShared->map_surfaces.Detach();
	}

	if (brush.pShared->map_areaportals.Base())
	{
		brush.pShared->map_areaportals.Detach();
	}

	if (brush.pShared->portalopen.Base())
	{
		brush.pShared->portalopen.Detach();
	}

	if (brush.pShared->map_areas.Base())
	{
		brush.pShared->map_areas.Detach();
	}

	brush.pShared->map_entitystring.Discard();

	if (brush.pShared->map_brushes.Base())
	{
		brush.pShared->map_brushes.Detach();
	}

	if (brush.pShared->map_dispList.Base())
	{
		brush.pShared->map_dispList.Detach();
	}

	if (brush.pShared->map_cmodels.Base())
	{
		brush.pShared->map_cmodels.Detach();
	}

	if (brush.pShared->map_leafbrushes.Base())
	{
		brush.pShared->map_leafbrushes.Detach();
	}

	if (brush.pShared->map_leafs.Base())
	{
		brush.pShared->map_leafs.Detach();
	}

	if (brush.pShared->map_nodes.Base())
	{
		brush.pShared->map_nodes.Detach();
	}

	if (brush.pShared->map_brushsides.Base())
	{
		brush.pShared->map_brushsides.Detach();
	}

	if (brush.pShared->map_vis)
	{
		brush.pShared->map_vis = NULL;
	}

	brush.pShared->numplanes = 0;
	brush.pShared->numbrushsides = 0;
	brush.pShared->emptyleaf = brush.pShared->solidleaf = 0;
	brush.pShared->numcmnodes = 0;
	brush.pShared->numcmleafs = 0;
	brush.pShared->numbrushes = 0;
	brush.pShared->numdisplist = 0;
	brush.pShared->numleafbrushes = 0;
	brush.pShared->numareas = 0;
	brush.pShared->numtextures = 0;
	brush.pShared->floodvalid = 0;
	brush.pShared->numareaportals = 0;
	brush.pShared->numclusters = 0;
	brush.pShared->numcmodels = 0;
	brush.pShared->numvisibility = 0;
	brush.pShared->numentitychars = 0;
	brush.pShared->numportalopen = 0;
	brush.pShared->map_name[0] = 0;
	brush.pShared->map_rootnode = NULL;

	brush.pShared->m_pDispCollTrees = NULL;
	brush.pShared->m_pDispBounds = NULL;
	brush.pShared->m_DispCollTreeCount = 0;

	if (GetDispList()->Base())
	{
		GetDispList()->Detach();
		SetDispListCount(0);
	}
}
bool model_t::Load(const char* pName,CLumpHeaderInfo& header) {
	// This is a table that maps texinfo references to csurface_t
// It is freed after the map has been loaded
	CUtlVector<unsigned short> 	map_texinfo;

	// copy map name
	Q_strncpy(brush.pShared->map_name, pName, sizeof(brush.pShared->map_name));

	//
	// load bsp file data
	//

	COM_TimestampedLog("  CollisionBSPData_LoadTextures");
	CollisionBSPData_LoadTextures(header);

	COM_TimestampedLog("  CollisionBSPData_LoadTexinfo");
	CollisionBSPData_LoadTexinfo(header,map_texinfo);

	COM_TimestampedLog("  CollisionBSPData_LoadLeafs");
	CollisionBSPData_LoadLeafs(header);

	COM_TimestampedLog("  CollisionBSPData_LoadLeafBrushes");
	CollisionBSPData_LoadLeafBrushes(header);

	COM_TimestampedLog("  CollisionBSPData_LoadPlanes");
	CollisionBSPData_LoadPlanes(header);

	COM_TimestampedLog("  CollisionBSPData_LoadBrushes");
	CollisionBSPData_LoadBrushes(header);

	COM_TimestampedLog("  CollisionBSPData_LoadBrushSides");
	CollisionBSPData_LoadBrushSides(header,map_texinfo);

	COM_TimestampedLog("  CollisionBSPData_LoadSubmodels");
	CollisionBSPData_LoadSubmodels(header);

	COM_TimestampedLog("  CollisionBSPData_LoadPlanes");
	CollisionBSPData_LoadNodes(header);

	COM_TimestampedLog("  CollisionBSPData_LoadAreas");
	CollisionBSPData_LoadAreas(header);

	COM_TimestampedLog("  CollisionBSPData_LoadAreaPortals");
	CollisionBSPData_LoadAreaPortals(header);

	COM_TimestampedLog("  CollisionBSPData_LoadVisibility");
	CollisionBSPData_LoadVisibility(header);

	COM_TimestampedLog("  CollisionBSPData_LoadEntityString");
	CollisionBSPData_LoadEntityString(header);

	COM_TimestampedLog("  CollisionBSPData_LoadPhysics");
	CollisionBSPData_LoadPhysics(header);

	COM_TimestampedLog("  CollisionBSPData_LoadDispInfo");
	CollisionBSPData_LoadDispInfo(header);

	CM_DispTreeLeafnum();

	// Close map file, etc.
	return true;
}
CRangeValidatedArray<unsigned short>* model_t::GetDispList() {
	return &brush.pShared->map_dispList;
}
void model_t::SetDispListCount(int count) {
	brush.pShared->numdisplist = count;
}
unsigned short model_t::GetDispList(int index) {
	return brush.pShared->map_dispList[index];
}
int model_t::GetEntityCharsCount() {
	return brush.pShared->numentitychars;
}
int model_t::GetTexturesCount() {
	return brush.pShared->numtextures;
}
unsigned short* model_t::GetDispListBase() {
	return brush.pShared->map_dispList.Base();
}
int model_t::GetDispListCount() {
	return brush.pShared->numdisplist;
}
int model_t::GetDispCollTreesCount() {
	return brush.pShared->m_DispCollTreeCount;
}
CDispCollTree* model_t::GetDispCollTrees(int index) {
	return &brush.pShared->m_pDispCollTrees[index];
}
alignedbbox_t* model_t::GetDispBounds(int index) {
	return &brush.pShared->m_pDispBounds[index];
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void model_t::CollisionBSPData_LoadTextures(CLumpHeaderInfo& header)
{
	CLumpInfo lh(header,LUMP_TEXDATA);

	CLumpInfo lhStringData(header,LUMP_TEXDATA_STRING_DATA);
	const char* pStringData = (const char*)lhStringData.LumpBase();

	CLumpInfo lhStringTable(header,LUMP_TEXDATA_STRING_TABLE);
	if (lhStringTable.LumpSize() % sizeof(int))
		Sys_Error("CMod_LoadTextures: funny lump size");
	int* pStringTable = (int*)lhStringTable.LumpBase();

	dtexdata_t* in;
	int			i, count;
	IMaterial* material;

	in = (dtexdata_t*)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
	{
		Sys_Error("CMod_LoadTextures: funny lump size");
	}
	count = lh.LumpSize() / sizeof(*in);
	if (count < 1)
	{
		Sys_Error("Map with no textures");
	}
	if (count > MAX_MAP_TEXDATA)
	{
		Sys_Error("Map has too many textures");
	}

	int nSize = count * sizeof(csurface_t);
	brush.pShared->map_surfaces.Attach(count, (csurface_t*)Hunk_Alloc(nSize));

	brush.pShared->numtextures = count;

	brush.pShared->map_texturenames = (char*)Hunk_Alloc(lhStringData.LumpSize() * sizeof(char), false);
	memcpy(brush.pShared->map_texturenames, pStringData, lhStringData.LumpSize());

	for (i = 0; i < count; i++, in++)
	{
		Assert(in->nameStringTableID >= 0);
		Assert(pStringTable[in->nameStringTableID] >= 0);

		const char* pInName = &pStringData[pStringTable[in->nameStringTableID]];
		int index = pInName - pStringData;

		csurface_t* out = &brush.pShared->map_surfaces[i];
		out->name = &brush.pShared->map_texturenames[index];
		out->surfaceProps = 0;
		out->flags = 0;

		material = materials->FindMaterial(brush.pShared->map_surfaces[i].name, TEXTURE_GROUP_WORLD, true);
		if (!IsErrorMaterial(material))
		{
			IMaterialVar* var;
			bool varFound;
			var = material->FindVar("$surfaceprop", &varFound, false);
			if (varFound)
			{
				const char* pProps = var->GetStringValue();
				brush.pShared->map_surfaces[i].surfaceProps = physprop->GetSurfaceIndex(pProps);
			}
		}
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void model_t::CollisionBSPData_LoadTexinfo(CLumpHeaderInfo& header,
	CUtlVector<unsigned short>& map_texinfo)
{
	CLumpInfo lh(header,LUMP_TEXINFO);

	texinfo_t* in;
	unsigned short	out;
	int			i, count;

	in = (texinfo_t*)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
		Sys_Error("CollisionBSPData_LoadTexinfo: funny lump size");
	count = lh.LumpSize() / sizeof(*in);
	if (count < 1)
		Sys_Error("Map with no texinfo");
	if (count > MAX_MAP_TEXINFO)
		Sys_Error("Map has too many surfaces");

	MEM_ALLOC_CREDIT();
	map_texinfo.RemoveAll();
	map_texinfo.EnsureCapacity(count);

	for (i = 0; i < count; i++, in++)
	{
		out = in->texdata;

		if (out >= brush.pShared->numtextures)
			out = 0;

		// HACKHACK: Copy this over for the whole material!!!
		brush.pShared->map_surfaces[out].flags |= in->flags;
		map_texinfo.AddToTail(out);
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void model_t::CollisionBSPData_LoadLeafs_Version_0(CLumpHeaderInfo& header, CLumpInfo& lh)
{
	int			i;
	dleaf_version_0_t* in;
	int			count;

	in = (dleaf_version_0_t*)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
	{
		Sys_Error("CollisionBSPData_LoadLeafs: funny lump size");
	}

	count = lh.LumpSize() / sizeof(*in);

	if (count < 1)
	{
		Sys_Error("Map with no leafs");
	}

	// need to save space for box planes
	if (count > MAX_MAP_PLANES)
	{
		Sys_Error("Map has too many planes");
	}

	// Need an extra one for the emptyleaf below
	int nSize = (count + 1) * sizeof(cleaf_t);
	brush.pShared->map_leafs.Attach(count + 1, (cleaf_t*)Hunk_Alloc(nSize));

	brush.pShared->numcmleafs = count;
	brush.pShared->numclusters = 0;

	for (i = 0; i < count; i++, in++)
	{
		cleaf_t* out = &brush.pShared->map_leafs[i];
		out->contents = in->contents;
		out->cluster = in->cluster;
		out->area = in->area;
		out->flags = in->flags;
		out->firstleafbrush = in->firstleafbrush;
		out->numleafbrushes = in->numleafbrushes;

		out->dispCount = 0;

		if (out->cluster >= brush.pShared->numclusters)
		{
			brush.pShared->numclusters = out->cluster + 1;
		}
	}

	if (brush.pShared->map_leafs[0].contents != CONTENTS_SOLID)
	{
		Sys_Error("Map leaf 0 is not CONTENTS_SOLID");
	}

	brush.pShared->solidleaf = 0;
	brush.pShared->emptyleaf = brush.pShared->numcmleafs;
	memset(&brush.pShared->map_leafs[brush.pShared->emptyleaf], 0, sizeof(brush.pShared->map_leafs[brush.pShared->emptyleaf]));
	brush.pShared->numcmleafs++;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void model_t::CollisionBSPData_LoadLeafs_Version_1(CLumpHeaderInfo& header, CLumpInfo& lh)
{
	int			i;
	dleaf_t* in;
	int			count;

	in = (dleaf_t*)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
	{
		Sys_Error("CollisionBSPData_LoadLeafs: funny lump size");
	}

	count = lh.LumpSize() / sizeof(*in);

	if (count < 1)
	{
		Sys_Error("Map with no leafs");
	}

	// need to save space for box planes
	if (count > MAX_MAP_PLANES)
	{
		Sys_Error("Map has too many planes");
	}

	// Need an extra one for the emptyleaf below
	int nSize = (count + 1) * sizeof(cleaf_t);
	brush.pShared->map_leafs.Attach(count + 1, (cleaf_t*)Hunk_Alloc(nSize));

	brush.pShared->numcmleafs = count;
	brush.pShared->numclusters = 0;

	for (i = 0; i < count; i++, in++)
	{
		cleaf_t* out = &brush.pShared->map_leafs[i];
		out->contents = in->contents;
		out->cluster = in->cluster;
		out->area = in->area;
		out->flags = in->flags;
		out->firstleafbrush = in->firstleafbrush;
		out->numleafbrushes = in->numleafbrushes;

		out->dispCount = 0;

		if (out->cluster >= brush.pShared->numclusters)
		{
			brush.pShared->numclusters = out->cluster + 1;
		}
	}

	if (brush.pShared->map_leafs[0].contents != CONTENTS_SOLID)
	{
		Sys_Error("Map leaf 0 is not CONTENTS_SOLID");
	}

	brush.pShared->solidleaf = 0;
	brush.pShared->emptyleaf = brush.pShared->numcmleafs;
	memset(&brush.pShared->map_leafs[brush.pShared->emptyleaf], 0, sizeof(brush.pShared->map_leafs[brush.pShared->emptyleaf]));
	brush.pShared->numcmleafs++;
}

void model_t::CollisionBSPData_LoadLeafs(CLumpHeaderInfo& header)
{
	CLumpInfo lh(header,LUMP_LEAFS);
	switch (lh.LumpVersion())
	{
	case 0:
		CollisionBSPData_LoadLeafs_Version_0(header,lh);
		break;
	case 1:
		CollisionBSPData_LoadLeafs_Version_1(header,lh);
		break;
	default:
		Assert(0);
		Error("Unknown LUMP_LEAFS version\n");
		break;
	}

}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void model_t::CollisionBSPData_LoadLeafBrushes(CLumpHeaderInfo& header)
{
	CLumpInfo lh(header,LUMP_LEAFBRUSHES);

	int			i;
	unsigned short* in;
	int			count;

	in = (unsigned short*)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
	{
		Sys_Error("CMod_LoadLeafBrushes: funny lump size");
	}

	count = lh.LumpSize() / sizeof(*in);
	if (count < 1)
	{
		Sys_Error("Map with no planes");
	}

	// need to save space for box planes
	if (count > MAX_MAP_LEAFBRUSHES)
	{
		Sys_Error("Map has too many leafbrushes");
	}

	brush.pShared->map_leafbrushes.Attach(count, (unsigned short*)Hunk_Alloc(count * sizeof(unsigned short), false));
	brush.pShared->numleafbrushes = count;

	for (i = 0; i < count; i++, in++)
	{
		brush.pShared->map_leafbrushes[i] = *in;
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void model_t::CollisionBSPData_LoadPlanes(CLumpHeaderInfo& header)
{
	CLumpInfo lh(header,LUMP_PLANES);

	int			i, j;
	dplane_t* in;
	int			count;
	int			bits;

	in = (dplane_t*)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
	{
		Sys_Error("CollisionBSPData_LoadPlanes: funny lump size");
	}

	count = lh.LumpSize() / sizeof(*in);

	if (count < 1)
	{
		Sys_Error("Map with no planes");
	}

	// need to save space for box planes
	if (count > MAX_MAP_PLANES)
	{
		Sys_Error("Map has too many planes");
	}

	int nSize = count * sizeof(cplane_t);
	brush.pShared->map_planes.Attach(count, (cplane_t*)Hunk_Alloc(nSize));

	brush.pShared->numplanes = count;

	for (i = 0; i < count; i++, in++)
	{
		cplane_t* out = &brush.pShared->map_planes[i];
		bits = 0;
		for (j = 0; j < 3; j++)
		{
			out->normal[j] = in->normal[j];
			if (out->normal[j] < 0)
			{
				bits |= 1 << j;
			}
		}

		out->dist = in->dist;
		out->type = in->type;
		out->signbits = bits;
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void model_t::CollisionBSPData_LoadBrushes(CLumpHeaderInfo& header)
{
	CLumpInfo lh(header,LUMP_BRUSHES);

	dbrush_t* in;
	int			i, count;

	in = (dbrush_t*)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
	{
		Sys_Error("CMod_LoadBrushes: funny lump size");
	}

	count = lh.LumpSize() / sizeof(*in);
	if (count > MAX_MAP_BRUSHES)
	{
		Sys_Error("Map has too many brushes");
	}

	int nSize = count * sizeof(cbrush_t);
	brush.pShared->map_brushes.Attach(count, (cbrush_t*)Hunk_Alloc(nSize));

	brush.pShared->numbrushes = count;

	for (i = 0; i < count; i++, in++)
	{
		cbrush_t* out = &brush.pShared->map_brushes[i];
		out->firstbrushside = in->firstside;
		out->numsides = in->numsides;
		out->contents = in->contents;
	}
}

inline bool model_t::IsBoxBrush(const cbrush_t& brush, dbrushside_t* pSides, cplane_t* pPlanes)
{
	int countAxial = 0;
	if (brush.numsides == 6)
	{
		for (int i = 0; i < brush.numsides; i++)
		{
			cplane_t* plane = pPlanes + pSides[brush.firstbrushside + i].planenum;
			if (plane->type > PLANE_Z)
				break;
			countAxial++;
		}
	}
	return (countAxial == brush.numsides) ? true : false;
}

inline void model_t::ExtractBoxBrush(cboxbrush_t* pBox, const cbrush_t& brush, dbrushside_t* pSides, cplane_t* pPlanes, CUtlVector<unsigned short>& map_texinfo)
{
	// brush.numsides is no longer valid.  Assume numsides == 6
	for (int i = 0; i < 6; i++)
	{
		dbrushside_t* side = pSides + i + brush.firstbrushside;
		cplane_t* plane = pPlanes + side->planenum;
		int t = side->texinfo;
		Assert(t < map_texinfo.Count());
		int surfaceIndex = (t < 0) ? SURFACE_INDEX_INVALID : map_texinfo[t];
		int axis = plane->type;
		Assert(fabs(plane->normal[axis]) == 1.0f);
		if (plane->normal[axis] == 1.0f)
		{
			pBox->maxs[axis] = plane->dist;
			pBox->surfaceIndex[axis + 3] = surfaceIndex;
		}
		else if (plane->normal[axis] == -1.0f)
		{
			pBox->mins[axis] = -plane->dist;
			pBox->surfaceIndex[axis] = surfaceIndex;
		}
		else
		{
			Assert(0);
		}
	}
	pBox->pad2[0] = 0;
	pBox->pad2[1] = 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void model_t::CollisionBSPData_LoadBrushSides(CLumpHeaderInfo& header,CUtlVector<unsigned short>& map_texinfo)
{
	CLumpInfo lh(header,LUMP_BRUSHSIDES);

	int				i, j;
	dbrushside_t* in;

	in = (dbrushside_t*)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
	{
		Sys_Error("CMod_LoadBrushSides: funny lump size");
	}

	int inputSideCount = lh.LumpSize() / sizeof(*in);

	// need to save space for box planes
	if (inputSideCount > MAX_MAP_BRUSHSIDES)
	{
		Sys_Error("Map has too many planes");
	}


	// Brushes are compressed on load to remove any AABB brushes.  The brushsides for those are removed
	// and those brushes are stored as cboxbrush_t.  But the texinfo/surface data needs to be copied
	// So the algorithm is:
	//
	// count box brushes
	// count total brush sides
	// allocate
	// iterate brushes and copy sides or fill out box brushes
	// done
	//

	int boxBrushCount = 0;
	int brushSideCount = 0;
	for (i = 0; i < brush.pShared->numbrushes; i++)
	{
		if (IsBoxBrush(brush.pShared->map_brushes[i], in, brush.pShared->map_planes.Base()))
		{
			// mark as axial
			brush.pShared->map_brushes[i].numsides = NUMSIDES_BOXBRUSH;
			boxBrushCount++;
		}
		else
		{
			brushSideCount += brush.pShared->map_brushes[i].numsides;
		}
	}

	int nSize = brushSideCount * sizeof(cbrushside_t);
	brush.pShared->map_brushsides.Attach(brushSideCount, (cbrushside_t*)Hunk_Alloc(nSize, false));
	brush.pShared->map_boxbrushes.Attach(boxBrushCount, (cboxbrush_t*)Hunk_Alloc(boxBrushCount * sizeof(cboxbrush_t), false));

	brush.pShared->numbrushsides = brushSideCount;
	brush.pShared->numboxbrushes = boxBrushCount;

	int outBoxBrush = 0;
	int outBrushSide = 0;
	for (i = 0; i < brush.pShared->numbrushes; i++)
	{
		cbrush_t* pBrush = &brush.pShared->map_brushes[i];

		if (pBrush->IsBox())
		{
			// fill out the box brush - extract from the input sides
			cboxbrush_t* pBox = &brush.pShared->map_boxbrushes[outBoxBrush];
			ExtractBoxBrush(pBox, *pBrush, in, brush.pShared->map_planes.Base(), map_texinfo);
			pBrush->SetBox(outBoxBrush);
			outBoxBrush++;
		}
		else
		{
			// copy each side into the output array
			int firstInputSide = pBrush->firstbrushside;
			pBrush->firstbrushside = outBrushSide;
			for (j = 0; j < pBrush->numsides; j++)
			{
				cbrushside_t* RESTRICT pSide = &brush.pShared->map_brushsides[outBrushSide];
				dbrushside_t* RESTRICT pInputSide = in + firstInputSide + j;
				pSide->plane = &brush.pShared->map_planes[pInputSide->planenum];
				int t = pInputSide->texinfo;
				if (t >= map_texinfo.Size())
				{
					Sys_Error("Bad brushside texinfo");
				}

				// BUGBUG: Why is vbsp writing out -1 as the texinfo id?  (TEXINFO_NODE ?)
				pSide->surfaceIndex = (t < 0) ? SURFACE_INDEX_INVALID : map_texinfo[t];
				pSide->bBevel = pInputSide->bevel ? true : false;
				outBrushSide++;
			}
		}
	}
	Assert(outBrushSide == numbrushsides && outBoxBrush == numboxbrushes);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void model_t::CollisionBSPData_LoadSubmodels(CLumpHeaderInfo& header)
{
	CLumpInfo lh(header,LUMP_MODELS);

	dmodel_t* in;
	int			i, j, count;

	in = (dmodel_t*)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
		Sys_Error("CMod_LoadSubmodels: funny lump size");
	count = lh.LumpSize() / sizeof(*in);

	if (count < 1)
		Sys_Error("Map with no models");
	if (count > MAX_MAP_MODELS)
		Sys_Error("Map has too many models");

	int nSize = count * sizeof(cmodel_t);
	brush.pShared->map_cmodels.Attach(count, (cmodel_t*)Hunk_Alloc(nSize));
	brush.pShared->numcmodels = count;

	for (i = 0; i < count; i++, in++)
	{
		cmodel_t* out = &brush.pShared->map_cmodels[i];

		for (j = 0; j < 3; j++)
		{	// spread the mins / maxs by a pixel
			out->mins[j] = in->mins[j] - 1;
			out->maxs[j] = in->maxs[j] + 1;
			out->origin[j] = in->origin[j];
		}
		out->headnode = in->headnode;
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void model_t::CollisionBSPData_LoadNodes(CLumpHeaderInfo& header)
{
	CLumpInfo lh(header,LUMP_NODES);

	dnode_t* in;
	int			i, j, count;

	in = (dnode_t*)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
		Sys_Error("CollisionBSPData_LoadNodes: funny lump size");
	count = lh.LumpSize() / sizeof(*in);

	if (count < 1)
		Sys_Error("Map has no nodes");
	if (count > MAX_MAP_NODES)
		Sys_Error("Map has too many nodes");

	// 6 extra for box hull
	int nSize = (count + 6) * sizeof(cnode_t);
	brush.pShared->map_nodes.Attach(count + 6, (cnode_t*)Hunk_Alloc(nSize));

	brush.pShared->numcmnodes = count;
	brush.pShared->map_rootnode = brush.pShared->map_nodes.Base();

	for (i = 0; i < count; i++, in++)
	{
		cnode_t* out = &brush.pShared->map_nodes[i];
		out->plane = &brush.pShared->map_planes[in->planenum];
		for (j = 0; j < 2; j++)
		{
			out->children[j] = in->children[j];
		}
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void model_t::CollisionBSPData_LoadAreas(CLumpHeaderInfo& header)
{
	CLumpInfo lh(header,LUMP_AREAS);

	int			i;
	darea_t* in;
	int			count;

	in = (darea_t*)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
	{
		Sys_Error("CMod_LoadAreas: funny lump size");
	}

	count = lh.LumpSize() / sizeof(*in);
	if (count > MAX_MAP_AREAS)
	{
		Sys_Error("Map has too many areas");
	}

	int nSize = count * sizeof(carea_t);
	brush.pShared->map_areas.Attach(count, (carea_t*)Hunk_Alloc(nSize));

	brush.pShared->numareas = count;

	for (i = 0; i < count; i++, in++)
	{
		carea_t* out = &brush.pShared->map_areas[i];
		out->numareaportals = in->numareaportals;
		out->firstareaportal = in->firstareaportal;
		out->floodvalid = 0;
		out->floodnum = 0;
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void model_t::CollisionBSPData_LoadAreaPortals(CLumpHeaderInfo& header)
{
	CLumpInfo lh(header,LUMP_AREAPORTALS);

	dareaportal_t* in;
	int				count;

	in = (dareaportal_t*)lh.LumpBase();
	if (lh.LumpSize() % sizeof(*in))
	{
		Sys_Error("CMod_LoadAreaPortals: funny lump size");
	}

	count = lh.LumpSize() / sizeof(*in);
	if (count > MAX_MAP_AREAPORTALS)
	{
		Sys_Error("Map has too many area portals");
	}

	// Need to add one more in owing to 1-based instead of 0-based data!
	++count;

	brush.pShared->numportalopen = count;
	brush.pShared->portalopen.Attach(count, (bool*)Hunk_Alloc(brush.pShared->numportalopen * sizeof(bool), false));
	for (int i = 0; i < brush.pShared->numportalopen; i++)
	{
		brush.pShared->portalopen[i] = false;
	}

	brush.pShared->numareaportals = count;
	int nSize = count * sizeof(dareaportal_t);
	brush.pShared->map_areaportals.Attach(count, (dareaportal_t*)Hunk_Alloc(nSize));

	Assert(nSize >= lh.LumpSize());
	memcpy(brush.pShared->map_areaportals.Base(), in, lh.LumpSize());
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void model_t::CollisionBSPData_LoadVisibility(CLumpHeaderInfo& header)
{
	CLumpInfo lh(header,LUMP_VISIBILITY);

	brush.pShared->numvisibility = lh.LumpSize();
	if (lh.LumpSize() > MAX_MAP_VISIBILITY)
		Sys_Error("Map has too large visibility lump");

	int visDataSize = lh.LumpSize();
	if (visDataSize == 0)
	{
		brush.pShared->map_vis = NULL;
	}
	else
	{
		brush.pShared->map_vis = (dvis_t*)Hunk_Alloc(visDataSize, false);
		memcpy(brush.pShared->map_vis, lh.LumpBase(), visDataSize);
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void model_t::CollisionBSPData_LoadEntityString(CLumpHeaderInfo& header)
{
	CLumpInfo lh(header,LUMP_ENTITIES);

	brush.pShared->numentitychars = lh.LumpSize();
	MEM_ALLOC_CREDIT();
	char szMapName[MAX_PATH] = { 0 };
	V_strncpy(szMapName, header.GetMapName(), sizeof(szMapName));
	brush.pShared->map_entitystring.Init(szMapName, lh.LumpOffset(), lh.LumpSize(), lh.LumpBase());
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void model_t::CollisionBSPData_LoadPhysics(CLumpHeaderInfo& header)
{
#ifdef _WIN32
	CLumpInfo lh(header,LUMP_PHYSCOLLIDE);
#else
	int nLoadLump = LUMP_PHYSCOLLIDE;
	// backwards compat support for older game dlls
	// they crash if they don't have vcollide data for terrain 
	// even though the new engine ignores it
	if (g_iServerGameDLLVersion >= 5)
	{
		// if there's a linux lump present, go ahead and load it
		// otherwise, the win32 lump will work as long as it doesn't have any
		// mopp surfaces in it (if compiled with the current vbsp.exe or a map without any displacements).  
		// The legacy server game DLLs will crash when mopps are present but since 
		// they also crash with a NULL lump there's nothing lost in that case.
		if (CMapLoadHelper::LumpSize(LUMP_PHYSCOLLIDESURFACE) > 0)
		{
			nLoadLump = LUMP_PHYSCOLLIDESURFACE;
		}
		else
		{
			DevWarning("Legacy game DLL may not support terrain vphysics collisions with this BSP!\n");
		}
	}

	CMapLoadHelper lh(nLoadLump);
#endif
	if (!lh.LumpSize())
		return;

	byte* ptr = lh.LumpBase();
	byte* basePtr = ptr;

	dphysmodel_t physModel;

	// physics data is variable length.  The last physmodel is a NULL pointer
	// with modelIndex -1, dataSize -1
	do
	{
		memcpy(&physModel, ptr, sizeof(physModel));
		ptr += sizeof(physModel);

		if (physModel.dataSize > 0)
		{
			cmodel_t* pModel = &brush.pShared->map_cmodels[physModel.modelIndex];
			physcollision->VCollideLoad(&pModel->vcollisionData, physModel.solidCount, (const char*)ptr, physModel.dataSize + physModel.keydataSize);
			ptr += physModel.dataSize;
			ptr += physModel.keydataSize;
		}

		// avoid infinite loop on badly formed file
		if ((int)(ptr - basePtr) > lh.LumpSize())
			break;

	} while (physModel.dataSize > 0);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void model_t::CollisionBSPData_LoadDispInfo(CLumpHeaderInfo& header)
{
	// How many displacements in the map?
	int coreDispCount = header.LumpSize(LUMP_DISPINFO) / sizeof(ddispinfo_t);
	if (coreDispCount == 0)
		return;

	//
	// get the vertex data
	//
	CLumpInfo lhv(header,LUMP_VERTEXES);
	dvertex_t* pVerts = (dvertex_t*)lhv.LumpBase();
	if (lhv.LumpSize() % sizeof(dvertex_t))
		Sys_Error("CMod_LoadDispInfo: bad vertex lump size!");

	//
	// get the edge data
	//
	CLumpInfo lhe(header,LUMP_EDGES);
	dedge_t* pEdges = (dedge_t*)lhe.LumpBase();
	if (lhe.LumpSize() % sizeof(dedge_t))
		Sys_Error("CMod_LoadDispInfo: bad edge lump size!");

	//
	// get surf edges data
	//
	CLumpInfo lhs(header,LUMP_SURFEDGES);
	int* pSurfEdges = (int*)lhs.LumpBase();
	if (lhs.LumpSize() % sizeof(int))
		Sys_Error("CMod_LoadDispInfo: bad surf edge lump size!");

	//
	// get face data
	//
	int face_lump_to_load = LUMP_FACES;
	if (g_pMaterialSystemHardwareConfig->GetHDREnabled() && /*g_pMaterialSystemHardwareConfig->GetHDRType() != HDR_TYPE_NONE &&*/
		header.LumpSize(LUMP_FACES_HDR) > 0)
	{
		face_lump_to_load = LUMP_FACES_HDR;
	}
	CLumpInfo lhf(header,face_lump_to_load);
	dface_t* pFaces = (dface_t*)lhf.LumpBase();
	if (lhf.LumpSize() % sizeof(dface_t))
		Sys_Error("CMod_LoadDispInfo: bad face lump size!");
	int faceCount = lhf.LumpSize() / sizeof(dface_t);

	dface_t* pFaceList = pFaces;
	if (!pFaceList)
		return;

	//
	// get texinfo data
	//
	CLumpInfo lhti(header,LUMP_TEXINFO);
	texinfo_t* pTexinfoList = (texinfo_t*)lhti.LumpBase();
	if (lhti.LumpSize() % sizeof(texinfo_t))
		Sys_Error("CMod_LoadDispInfo: bad texinfo lump size!");

	// allocate displacement collision trees
	brush.pShared->m_DispCollTreeCount = coreDispCount;
	brush.pShared->m_pDispCollTrees = DispCollTrees_Alloc(brush.pShared->m_DispCollTreeCount);
	brush.pShared->m_pDispBounds = (alignedbbox_t*)Hunk_Alloc(brush.pShared->m_DispCollTreeCount * sizeof(alignedbbox_t), false);

	// Build the inverse mapping from disp index to face
	int nMemSize = coreDispCount * sizeof(unsigned short);
	unsigned short* pDispIndexToFaceIndex = (unsigned short*)stackalloc(nMemSize);
	memset(pDispIndexToFaceIndex, 0xFF, nMemSize);

	int i;
	for (i = 0; i < faceCount; ++i, ++pFaces)
	{
		// check face for displacement data
		if (pFaces->dispinfo == -1)
			continue;

		// get the current displacement build surface
		if (pFaces->dispinfo >= coreDispCount)
			continue;

		pDispIndexToFaceIndex[pFaces->dispinfo] = (unsigned short)i;
	}

	// Load one dispinfo from disk at a time and set it up.
	int iCurVert = 0;
	int iCurTri = 0;
	CDispVert tempVerts[MAX_DISPVERTS];
	CDispTri  tempTris[MAX_DISPTRIS];

	int nSize = 0;
	int nCacheSize = 0;
	int nPowerCount[3] = { 0, 0, 0 };

	CLumpInfo lhDispInfo(header,LUMP_DISPINFO);
	CLumpInfo lhDispVerts(header,LUMP_DISP_VERTS);
	CLumpInfo lhDispTris(header,LUMP_DISP_TRIS);

	for (i = 0; i < coreDispCount; ++i)
	{
		// Find the face associated with this dispinfo
		unsigned short nFaceIndex = pDispIndexToFaceIndex[i];
		if (nFaceIndex == 0xFFFF)
			continue;

		// Load up the dispinfo and create the CCoreDispInfo from it.
		ddispinfo_t dispInfo;
		lhDispInfo.LoadLumpElement(i, sizeof(ddispinfo_t), &dispInfo);

		// Read in the vertices.
		int nVerts = NUM_DISP_POWER_VERTS(dispInfo.power);
		lhDispVerts.LoadLumpData(iCurVert * sizeof(CDispVert), nVerts * sizeof(CDispVert), tempVerts);
		iCurVert += nVerts;

		// Read in the tris.
		int nTris = NUM_DISP_POWER_TRIS(dispInfo.power);
		lhDispTris.LoadLumpData(iCurTri * sizeof(CDispTri), nTris * sizeof(CDispTri), tempTris);
		iCurTri += nTris;

		CCoreDispInfo coreDisp;
		CCoreDispSurface* pDispSurf = coreDisp.GetSurface();
		pDispSurf->SetPointStart(dispInfo.startPosition);
		pDispSurf->SetContents(dispInfo.contents);

		coreDisp.InitDispInfo(dispInfo.power, dispInfo.minTess, dispInfo.smoothingAngle, tempVerts, tempTris);

		// Hook the disp surface to the face
		pFaces = &pFaceList[nFaceIndex];
		pDispSurf->SetHandle(nFaceIndex);

		// get points
		if (pFaces->numedges > 4)
			continue;

		Vector surfPoints[4];
		pDispSurf->SetPointCount(pFaces->numedges);
		int j;
		for (j = 0; j < pFaces->numedges; j++)
		{
			int eIndex = pSurfEdges[pFaces->firstedge + j];
			if (eIndex < 0)
			{
				VectorCopy(pVerts[pEdges[-eIndex].v[1]].point, surfPoints[j]);
			}
			else
			{
				VectorCopy(pVerts[pEdges[eIndex].v[0]].point, surfPoints[j]);
			}
		}

		for (j = 0; j < 4; j++)
		{
			pDispSurf->SetPoint(j, surfPoints[j]);
		}

		pDispSurf->FindSurfPointStartIndex();
		pDispSurf->AdjustSurfPointData();

		//
		// generate the collision displacement surfaces
		//
		CDispCollTree* pDispTree = &brush.pShared->m_pDispCollTrees[i];
		pDispTree->SetPower(0);

		//
		// check for null faces, should have been taken care of in vbsp!!!
		//
		int pointCount = pDispSurf->GetPointCount();
		if (pointCount != 4)
			continue;

		coreDisp.Create();

		// new collision
		pDispTree->Create(&coreDisp);
		brush.pShared->m_pDispBounds[i].Init(pDispTree->m_mins, pDispTree->m_maxs, pDispTree->m_iCounter, pDispTree->GetContents());
		nSize += pDispTree->GetMemorySize();
		nCacheSize += pDispTree->GetCacheMemorySize();
		nPowerCount[pDispTree->GetPower() - 2]++;

		// Surface props.
		texinfo_t* pTex = &pTexinfoList[pFaces->texinfo];
		if (pTex->texdata >= 0)
		{
			IMaterial* pMaterial = materials->FindMaterial(brush.pShared->map_surfaces[pTex->texdata].name, TEXTURE_GROUP_WORLD, true);
			if (!IsErrorMaterial(pMaterial))
			{
				IMaterialVar* pVar;
				bool bVarFound;
				pVar = pMaterial->FindVar("$surfaceprop", &bVarFound, false);
				if (bVarFound)
				{
					const char* pProps = pVar->GetStringValue();
					pDispTree->SetSurfaceProps(0, physprop->GetSurfaceIndex(pProps));
					pDispTree->SetSurfaceProps(1, physprop->GetSurfaceIndex(pProps));
				}

				pVar = pMaterial->FindVar("$surfaceprop2", &bVarFound, false);
				if (bVarFound)
				{
					const char* pProps = pVar->GetStringValue();
					pDispTree->SetSurfaceProps(1, physprop->GetSurfaceIndex(pProps));
				}
			}
		}
	}

	CLumpInfo lhDispPhys(header,LUMP_PHYSDISP);
	dphysdisp_t* pDispPhys = (dphysdisp_t*)lhDispPhys.LumpBase();
	// create the vphysics collision models for each displacement
	CM_CreateDispPhysCollide(this, pDispPhys, lhDispPhys.LumpSize());
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void model_t::CM_DispTreeLeafnum()
{
	// check to see if there are any displacement trees to push down the bsp tree??
	if (GetDispCollTreesCount() == 0)
		return;

	for (int i = 0; i < GetLeafsCount(); i++)
	{
		GetLeafs(i)->dispCount = 0;
	}
	//
	// get the number of displacements per leaf
	//
	CDispLeafBuilder leafBuilder(this);

	for (int i = 0; i < GetDispCollTreesCount(); i++)
	{
		leafBuilder.BuildLeafListForDisplacement(i);
	}
	int count = leafBuilder.GetDispListCount();
	GetDispList()->Attach(count, (unsigned short*)Hunk_Alloc(sizeof(unsigned short) * count, false));
	leafBuilder.WriteLeafList(GetDispList()->Base());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool model_t::CollisionBSPData_Init()
{
	this->Init();

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//void DispCollTrees_FreeLeafList(CCollisionBSPData* pBSPData)
//{
	
//}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void model_t::CollisionBSPData_Destroy()
{
	for ( int i = 0; i < this->GetCModelsCount(); i++)
	{
		physcollision->VCollideUnload( &this->GetCModels(i)->vcollisionData);
	}

	// free displacement data
	//DispCollTrees_FreeLeafList( pBSPData );
	CM_DestroyDispPhysCollide(this);
	DispCollTrees_Free(this->GetDispCollTrees(0));
	

	this->Destory();
}

//-----------------------------------------------------------------------------
// Returns the collision tree associated with the ith displacement
//-----------------------------------------------------------------------------

CDispCollTree* CollisionBSPData_GetCollisionTree( int i )
{
	if ((i < 0) || (i >= g_pHost->Host_GetWorldModel()->GetDispCollTreesCount()))
		return 0;

	return g_pHost->Host_GetWorldModel()->GetDispCollTrees(i);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CollisionBSPData_LinkPhysics( void )
{
	//
	// initialize the physics surface properties -- if necessary!
	//
	if( !physprop )
	{
		physprop = ( IPhysicsSurfaceProps* )g_AppSystemFactory( VPHYSICS_SURFACEPROPS_INTERFACE_VERSION, NULL );
		physcollision = ( IPhysicsCollision* )g_AppSystemFactory( VPHYSICS_COLLISION_INTERFACE_VERSION, NULL );

		if ( !physprop || !physcollision )
		{
			Sys_Error( "CollisionBSPData_PreLoad: Can't link physics" );
		}
	}
}


//=============================================================================
//
// Loading Functions
//

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void model_t::CollisionBSPData_PreLoad()
{
	// initialize the collision bsp data
	CollisionBSPData_Init(); 
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool model_t::CollisionBSPData_Load( const char *pName, CLumpHeaderInfo& header )
{
	

	return this->Load(pName,header);
}





//=============================================================================
//
// Collision Count Functions
//

#ifdef COUNT_COLLISIONS
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CollisionCounts_Init( CCollisionCounts *pCounts )
{
	pCounts->m_PointContents = 0;
	pCounts->m_Traces = 0;
	pCounts->m_BrushTraces = 0;
	pCounts->m_DispTraces = 0;
	pCounts->m_Stabs = 0;
}
#endif
