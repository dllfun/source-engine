//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#ifndef VPANEL_H
#define VPANEL_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/Dar.h>
#include <vgui/IPanel.h>

#ifdef GetClassName
#undef GetClassName
#endif

namespace vgui
{

	class SurfaceBase;
	class IClientPanel;
	struct SerialPanel_t;

	//-----------------------------------------------------------------------------
	// Purpose: VGUI private implementation of panel
	//-----------------------------------------------------------------------------
	class VPanel
	{
	public:
		VPanel();
		virtual ~VPanel();

		virtual void Init(IClientPanel *attachedClientPanel);

		virtual SurfacePlat *Plat();
		virtual void SetPlat(SurfacePlat *pl);

		virtual VPANEL GetVPANEL() { return _vPanel; } // safe pointer handling
		virtual void SetVPANEL(VPANEL vPanel) { _vPanel = vPanel; }

		virtual bool IsPopup();
		virtual void SetPopup(bool state);
		virtual bool IsFullyVisible();

		virtual void SetPos(int x, int y);
		virtual void GetPos(int &x, int &y);
		virtual void SetSize(int wide,int tall);
		virtual void GetSize(int& wide,int& tall);
		virtual void SetMinimumSize(int wide,int tall);
		virtual void GetMinimumSize(int& wide,int& tall);
		virtual void SetZPos(int z);
		virtual int  GetZPos();

		virtual void GetAbsPos(int &x, int &y);
		virtual void GetClipRect(int &x0, int &y0, int &x1, int &y1);
		virtual void SetInset(int left, int top, int right, int bottom);
		virtual void GetInset(int &left, int &top, int &right, int &bottom);

		virtual void Solve();

		virtual void SetVisible(bool state);
		virtual void SetEnabled(bool state);
		virtual bool IsVisible();
		virtual bool IsEnabled();
		virtual void SetParent(VPANEL newParent);
		virtual int GetChildCount();
		virtual VPANEL GetChild(int index);
		virtual VPANEL GetParent();
		virtual void MoveToFront();
		virtual void MoveToBack();
		virtual bool HasParent(VPANEL potentialParent);

		virtual CUtlVector< VPANEL > &GetChildren();

		// gets names of the object (for debugging purposes)
		virtual const char *GetName();
		virtual const char *GetClassName();

		virtual HScheme GetScheme();

		// handles a message
		virtual void SendMessage(KeyValues *params, VPANEL ifromPanel);

		// wrapper to get Client panel interface
		virtual IClientPanel *Client() { return _clientPanel; }

		// input interest
		virtual void SetKeyBoardInputEnabled(bool state);
		virtual void SetMouseInputEnabled(bool state);
		virtual bool IsKeyBoardInputEnabled();
		virtual bool IsMouseInputEnabled();

		virtual bool IsTopmostPopup() const;
		virtual void SetTopmostPopup( bool bEnable );

		// sibling pins
		virtual void SetSiblingPin(VPANEL newSibling, byte iMyCornerToPin = 0, byte iSiblingCornerToPinTo = 0 );

	public:
		virtual void GetInternalAbsPos(int &x, int &y);
		virtual void TraverseLevel( int val );

	private:
		VPANEL _vPanel;
		Dar<VPANEL> _childDar;
		VPANEL _parent;
		SurfacePlat	*_plat;	// platform-specific data

		// our companion Client panel
		IClientPanel *_clientPanel;
		//VPANEL _clientPanelVPANEL; // Temp to check if _clientPanel is valid.

		short _pos[2];
		short _size[2];
		short _minimumSize[2];

		short _inset[4];
		short _clipRect[4];
		short _absPos[2];

		short _zpos;	// z-order position

		bool _visible : 1;
		bool _enabled : 1;
		bool _popup : 1;
		bool _mouseInput : 1; // used for popups
		bool _kbInput : 1;
		bool _isTopmostPopup : 1;

		VPANEL  _pinsibling;
		byte	_pinsibling_my_corner;
		byte	_pinsibling_their_corner;

		int	 m_nMessageContextId;
		int m_nThinkTraverseLevel;
	};

}


#endif // VPANEL_H
