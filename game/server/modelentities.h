//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef MODELENTITIES_H
#define MODELENTITIES_H
#ifdef _WIN32
#pragma once
#endif

//!! replace this with generic start enabled/disabled
#define SF_WALL_START_OFF		0x0001
#define SF_IGNORE_PLAYERUSE		0x0002

//-----------------------------------------------------------------------------
// Purpose: basic solid geometry
// enabled state:	brush is visible
// disabled staute:	brush not visible
//-----------------------------------------------------------------------------
class CFuncBrush : public CBaseEntity
{
public:

	CFuncBrush();
	DECLARE_CLASS( CFuncBrush, CBaseEntity );
	DECLARE_SERVERCLASS();

	virtual void Spawn( void );
	bool CreateVPhysics( void );

	virtual int	ObjectCaps( void ) { return HasSpawnFlags(SF_IGNORE_PLAYERUSE) ? BaseClass::ObjectCaps() : BaseClass::ObjectCaps() | FCAP_IMPULSE_USE; }

	virtual int DrawDebugTextOverlays( void );

	void TurnOff( void );
	void TurnOn( void );

	// Input handlers
	void InputTurnOff( inputdata_t &inputdata );
	void InputTurnOn( inputdata_t &inputdata );
	void InputToggle( inputdata_t &inputdata );
	void InputSetExcluded( inputdata_t &inputdata );
	void InputSetInvert( inputdata_t &inputdata );

	enum BrushSolidities_e {
		BRUSHSOLID_TOGGLE = 0,
		BRUSHSOLID_NEVER  = 1,
		BRUSHSOLID_ALWAYS = 2,
	};

	BrushSolidities_e m_iSolidity;
	int m_iDisabled;
	bool m_bSolidBsp;
	string_t m_iszExcludedClass;
	bool m_bInvertExclusion;

	DECLARE_DATADESC();

	virtual bool IsOn( void );
public:
	BEGIN_INIT_SEND_TABLE(CFuncBrush)
	BEGIN_SEND_TABLE(CFuncBrush, DT_FuncBrush, DT_BaseEntity)

	END_SEND_TABLE(DT_FuncBrush)
	END_INIT_SEND_TABLE()
};


#endif // MODELENTITIES_H
