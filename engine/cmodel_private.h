//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CMODEL_PRIVATE_H
#define CMODEL_PRIVATE_H
#pragma once

#include "qlimits.h"
#include "bitvec.h"
#include "bspfile.h"
#include "utlbuffer.h"
#include "modelloader.h"

#include "filesystem.h"
#include "filesystem_engine.h"

#include "dispcoll_common.h"

class CDispCollTree;
class CCollisionBSPData;
struct cbrush_t;

#define MAX_CHECK_COUNT_DEPTH 2

struct TraceVisits_t
{
	CVarBitVec m_Brushes;
	CVarBitVec m_Disps;
};

typedef uint32 TraceCounter_t;
typedef CUtlVector<TraceCounter_t> CTraceCounterVec;

struct TraceInfo_t
{
	TraceInfo_t()
	{
		memset( this, 0, offsetof( TraceInfo_t, m_BrushCounters ) );
		m_nCheckDepth = -1;
	}

	Vector m_start;
	Vector m_end;
	Vector m_mins;
	Vector m_maxs;
	Vector m_extents;
	Vector m_delta;
	Vector m_invDelta;

	trace_t m_trace;
	trace_t m_stabTrace;

	int m_contents;
	bool m_ispoint;
	bool m_isswept;

	// BSP Data
	CCollisionBSPData *m_pBSPData;

	// Displacement Data
	Vector m_DispStabDir;		// the direction to stab in
	int m_bDispHit;				// hit displacement surface last

	bool m_bCheckPrimary;
	int m_nCheckDepth;
	TraceCounter_t m_Count[MAX_CHECK_COUNT_DEPTH];

	CTraceCounterVec m_BrushCounters[MAX_CHECK_COUNT_DEPTH];
	CTraceCounterVec m_DispCounters[MAX_CHECK_COUNT_DEPTH];

	TraceCounter_t GetCount()				{ return m_Count[m_nCheckDepth]; }
	TraceCounter_t *GetBrushCounters()		{ return m_BrushCounters[m_nCheckDepth].Base(); }
	TraceCounter_t *GetDispCounters()		{ return m_DispCounters[m_nCheckDepth].Base(); }

	bool Visit( cbrush_t *pBrush, int ndxBrush );
	bool Visit( int dispIndex );

	bool Visit( cbrush_t *pBrush, int ndxBrush, TraceCounter_t cachedCount, TraceCounter_t *pCachedCounters );
	bool Visit( int dispIndex, TraceCounter_t cachedCount, TraceCounter_t *pCachedCounters );
};

extern TraceInfo_t g_PrimaryTraceInfo;

#define		NEVER_UPDATED		-99999

//=============================================================================
//
// Local CModel Structures (cmodel.cpp and cmodel_disp.cpp)
//

struct cbrushside_t
{
	cplane_t	*plane;
	unsigned short surfaceIndex;
	unsigned short bBevel;							// is the side a bevel plane?
};

#define NUMSIDES_BOXBRUSH	0xFFFF

struct cbrush_t
{
	int				contents;
	unsigned short	numsides;
	unsigned short	firstbrushside;

	inline int GetBox() const { return firstbrushside; }
	inline void SetBox( int boxID )
	{
		numsides = NUMSIDES_BOXBRUSH;
		firstbrushside = boxID;
	}
	inline bool IsBox() const { return numsides == NUMSIDES_BOXBRUSH ? true : false; }
};

// 48-bytes, aligned to 16-byte boundary
// this is a brush that is an AABB.  It's encoded this way instead of with 6 brushsides
struct cboxbrush_t
{
	VectorAligned	mins;
	VectorAligned	maxs;

	unsigned short	surfaceIndex[6];
	unsigned short	pad2[2];
};

struct cleaf_t
{
	int			    contents;
	short			cluster;
	short			area : 9;
	short			flags : 7;
	unsigned short	firstleafbrush;
	unsigned short	numleafbrushes;
	unsigned short	dispListStart;
	unsigned short	dispCount;
};

struct carea_t
{
	int		numareaportals;
	int		firstareaportal;
	int		floodnum;							// if two areas have equal floodnums, they are connected
	int		floodvalid;
};


struct cnode_t
{
	cplane_t	*plane;
	int			children[2];		// negative numbers are leafs
};


// global collision checkcount
TraceInfo_t *BeginTrace();
void PushTraceVisits( TraceInfo_t *pTraceInfo );
void PopTraceVisits( TraceInfo_t *pTraceInfo );
void EndTrace( TraceInfo_t *&pTraceInfo );


//-----------------------------------------------------------------------------
// We keep running into overflow errors here. This is to avoid that problem
//-----------------------------------------------------------------------------
template <class T>
class CRangeValidatedArray
{
public:
	void Attach( int nCount, T* pData );
	void Detach();

	T &operator[]( int i );
	const T &operator[]( int i ) const;

	T *Base();

private:
	T *m_pArray;

#ifdef DBGFLAG_ASSERT
	int m_nCount;
#endif
};

template <class T>
inline T &CRangeValidatedArray<T>::operator[]( int i )
{
	Assert( (i >= 0) && (i < m_nCount) );
	return m_pArray[i];
}

template <class T>
inline const T &CRangeValidatedArray<T>::operator[]( int i ) const
{
	Assert( (i >= 0) && (i < m_nCount) );
	return m_pArray[i];
}

template <class T>
inline void CRangeValidatedArray<T>::Attach( int nCount, T* pData )
{
	m_pArray = pData;

#ifdef DBGFLAG_ASSERT
	m_nCount = nCount;
#endif
}

template <class T>
inline void CRangeValidatedArray<T>::Detach()
{
	m_pArray = NULL;

#ifdef DBGFLAG_ASSERT
	m_nCount = 0;
#endif
}

template <class T>
inline T *CRangeValidatedArray<T>::Base()
{
	return m_pArray;
}

//-----------------------------------------------------------------------------
// A dumpable version of RangeValidatedArray
//-----------------------------------------------------------------------------
#include "tier0/memdbgon.h"
template <class T> 
class CDiscardableArray 
{
public:
	CDiscardableArray()
	{
		m_nCount = 0;
		m_nOffset = -1;
		memset( m_pFilename, 0, sizeof( m_pFilename ) );
	}

	~CDiscardableArray()
	{
	}

	void Init( char* pFilename, int nOffset, int nCount, void *pData = NULL )
	{
		if ( m_buf.TellPut() )
		{
			m_buf.Purge();
		}

		m_nCount = nCount;
		V_strcpy_safe( m_pFilename, pFilename );
		m_nOffset = nOffset;

		// can preload as required
		if ( pData )
		{
			m_buf.Put( pData, nCount );
		}
	}

	// Either get or load the array as needed:
	T* Get()
	{
		if ( !m_buf.TellPut() )
		{
			MEM_ALLOC_CREDIT();

			if ( !g_pFileSystem->ReadFile( m_pFilename, NULL, m_buf, sizeof(T) * m_nCount, m_nOffset ) )
			{
				return NULL;
			}
		}

		return (T *)m_buf.PeekGet();
	}

	void Discard()
	{
		// TODO(johns): Neutered -- Tear this class out. This can no longer be discarded with transparently compressed
		//              BSPs. This is on the range of 500k of memory, and is probably overly complex for the savings in
		//              the current era.
		DevMsg("TODO: Refusing to discard %u bytes\n", sizeof(T) * m_nCount);
		// m_buf.Purge();
	}

protected:
	int			m_nCount;
	CUtlBuffer	m_buf;
	char		m_pFilename[256];
	int			m_nOffset;
};
#include "tier0/memdbgoff.h"

const int SURFACE_INDEX_INVALID = 0xFFFF;

//=============================================================================
//
// Displacement Collision Functions and Data
//
// UNDONE: Find a way to overlap the counter/contents bits with mins.w/maxs.w without hosing performance
struct alignedbbox_t
{
	VectorAligned	mins;
	VectorAligned	maxs;
	int				dispCounter;
	int				dispContents;
	int				pad[2];
	void SetCounter(int counter)
	{
		dispCounter = counter;
	}
	int GetCounter()
	{
		return dispCounter;
	}
	void SetContents(int contents)
	{
		dispContents = contents;
	}
	int GetContents()
	{
		return dispContents;
	}
	void Init(const Vector& minsIn, const Vector& maxsIn, int counterIn, int contentsIn)
	{
		mins = minsIn;
		SetCounter(counterIn);
		maxs = maxsIn;
		SetContents(contentsIn);
	}
};

//=============================================================================
//
// Collision BSP Data Class
//
class CCollisionBSPData
{
public:
	void Init();
	void Destory();
	bool Load(const char* pName);
	char* GetMapName();
	int GetNodesCount();
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
	int GetAreaPortalsCount();
	dareaportal_t* GetAreaPortals(int index);
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
private:
	// This is sort of a hack, but it was a little too painful to do this any other way
	// The goal of this dude is to allow us to override the tree with some
	// other tree (or a subtree)
	cnode_t*					map_rootnode;

	char						map_name[MAX_QPATH];
	static csurface_t			nullsurface;

	int									numbrushsides;
	CRangeValidatedArray<cbrushside_t>	map_brushsides;
	int									numboxbrushes;
	CRangeValidatedArray<cboxbrush_t>	map_boxbrushes;
	int									numplanes;
	CRangeValidatedArray<cplane_t>		map_planes;
	int									numnodes;
	CRangeValidatedArray<cnode_t>		map_nodes;
	int									numleafs;				// allow leaf funcs to be called without a map
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
	dvis_t								*map_vis;

	int									numentitychars;
	CDiscardableArray<char>				map_entitystring;

	int									numareas;
	CRangeValidatedArray<carea_t>		map_areas;
	int									numareaportals;
	CRangeValidatedArray<dareaportal_t>	map_areaportals;
	int									numclusters;
	char								*map_nullname;
	int									numtextures;
	char								*map_texturenames;
	CRangeValidatedArray<csurface_t>	map_surfaces;
	int									floodvalid;
	int									numportalopen;
	CRangeValidatedArray<bool>			portalopen;

	//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadTextures();
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadTexinfo(
		CUtlVector<unsigned short>& map_texinfo);
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadLeafs_Version_0(CMapLoadHelper& lh);
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadLeafs_Version_1(CMapLoadHelper& lh);
	void CollisionBSPData_LoadLeafs();
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadLeafBrushes();
	//----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadPlanes();
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadBrushes();
	inline bool IsBoxBrush(const cbrush_t& brush, dbrushside_t* pSides, cplane_t* pPlanes);
	inline void ExtractBoxBrush(cboxbrush_t* pBox, const cbrush_t& brush, dbrushside_t* pSides, cplane_t* pPlanes, CUtlVector<unsigned short>& map_texinfo);
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadBrushSides(CUtlVector<unsigned short>& map_texinfo);
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadSubmodels();
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadNodes();
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadAreas();
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadAreaPortals();
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadVisibility();
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadEntityString();
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadPhysics();
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void CollisionBSPData_LoadDispInfo();

	int g_DispCollTreeCount = 0;
	CDispCollTree* g_pDispCollTrees = NULL;
	alignedbbox_t* g_pDispBounds = NULL;

	class CDispLeafBuilder
	{
	public:
		CDispLeafBuilder(CCollisionBSPData* pBSPData)
		{
			m_pBSPData = pBSPData;
			// don't want this to resize much, so make the backing buffer large
			m_dispList.EnsureCapacity(MAX_MAP_DISPINFO * 2);

			// size both of these to the size of the array since there is exactly one per element
			m_leafCount.SetCount(m_pBSPData->GetDispCollTreesCount());
			m_firstIndex.SetCount(m_pBSPData->GetDispCollTreesCount());
			for (int i = 0; i < m_pBSPData->GetDispCollTreesCount(); i++)
			{
				m_leafCount[i] = 0;
				m_firstIndex[i] = -1;
			}
		}

		void BuildLeafListForDisplacement(int index)
		{
			// get tree and see if it is real (power != 0)
			CDispCollTree* pDispTree = m_pBSPData->GetDispCollTrees(index);
			if (!pDispTree || (pDispTree->GetPower() == 0))
				return;
			m_firstIndex[index] = m_dispList.Count();
			m_leafCount[index] = 0;
			const int MAX_NODES = 1024;
			int nodeList[MAX_NODES];
			int listRead = 0;
			int listWrite = 1;
			nodeList[0] = m_pBSPData->GetCModels(0)->headnode;
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
					cnode_t* pNode = m_pBSPData->GetNodes(nodeIndex);
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
			for (int i = 0; i < m_pBSPData->GetLeafsCount(); i++)
			{
				cleaf_t* pLeaf = m_pBSPData->GetLeafs(i);
				pLeaf->dispCount = 0;
			}
			// compute new count per leaf
			for (int i = 0; i < m_dispList.Count(); i++)
			{
				int leafIndex = m_dispList[i];
				cleaf_t* pLeaf = m_pBSPData->GetLeafs(leafIndex);
				pLeaf->dispCount++;
			}
			// point each leaf at the start of it's output range in the output array
			unsigned short firstDispIndex = 0;
			for (int i = 0; i < m_pBSPData->GetLeafsCount(); i++)
			{
				cleaf_t* pLeaf = m_pBSPData->GetLeafs(i);
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
					cleaf_t* pLeaf = m_pBSPData->GetLeafs(leafIndex);
					int outListIndex = pLeaf->dispListStart + pLeaf->dispCount;	// output position for this leaf
					pLeafList[outListIndex] = i;							// write the reference there
					Assert(outListIndex < GetDispListCount());
					pLeaf->dispCount++;										// move this leaf's output pointer
				}
			}
		}

	private:
		CCollisionBSPData* m_pBSPData;
		// this is a list of all of the leaf indices for each displacement
		CUtlVector<unsigned short> m_dispList;
		// this is the first entry into dispList for each displacement
		CUtlVector<int> m_firstIndex;
		// this is the # of leaf entries for each displacement
		CUtlVector<unsigned short> m_leafCount;
	};
	// setup
	void CM_DispTreeLeafnum();
};

//=============================================================================
//
// physics collision
//
class IPhysicsSurfaceProps;
class IPhysicsCollision;

extern IPhysicsSurfaceProps* physprop;
extern IPhysicsCollision* physcollision;

//=============================================================================
//
// This might eventually become a class/interface when we want multiple instances
// etc.......for now....
//
//extern csurface_t nullsurface;

//extern bool bStartSolidDisp;

bool CollisionBSPData_Init( CCollisionBSPData *pBSPData );
void CollisionBSPData_Destroy( CCollisionBSPData *pBSPData );
void CollisionBSPData_LinkPhysics( void );

void CollisionBSPData_PreLoad( CCollisionBSPData *pBSPData );
bool CollisionBSPData_Load( const char *pName, CCollisionBSPData *pBSPData );
void CollisionBSPData_PostLoad( void );

//-----------------------------------------------------------------------------
// Returns the collision tree associated with the ith displacement
//-----------------------------------------------------------------------------

CDispCollTree* CollisionBSPData_GetCollisionTree( int i );

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline CCollisionBSPData *GetCollisionBSPData( void /*int ndxBSP*/ )
{
	extern CCollisionBSPData g_BSPData;								// the global collision bsp
	return &g_BSPData;
}

//=============================================================================
//
// Collision Model Counts
//
#ifdef COUNT_COLLISIONS
class CCollisionCounts
{
public:
	int		m_PointContents;
	int		m_Traces;
	int		m_BrushTraces;
	int		m_DispTraces;
	int		m_Stabs;
};

void CollisionCounts_Init( CCollisionCounts *pCounts );

extern CCollisionCounts g_CollisionCounts;
#endif


//=============================================================================
//
// Older Code That Should Live Here!!!!
// a shared file should contain all the CDispCollTree stuff
//


//extern int g_DispCollTreeCount;
//extern CDispCollTree *g_pDispCollTrees;
//extern alignedbbox_t *g_pDispBounds;
//extern csurface_t dispSurf;

// memory allocation/de-allocation
//void DispCollTrees_FreeLeafList( CCollisionBSPData *pBSPData );



// collision
void CM_TestInDispTree( TraceInfo_t *pTraceInfo, cleaf_t *pLeaf, Vector const &traceStart, 
				Vector const &boxMin, Vector const &boxMax, int collisionMask, trace_t *pTrace );
template <bool IS_POINT>
void FASTCALL CM_TraceToDispTree( TraceInfo_t *pTraceInfo, CDispCollTree *pDispTree, float startFrac, float endFrac );
void CM_PostTraceToDispTree( TraceInfo_t *pTraceInfo );

//=============================================================================
//
// profiling purposes only -- remove when done!!!
//

void CM_TestBoxInBrush ( const Vector& mins, const Vector& maxs, const Vector& p1,
					  trace_t *trace, cbrush_t *brush, BOOL bDispSurf );
void FASTCALL CM_RecursiveHullCheck ( TraceInfo_t *pTraceInfo, int num, const float p1f, const float p2f );


//=============================================================================

inline bool TraceInfo_t::Visit( cbrush_t *pBrush, int ndxBrush, TraceCounter_t cachedCount, TraceCounter_t *pCachedCounters )
{
	TraceCounter_t * RESTRICT pCounter = pCachedCounters + ndxBrush;

	if ( *pCounter == cachedCount )
	{
		return false;
	}

	*pCounter = cachedCount;
	return true;
}

FORCEINLINE bool TraceInfo_t::Visit( int dispCounter, TraceCounter_t cachedCount, TraceCounter_t *pCachedCounters )
{
	TraceCounter_t * RESTRICT pCounter = pCachedCounters + dispCounter;

	if ( *pCounter == cachedCount )
	{
		return false;
	}

	*pCounter = cachedCount;
	return true;
}

FORCEINLINE bool TraceInfo_t::Visit( cbrush_t *pBrush, int ndxBrush )
{
	return Visit( pBrush, ndxBrush, GetCount(), GetBrushCounters() );
}

FORCEINLINE bool TraceInfo_t::Visit( int dispIndex )
{
	return Visit( dispIndex, GetCount(), GetDispCounters() );
}

#endif // CMODEL_PRIVATE_H
