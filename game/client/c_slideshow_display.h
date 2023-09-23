//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef C_SLIDESHOW_DISPLAY_H
#define C_SLIDESHOW_DISPLAY_H

#include "cbase.h"
#include "utlvector.h"


struct SlideMaterialList_t
{
	char				szSlideKeyword[64];
	CUtlVector<int>		iSlideMaterials;
	CUtlVector<int>		iSlideIndex;
};


class C_SlideshowDisplay : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_SlideshowDisplay, CBaseEntity );
	DECLARE_CLIENTCLASS();

	C_SlideshowDisplay();
	virtual ~C_SlideshowDisplay();

	void Spawn( void );

	virtual void	OnDataChanged( DataUpdateType_t updateType );

	void ClientThink( void );

	bool IsEnabled( void ) { return m_bEnabled; }

	void GetDisplayText( char *pchText ) { Q_strcpy( pchText, m_szDisplayText ); }
	int CurrentMaterialIndex( void ) { return m_iCurrentMaterialIndex; }
	int GetMaterialIndex( int iSlideIndex );
	int NumMaterials( void );
	int CurrentSlideIndex( void ) { return m_iCurrentSlideIndex; }

private:

	void BuildSlideShowImagesList( void );

private:

	bool	m_bEnabled;

	char	m_szDisplayText[ 128 ];

	char	m_szSlideshowDirectory[ 128 ];

	CUtlVector<SlideMaterialList_t*>	m_SlideMaterialLists;
	unsigned char						m_chCurrentSlideLists[ 16 ];
	int									m_iCurrentMaterialIndex;
	int									m_iCurrentSlideIndex;

	float	m_fMinSlideTime;
	float	m_fMaxSlideTime;

	float	m_NextSlideTime;

	int		m_iCycleType;
	bool	m_bNoListRepeats;
	int		m_iCurrentSlideList;
	int		m_iCurrentSlide;

public:
	BEGIN_INIT_RECV_TABLE(C_SlideshowDisplay)
	BEGIN_RECV_TABLE(C_SlideshowDisplay, DT_SlideshowDisplay, DT_BaseEntity)
		RecvPropBool(RECVINFO(m_bEnabled)),
		RecvPropString(RECVINFO(m_szDisplayText)),
		RecvPropString(RECVINFO(m_szSlideshowDirectory)),
		RecvPropArray3(RECVINFO_ARRAY(m_chCurrentSlideLists), RecvPropInt(RECVINFO(m_chCurrentSlideLists[0]))),
		RecvPropFloat(RECVINFO(m_fMinSlideTime)),
		RecvPropFloat(RECVINFO(m_fMaxSlideTime)),
		RecvPropInt(RECVINFO(m_iCycleType)),
		RecvPropBool(RECVINFO(m_bNoListRepeats)),
	END_RECV_TABLE(DT_SlideshowDisplay)
	END_INIT_RECV_TABLE()
};

extern CUtlVector< C_SlideshowDisplay* > g_SlideshowDisplays;

#endif //C_SLIDESHOW_STATS_DISPLAY_H