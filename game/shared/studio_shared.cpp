//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

//#include "cbase.h"
#include "studio.h"
#include "engine/ivmodelinfo.h"
#include "utlsymbol.h"
#include "datacache/imdlcache.h"
#include "tier3/tier3.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

////////////////////////////////////////////////////////////////////////
const studiohdr_t *studiohdr_t::FindModel( void **cache, char const * pModelName) const
{
	MDLHandle_t handle = g_pMDLCache->FindMDL(pModelName);
	*cache = (void*)(uintp)handle;
	return g_pMDLCache->GetStudioHdr(handle);
}

virtualmodel_t *studiohdr_t::GetVirtualModel( void ) const
{
	if ( numincludemodels == 0 )
		return NULL;
	MDLHandle_t handle = VoidPtrToMDLHandle(VirtualModel());
	return g_pMDLCache->GetVirtualModelFast(this, handle);
}

const studiohdr_t *virtualgroup_t::GetStudioHdr( ) const
{
	MDLHandle_t handle = VoidPtrToMDLHandle(this->cache);
	return g_pMDLCache->GetStudioHdr(handle);
}

byte *studiohdr_t::GetAnimBlock( int iBlock ) const
{
	MDLHandle_t handle = VoidPtrToMDLHandle(this->VirtualModel());
	return g_pMDLCache->GetAnimBlock(handle, iBlock);
}

int	studiohdr_t::GetAutoplayList( unsigned short **pOut ) const
{
	MDLHandle_t handle = VoidPtrToMDLHandle(this->VirtualModel());
	return g_pMDLCache->GetAutoplayList(handle, pOut);
}
