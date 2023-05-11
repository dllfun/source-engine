//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Entity that simulates bullets that are underwater.
//
//=============================================================================//

#ifndef WEAPON_WATERBULLET_H
#define WEAPON_WATERBULLET_H
#ifdef _WIN32
#pragma once
#endif

#define WATER_BULLET_BUBBLES_PER_INCH 0.05f

//=========================================================
//=========================================================
class CWaterBullet : public CBaseAnimating
{
	DECLARE_CLASS( CWaterBullet, CBaseAnimating );

public:
	void Precache();
	void Spawn( const Vector &vecOrigin, const Vector &vecDir );
	void Touch( CBaseEntity *pOther );
	void BulletThink();

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
	DECLARE_SEND_TABLE_ACCESS(DT_WaterBullet);

	BEGIN_SEND_TABLE(CWaterBullet, DT_WaterBullet, DT_BaseAnimating)

	END_SEND_TABLE(DT_WaterBullet)
};

#endif // WEAPON_WATERBULLET_H
