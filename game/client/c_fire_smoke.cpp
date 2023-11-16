//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "iviewrender.h"
#include "clienteffectprecachesystem.h"
#include "studio.h"
#include "bone_setup.h"
#include "engine/ivmodelinfo.h"
#include "c_fire_smoke.h"
#include "engine/IEngineSound.h"
#include "iefx.h"
#include "dlight.h"
#include "tier0/icommandline.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_Fire : public C_BaseEntity {
public:
	DECLARE_CLIENTCLASS();
	C_Fire() {}

public:
	BEGIN_INIT_RECV_TABLE(C_Fire)
	BEGIN_RECV_TABLE(C_Fire, DT_Fire, DT_BaseEntity)

	END_RECV_TABLE(DT_Fire)
	END_INIT_RECV_TABLE()
};

IMPLEMENT_CLIENTCLASS(C_Fire, DT_Fire, CFire)

CLIENTEFFECT_REGISTER_BEGIN( SmokeStackMaterials )
	CLIENTEFFECT_MATERIAL( "particle/SmokeStack" )
CLIENTEFFECT_REGISTER_END()






//Receive datatable
IMPLEMENT_CLIENTCLASS( C_FireSmoke, DT_FireSmoke, CFireSmoke )


//==================================================
// C_FireSmoke
//==================================================

C_FireSmoke::C_FireSmoke()
{
	SetClassname("C_FireSmoke");
}

C_FireSmoke::~C_FireSmoke()
{

	// Shut down our effect if we have it
	if ( m_hEffect )
	{
		m_hEffect->StopEmission(false, false , true);
		m_hEffect = NULL;
	}

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FireSmoke::Simulate( void )
{
}

#define	FLAME_ALPHA_START	0.9f
#define FLAME_ALPHA_END		1.0f

#define	FLAME_TRANS_START	0.75f

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FireSmoke::AddFlames( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bnewentity - 
//-----------------------------------------------------------------------------
void C_FireSmoke::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );


	if ( updateType == DATA_UPDATE_CREATED )
	{
		Start();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FireSmoke::UpdateEffects( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_FireSmoke::ShouldDraw()
{

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FireSmoke::Start( void )
{
	const char *lpszEffectName;
	int nSize = (int) floor( m_flStartScale / 36.0f );
	switch ( nSize )
	{
	case 0:
		lpszEffectName = ( m_nFlags & bitsFIRESMOKE_SMOKE ) ? "env_fire_tiny_smoke" : "env_fire_tiny";
		break;

	case 1:
		lpszEffectName = ( m_nFlags & bitsFIRESMOKE_SMOKE ) ? "env_fire_small_smoke" : "env_fire_small";
		break;

	case 2:
		lpszEffectName = ( m_nFlags & bitsFIRESMOKE_SMOKE ) ? "env_fire_medium_smoke" : "env_fire_medium";
		break;

	case 3:
	default:
		lpszEffectName = ( m_nFlags & bitsFIRESMOKE_SMOKE ) ? "env_fire_large_smoke" : "env_fire_large";
		break;
	}

	// Create the effect of the correct size
	m_hEffect = ParticleProp()->Create( lpszEffectName, PATTACH_ABSORIGIN );

}


//-----------------------------------------------------------------------------
// Purpose: FIXME: what's the right way to do this?
//-----------------------------------------------------------------------------
void C_FireSmoke::StartClientOnly( void )
{
	Start();

	ClientEntityList().AddNonNetworkableEntity(	this );
	CollisionProp()->CreatePartitionHandle();
	AddEffects( EF_NORECEIVESHADOW | EF_NOSHADOW );
	AddToLeafSystem();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FireSmoke::RemoveClientOnly(void)
{
	ClientThinkList()->RemoveThinkable( GetClientHandle() );

	// Remove from the client entity list.
	ClientEntityList().RemoveEntity( GetClientHandle() );

	partition->Remove( PARTITION_CLIENT_SOLID_EDICTS | PARTITION_CLIENT_RESPONSIVE_EDICTS | PARTITION_CLIENT_NON_STATIC_EDICTS, CollisionProp()->GetPartitionHandle() );

	RemoveFromLeafSystem();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FireSmoke::UpdateAnimation( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FireSmoke::UpdateFlames( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FireSmoke::UpdateScale( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FireSmoke::Update( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FireSmoke::FindClipPlane( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Spawn smoke (...duh)
//-----------------------------------------------------------------------------

void C_FireSmoke::SpawnSmoke( void )
{
}


IMPLEMENT_CLIENTCLASS( C_EntityFlame, DT_EntityFlame, CEntityFlame )


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_EntityFlame::C_EntityFlame( void ) :
m_hEffect( NULL )
{
	m_hOldAttached = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_EntityFlame::~C_EntityFlame( void )
{
	StopEffect();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_EntityFlame::StopEffect( void )
{
	if ( m_hEffect )
	{
		ParticleProp()->StopEmission( m_hEffect, true );
		m_hEffect->SetControlPointEntity( 0, NULL );
		m_hEffect->SetControlPointEntity( 1, NULL );
		m_hEffect = NULL;
	}

	if ( m_hEntAttached )
	{
		m_hEntAttached->RemoveFlag( FL_ONFIRE );
		m_hEntAttached->SetEffectEntity( NULL );
		m_hEntAttached->StopSound( "General.BurningFlesh" );
		m_hEntAttached->StopSound( "General.BurningObject" );
		
		
		m_hEntAttached = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_EntityFlame::UpdateOnRemove( void )
{
	StopEffect();
	BaseClass::UpdateOnRemove();
}

void C_EntityFlame::CreateEffect( void )
{
	if ( m_hEffect )
	{
		ParticleProp()->StopEmission( m_hEffect, true );
		m_hEffect->SetControlPointEntity( 0, NULL );
		m_hEffect->SetControlPointEntity( 1, NULL );
		m_hEffect = NULL;
	}

#ifdef TF_CLIENT_DLL
	m_hEffect = ParticleProp()->Create( "burningplayer_red", PATTACH_ABSORIGIN_FOLLOW );
#else
	m_hEffect = ParticleProp()->Create( "burning_character", PATTACH_ABSORIGIN_FOLLOW );
#endif

	if ( m_hEffect )
	{
		C_BaseEntity *pEntity = m_hEntAttached;
		m_hOldAttached = m_hEntAttached;

		ParticleProp()->AddControlPoint( m_hEffect, 1, pEntity, PATTACH_ABSORIGIN_FOLLOW );
		m_hEffect->SetControlPoint( 0, GetAbsOrigin() );
		m_hEffect->SetControlPoint( 1, GetAbsOrigin() );
		m_hEffect->SetControlPointEntity( 0, pEntity );
		m_hEffect->SetControlPointEntity( 1, pEntity );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_EntityFlame::OnDataChanged( DataUpdateType_t updateType )
{
	if ( updateType == DATA_UPDATE_CREATED )
	{
		CreateEffect();
	}

	// FIXME: This is a bit of a shady path
	if ( updateType == DATA_UPDATE_DATATABLE_CHANGED )
	{
		// If our owner changed, then recreate the effect
		if ( m_hEntAttached.ToInt() != m_hOldAttached.ToInt() )
		{
			CreateEffect();
		}
	}

	BaseClass::OnDataChanged( updateType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_EntityFlame::Simulate( void )
{
	if ( gpGlobals->GetFrameTime() <= 0.0f )
		return;

#ifdef HL2_EPISODIC 

	if ( IsEffectActive(EF_BRIGHTLIGHT) || IsEffectActive(EF_DIMLIGHT) )
	{
		dlight_t *dl = effects->CL_AllocDlight ( index );
		dl->origin = GetAbsOrigin();
 		dl->origin[2] += 16;
		dl->color.r = 254;
		dl->color.g = 174;
		dl->color.b = 10;
		dl->radius = random->RandomFloat(400,431);
		dl->die = gpGlobals->GetCurTime() + 0.001;
	}

#endif // HL2_EPISODIC 
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_EntityFlame::ClientThink( void )
{
	StopEffect();
	Release();
}
