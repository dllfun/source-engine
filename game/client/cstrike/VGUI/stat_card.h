//========= Copyright Valve Corporation, All rights reserved. ============//
//
//
// $NoKeywords: $
//=============================================================================//

#ifndef STATCARD_H
#define STATCARD_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/IScheme.h>
#include "hud.h"
#include "hudelement.h"
#include "vgui_avatarimage.h"
#include "cs_shareddefs.h"
#include <vgui_controls/EditablePanel.h>

//using namespace vgui;

class StatCard : public vgui::EditablePanel
{
private:
	DECLARE_CLASS_SIMPLE( StatCard, vgui::EditablePanel );

public:
	StatCard(vgui::Panel *parent, const char *name);
    ~StatCard();
	
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );	
	void UpdateInfo();

protected:
	

	vgui::ImagePanel*			m_pAvatarDefault;
	//ImagePanel*			m_pBackgroundArt;
	CAvatarImagePanel*	m_pAvatar;

	vgui::Label* m_pName;
	vgui::Label* m_pKillToDeathRatio;
	vgui::Label* m_pStars;
	

private:
};

#endif //STATCARD_H