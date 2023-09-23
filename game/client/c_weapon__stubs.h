//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_WEAPON__STUBS_H
#define C_WEAPON__STUBS_H
#ifdef _WIN32
#pragma once
#endif

#include "client_class.h"

// This is an ugly hack to link client classes to weapons for now
//  these will be removed once we predict all weapons, especially TF2 weapons
#define STUB_WEAPON_CLASS_IMPLEMENT( entityName, className )		\
	BEGIN_PREDICTION_DATA( className )								\
	END_PREDICTION_DATA()											\
	LINK_ENTITY_TO_CLASS( entityName, className );


#define STUB_WEAPON_CLASS( entityName, className, baseClassName, baseTableName )	\
	class C_##className : public baseClassName					\
	{																\
		DECLARE_CLASS( C_##className, baseClassName );							\
	public:															\
		DECLARE_PREDICTABLE();										\
		DECLARE_CLIENTCLASS();										\
		C_##className() = default;											\
	private:														\
		C_##className( const C_##className & );						\
	public:															\
		BEGIN_INIT_RECV_TABLE(C_##className)\
		BEGIN_RECV_TABLE(C_##className, DT_##className, baseTableName)	\
		END_RECV_TABLE(DT_##className)\
		END_INIT_RECV_TABLE() \
	};																\
	STUB_WEAPON_CLASS_IMPLEMENT( entityName, C_##className );		\
	IMPLEMENT_CLIENTCLASS( C_##className, DT_##className, C##className )
	

#endif // C_WEAPON__STUBS_H
