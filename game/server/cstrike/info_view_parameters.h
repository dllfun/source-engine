//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#ifndef INFO_VIEW_PARAMETERS_H
#define INFO_VIEW_PARAMETERS_H
#ifdef _WIN32
#pragma once
#endif


class CInfoViewParameters : public CBaseEntity
{
public:
	DECLARE_CLASS( CInfoViewParameters, CBaseEntity );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
	int m_nViewMode;

	BEGIN_INIT_SEND_TABLE(CInfoViewParameters)
	BEGIN_SEND_TABLE(CInfoViewParameters, DT_InfoViewParameters, DT_BaseEntity)

	END_SEND_TABLE(DT_InfoViewParameters)
	END_INIT_SEND_TABLE()
};


#endif // INFO_VIEW_PARAMETERS_H
