//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Wraps pointers to basic vgui interfaces
//
// $NoKeywords: $
//===========================================================================//

#ifndef VGUI_INTERNAL_H
#define VGUI_INTERNAL_H

#ifdef _WIN32
#pragma once
#endif

//#include <vgui/VGUI.h>
#include "interface.h"
//#include "tier3/tier3.h"
#include "xbox/xboxstubs.h"

#include "Cursor.h"
#include "Input.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include <vgui/VGUI.h>
#include <vgui/Dar.h>
#include <vgui/IInputInternal.h>
#include <vgui/IPanel.h>
#include <vgui/ISystem.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/IClientPanel.h>
#include <vgui/IScheme.h>
#include <vgui/IHTML.h>
#include <KeyValues.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#ifdef OSX
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
#include <tier0/dbg.h>
#include <tier1/utlhandletable.h>
#include "vgui_internal.h"
#include "VPanel.h"
#include "IMessageListener.h"
#include "tier2/tier2.h"
#include "tier3/tier3.h"
#include "utllinkedlist.h"
#include "utlpriorityqueue.h"
#include "utlvector.h"
#include "tier0/vprof.h"
#include "tier0/icommandline.h"
#include "inputsystem/iinputsystem.h"
#include <vgui_controls/Panel.h>
#include "materialsystem/imaterialsystem.h"
#include "bitmap/imageformat.h"
#include "Memorybitmap.h"

#ifdef GetClassName
#undef GetClassName
#endif
#ifdef CreateFont
#undef CreateFont
#endif
#ifdef PostMessage
#undef PostMessage
#endif

namespace vgui
{

} // namespace vgui

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Single item in the message queue
//-----------------------------------------------------------------------------
struct MessageItem_t
{
	KeyValues* _params; // message data
						// _params->GetName() is the message name

	VPANEL _messageTo;	// the panel this message is to be sent to
	VPANEL _from;		// the panel this message is from (if any)
	float _arrivalTime;	// time at which the message should be passed on to the recipient

	int _messageID;		// incrementing message index
};

class CVGui : public CTier3AppSystem< IVGui >
{
	typedef CTier3AppSystem< IVGui > BaseClass;

public:
	CVGui();
	~CVGui();

	//-----------------------------------------------------------------------------
		// SRC specific stuff
		// Here's where the app systems get to learn about each other 
	virtual bool Connect(CreateInterfaceFn factory);
	virtual void Disconnect();

	// Here's where systems can access other interfaces implemented by this object
	// Returns NULL if it doesn't implement the requested interface
	virtual void* QueryInterface(const char* pInterfaceName);

	// Init, shutdown
	virtual InitReturnVal_t Init();
	virtual void Shutdown();
	// End of specific interface
//-----------------------------------------------------------------------------


	virtual void RunFrame();

	virtual void Start()
	{
		m_bRunning = true;
	}

	// signals vgui to Stop running
	virtual void Stop()
	{
		m_bRunning = false;
	}

	// returns true if vgui is current active
	virtual bool IsRunning()
	{
		return m_bRunning;
	}

	virtual void ShutdownMessage(unsigned int shutdownID);

	// safe-pointer handle methods
	virtual VPANEL AllocPanel();
	virtual void FreePanel(VPANEL ipanel);
	//virtual HPanel PanelToHandle(VPANEL panel);
	//virtual VPANEL HandleToPanel(HPanel index);
	virtual void MarkPanelForDeletion(VPANEL panel);

	virtual void AddTickSignal(VPANEL panel, int intervalMilliseconds = 0);
	virtual void AddTickSignalToHead(VPANEL panel, int intervalMilliseconds = 0) OVERRIDE;
	virtual void RemoveTickSignal(VPANEL panel);


	// message pump method
	virtual void PostMessage(VPANEL target, KeyValues* params, VPANEL from, float delaySeconds = 0.0f);

	virtual void SetSleep(bool state) { m_bDoSleep = state; };
	virtual bool GetShouldVGuiControlSleep() { return m_bDoSleep; }

	virtual void DPrintf(const char* format, ...);
	virtual void DPrintf2(const char* format, ...);
	virtual void SpewAllActivePanelNames();

	// Creates/ destroys vgui contexts, which contains information
	// about which controls have mouse + key focus, for example.
	virtual HContext CreateContext();
	virtual void DestroyContext(HContext context);

	// Associates a particular panel with a vgui context
	// Associating NULL is valid; it disconnects the panel from the context
	virtual void AssociatePanelWithContext(HContext context, VPANEL pRoot);

	// Activates a particular input context, use DEFAULT_VGUI_CONTEXT
	// to get the one normally used by VGUI
	virtual void ActivateContext(HContext context);

	// enables VR mode
	virtual void SetVRMode(bool bVRMode) OVERRIDE
	{
		m_bVRMode = bVRMode;
	}
	virtual bool GetVRMode() OVERRIDE
	{
		return m_bVRMode;
	}

	bool IsDispatchingMessages(void)
	{
		return m_InDispatcher;
	}

	//void* HandleToVPanel(VPANEL index)
	//{
	//	if (!index || !m_HandleTable.IsHandleValid(index))
	//	{
	//		return NULL;
	//	}
	//	return m_HandleTable.GetHandle((UtlHandle_t)index);
	//}

	vgui::VPanel* InternalHandleToVPanel(VPANEL index)
	{
		if (!index || !m_HandleTable.IsHandleValid(index))
		{
			return NULL;
		}
		return m_HandleTable.GetHandle((UtlHandle_t)index);
	}

	virtual void Init(VPANEL vguiPanel, IClientPanel* panel)
	{
		InternalHandleToVPanel(vguiPanel)->Init(panel);
	}

	virtual Panel* GetPanel(VPANEL vguiPanel, const char* moduleName);

	// returns a pointer to the Client panel
	virtual IClientPanel* Client(VPANEL vguiPanel)
	{
		return InternalHandleToVPanel(vguiPanel)->Client();
	}

	// methods
	virtual void SetPos(VPANEL vguiPanel, int x, int y)
	{
		InternalHandleToVPanel(vguiPanel)->SetPos(x, y);
	}

	virtual void GetPos(VPANEL vguiPanel, int& x, int& y)
	{
		InternalHandleToVPanel(vguiPanel)->GetPos(x, y);
	}

	virtual void SetSize(VPANEL vguiPanel, int wide, int tall)
	{
		InternalHandleToVPanel(vguiPanel)->SetSize(wide, tall);
	}

	virtual void GetSize(VPANEL vguiPanel, int& wide, int& tall)
	{
		InternalHandleToVPanel(vguiPanel)->GetSize(wide, tall);
	}

	virtual void SetMinimumSize(VPANEL vguiPanel, int wide, int tall)
	{
		InternalHandleToVPanel(vguiPanel)->SetMinimumSize(wide, tall);
	}

	virtual void GetMinimumSize(VPANEL vguiPanel, int& wide, int& tall)
	{
		InternalHandleToVPanel(vguiPanel)->GetMinimumSize(wide, tall);
	}

	virtual void SetZPos(VPANEL vguiPanel, int z)
	{
		InternalHandleToVPanel(vguiPanel)->SetZPos(z);
	}

	virtual int GetZPos(VPANEL vguiPanel)
	{
		return InternalHandleToVPanel(vguiPanel)->GetZPos();
	}

	virtual void GetAbsPos(VPANEL vguiPanel, int& x, int& y)
	{
		InternalHandleToVPanel(vguiPanel)->GetAbsPos(x, y);
	}

	virtual void GetClipRect(VPANEL vguiPanel, int& x0, int& y0, int& x1, int& y1)
	{
		InternalHandleToVPanel(vguiPanel)->GetClipRect(x0, y0, x1, y1);
	}

	virtual void SetInset(VPANEL vguiPanel, int left, int top, int right, int bottom)
	{
		InternalHandleToVPanel(vguiPanel)->SetInset(left, top, right, bottom);
	}

	virtual void GetInset(VPANEL vguiPanel, int& left, int& top, int& right, int& bottom)
	{
		InternalHandleToVPanel(vguiPanel)->GetInset(left, top, right, bottom);
	}

	virtual void SetVisible(VPANEL vguiPanel, bool state)
	{
		InternalHandleToVPanel(vguiPanel)->SetVisible(state);
	}

	virtual void SetEnabled(VPANEL vguiPanel, bool state)
	{
		InternalHandleToVPanel(vguiPanel)->SetEnabled(state);
	}

	virtual bool IsVisible(VPANEL vguiPanel)
	{
		return InternalHandleToVPanel(vguiPanel)->IsVisible();
	}

	virtual bool IsEnabled(VPANEL vguiPanel)
	{
		return InternalHandleToVPanel(vguiPanel)->IsEnabled();
	}

	// Used by the drag/drop manager to always draw on top
	virtual bool IsTopmostPopup(VPANEL vguiPanel)
	{
		return InternalHandleToVPanel(vguiPanel)->IsTopmostPopup();
	}

	virtual void SetTopmostPopup(VPANEL vguiPanel, bool state)
	{
		return InternalHandleToVPanel(vguiPanel)->SetTopmostPopup(state);
	}

	virtual void SetParent(VPANEL vguiPanel, VPANEL newParent)
	{
		InternalHandleToVPanel(vguiPanel)->SetParent(newParent);
	}

	virtual int GetChildCount(VPANEL vguiPanel)
	{
		return InternalHandleToVPanel(vguiPanel)->GetChildCount();
	}

	virtual VPANEL GetChild(VPANEL vguiPanel, int index)
	{
		return InternalHandleToVPanel(vguiPanel)->GetChild(index);
	}

	virtual CUtlVector< VPANEL >& GetChildren(VPANEL vguiPanel)
	{
		return InternalHandleToVPanel(vguiPanel)->GetChildren();
	}

	virtual VPANEL GetParent(VPANEL vguiPanel)
	{
		return InternalHandleToVPanel(vguiPanel)->GetParent();
	}

	virtual void MoveToFront(VPANEL vguiPanel)
	{
		InternalHandleToVPanel(vguiPanel)->MoveToFront();
	}

	virtual void MoveToBack(VPANEL vguiPanel)
	{
		InternalHandleToVPanel(vguiPanel)->MoveToBack();
	}

	virtual bool HasParent(VPANEL vguiPanel, VPANEL potentialParent)
	{
		if (!vguiPanel)
			return false;

		return InternalHandleToVPanel(vguiPanel)->HasParent(potentialParent);
	}

	virtual bool IsPopup(VPANEL vguiPanel)
	{
		return InternalHandleToVPanel(vguiPanel)->IsPopup();
	}

	virtual void SetPopup(VPANEL vguiPanel, bool state)
	{
		InternalHandleToVPanel(vguiPanel)->SetPopup(state);
	}

	virtual bool IsFullyVisible(VPANEL vguiPanel)
	{
		return InternalHandleToVPanel(vguiPanel)->IsFullyVisible();
	}

	// calculates the panels current position within the hierarchy
	virtual void Solve(VPANEL vguiPanel)
	{
		InternalHandleToVPanel(vguiPanel)->Solve();
	}

	// used by ISurface to store platform-specific data
	virtual SurfacePlat* Plat(VPANEL vguiPanel)
	{
		return InternalHandleToVPanel(vguiPanel)->Plat();
	}

	virtual void SetPlat(VPANEL vguiPanel, SurfacePlat* Plat)
	{
		InternalHandleToVPanel(vguiPanel)->SetPlat(Plat);
	}

	virtual const char* GetName(VPANEL vguiPanel)
	{
		return InternalHandleToVPanel(vguiPanel)->GetName();
	}

	virtual const char* GetClassName(VPANEL vguiPanel)
	{
		return InternalHandleToVPanel(vguiPanel)->GetClassName();
	}

	virtual HScheme GetScheme(VPANEL vguiPanel)
	{
		return InternalHandleToVPanel(vguiPanel)->GetScheme();
	}

	//virtual bool IsProportional(VPANEL vguiPanel)
	//{
	//	return Client(vguiPanel)->IsProportional();
	//}

	//virtual bool IsAutoDeleteSet(VPANEL vguiPanel)
	//{
	//	return Client(vguiPanel)->IsAutoDeleteSet();
	//}

	//virtual void DeletePanel(VPANEL vguiPanel)
	//{
	//	Client(vguiPanel)->DeletePanel();
	//}

	virtual void SendMessage(VPANEL vguiPanel, KeyValues* params, VPANEL ifrompanel)
	{
		InternalHandleToVPanel(vguiPanel)->SendMessage(params, ifrompanel);
	}

	virtual void Think(VPANEL vguiPanel)
	{
		Client(vguiPanel)->Think();
	}

	virtual void PerformApplySchemeSettings(VPANEL vguiPanel)
	{
		Client(vguiPanel)->PerformApplySchemeSettings();
	}

	virtual void PaintTraverse(VPANEL vguiPanel, bool forceRepaint, bool allowForce)
	{
		Client(vguiPanel)->PaintTraverse(forceRepaint, allowForce);
	}

	virtual void Repaint(VPANEL vguiPanel)
	{
		Client(vguiPanel)->Repaint();
	}

	virtual VPANEL IsWithinTraverse(VPANEL vguiPanel, int x, int y, bool traversePopups)
	{
		return Client(vguiPanel)->IsWithinTraverse(x, y, traversePopups);
	}

	virtual void OnChildAdded(VPANEL vguiPanel, VPANEL child)
	{
		Client(vguiPanel)->OnChildAdded(child);
	}

	virtual void OnSizeChanged(VPANEL vguiPanel, int newWide, int newTall)
	{
		Client(vguiPanel)->OnSizeChanged(newWide, newTall);
	}

	virtual void InternalFocusChanged(VPANEL vguiPanel, bool lost)
	{
		Client(vguiPanel)->InternalFocusChanged(lost);
	}

	virtual bool RequestInfo(VPANEL vguiPanel, KeyValues* outputData)
	{
		return Client(vguiPanel)->RequestInfo(outputData);
	}

	virtual void RequestFocus(VPANEL vguiPanel, int direction = 0)
	{
		Client(vguiPanel)->RequestFocus(direction);
	}

	virtual bool RequestFocusPrev(VPANEL vguiPanel, VPANEL existingPanel)
	{
		return Client(vguiPanel)->RequestFocusPrev(existingPanel);
	}

	virtual bool RequestFocusNext(VPANEL vguiPanel, VPANEL existingPanel)
	{
		return Client(vguiPanel)->RequestFocusNext(existingPanel);
	}

	virtual VPANEL GetCurrentKeyFocus(VPANEL vguiPanel)
	{
		return Client(vguiPanel)->GetCurrentKeyFocus();
	}

	virtual int GetTabPosition(VPANEL vguiPanel)
	{
		return Client(vguiPanel)->GetTabPosition();
	}


	virtual const char* GetModuleName(VPANEL vguiPanel)
	{
		return Client(vguiPanel)->GetModuleName();
	}

	virtual void SetKeyBoardInputEnabled(VPANEL vguiPanel, bool state)
	{
		InternalHandleToVPanel(vguiPanel)->SetKeyBoardInputEnabled(state);
	}

	virtual void SetMouseInputEnabled(VPANEL vguiPanel, bool state)
	{
		InternalHandleToVPanel(vguiPanel)->SetMouseInputEnabled(state);
	}

	virtual bool IsMouseInputEnabled(VPANEL vguiPanel)
	{
		return InternalHandleToVPanel(vguiPanel)->IsMouseInputEnabled();
	}

	virtual bool IsKeyBoardInputEnabled(VPANEL vguiPanel)
	{
		return InternalHandleToVPanel(vguiPanel)->IsKeyBoardInputEnabled();
	}

	virtual void SetSiblingPin(VPANEL vguiPanel, VPANEL newSibling, byte iMyCornerToPin = 0, byte iSiblingCornerToPinTo = 0)
	{
		return InternalHandleToVPanel(vguiPanel)->SetSiblingPin(newSibling, iMyCornerToPin, iSiblingCornerToPinTo);
	}

	// windows stuff
	virtual void CreatePopup(VPANEL panel, bool minimized, bool showTaskbarIcon = true, bool disabled = false, bool mouseInput = true, bool kbInput = true);

	virtual void MovePopupToFront(vgui::VPANEL panel);

	virtual void MovePopupToBack(VPANEL panel);

	void RemovePopup(vgui::VPANEL panel);

	virtual int GetPopupCount();

	virtual vgui::VPANEL GetPopup(int index);

	void ResetPopupList();

	void AddPopup(vgui::VPANEL panel);

	void AddPopupsToList(vgui::VPANEL panel);

	virtual void AddPanel(vgui::VPANEL panel);
	virtual void ReleasePanel(vgui::VPANEL panel);

	virtual void SetTitle(vgui::VPANEL panel, const wchar_t* title);
	virtual const wchar_t* GetTitle(vgui::VPANEL panel);

	virtual void BringToFront(vgui::VPANEL panel);

	virtual void SetForegroundWindow(vgui::VPANEL panel);

	virtual void SetTopLevelFocus(vgui::VPANEL panel);

	virtual void SetPanelVisible(vgui::VPANEL panel, bool state);
	virtual void SetMinimized(vgui::VPANEL panel, bool state);
	// returns true if a panel is minimzed
	bool IsMinimized(vgui::VPANEL panel);
	virtual void FlashWindow(vgui::VPANEL panel, bool state);

	virtual void SwapBuffers(vgui::VPANEL panel);
	virtual void Invalidate(vgui::VPANEL panel);

	virtual void ApplyChanges();
	virtual bool IsWithin(int x, int y);
	virtual bool HasFocus();

	virtual bool RecreateContext(vgui::VPANEL panel);

	// notify icons?!?
	virtual vgui::VPANEL GetNotifyPanel();
	virtual void SetNotifyIcon(vgui::VPANEL context, vgui::HTexture icon, vgui::VPANEL panelToReceiveMessages, const char* text);

	virtual void SetAsTopMost(vgui::VPANEL panel, bool state);

	virtual void SetAsToolBar(vgui::VPANEL panel, bool state);

	virtual void SolveTraverse(vgui::VPANEL panel, bool forceApplySchemeSettings = false);

	virtual vgui::IHTML* CreateHTMLWindow(vgui::IHTMLEvents* events, vgui::VPANEL context);
	virtual void PaintHTMLWindow(vgui::IHTML* htmlwin);
	virtual void DeleteHTMLWindow(vgui::IHTML* htmlwin);
	virtual bool BHTMLWindowNeedsPaint(IHTML* htmlwin);

	virtual void SetModalPanel(VPANEL);
	virtual VPANEL GetModalPanel();

	virtual VPANEL GetTopmostPopup();

	virtual bool IsInThink(VPANEL panel);

	virtual void RestrictPaintToSinglePanel(vgui::VPANEL panel);
	virtual bool IsPanelUnderRestrictedPanel(VPANEL panel);
	virtual VPANEL GetRestrictPaintSinglePanel();

	virtual bool ShouldPaintChildPanel(vgui::VPANEL childPanel);

	virtual void CalculateMouseVisible();

	virtual bool NeedKBInput();

	virtual void SetPanelForInput(VPANEL vpanel);

	// Prevents vgui from changing the cursor
	virtual bool IsCursorLocked() const;
	virtual void SetCursor(vgui::HCursor cursor);

	virtual bool IsCursorVisible();
	virtual void SetCursorAlwaysVisible(bool visible);
	virtual void SetCursorPos(int x, int y);
	virtual void GetCursorPos(int& x, int& y);

	// Hook needed to Get input to work
	virtual void AttachToWindow(void* hwnd, bool bLetAppDriveInput);
	virtual bool HandleInputEvent(const InputEvent_t& event);

	virtual void UnlockCursor();
	virtual void LockCursor();

	virtual vgui::HCursor CreateCursorFromFile(char const* curOrAniFile, char const* pPathID);
	virtual void SetSoftwareCursor(bool bUseSoftwareCursor);
	virtual bool GetSoftwareCursor();
	virtual int GetSoftwareCursorTextureId(float* px, float* py);

	// Tells the surface to ignore windows messages
	virtual void EnableWindowsMessages(bool bEnable);

	virtual void SetEmbeddedPanel(vgui::VPANEL pEmbeddedPanel);
	virtual vgui::VPANEL GetEmbeddedPanel();

	virtual bool HasCursorPosFunctions() { return true; }

	virtual vgui::IImage* GetIconImageForFullPath(char const* pFullPath);

	virtual void EnableMouseCapture(vgui::VPANEL panel, bool state);

private:
	// VGUI contexts
	struct Context_t
	{
		HInputContext m_hInputContext;
	};

	struct Tick_t
	{
		VPANEL	panel;
		int		interval;
		int		nexttick;
		bool	bMarkDeleted;
		// Debugging
		char	panelname[64];
	};

	Tick_t* CreateNewTick(VPANEL panel, int intervalMilliseconds);

	// Returns the current context
	Context_t* GetContext(HContext context);

	VPANEL PanelCreated(vgui::VPanel* panel);
	vgui::VPanel* PanelDeleted(VPANEL panel);
	bool DispatchMessages();
	void DestroyAllContexts();
	void ClearMessageQueues();
	inline bool IsReentrant() const
	{
		return m_nReentrancyCount > 0;
	}

	// safe panel handle stuff
	CUtlHandleTable< vgui::VPanel, 20 > m_HandleTable;
	int m_iCurrentMessageID;

	bool m_bRunning : 1;
	bool m_bDoSleep : 1;
	bool m_InDispatcher : 1;
	bool m_bDebugMessages : 1;
	bool m_bVRMode : 1;
	bool m_bCanRemoveTickSignal : 1;
	int m_nReentrancyCount;

	CUtlVector< Tick_t* > m_TickSignalVec;
	CUtlLinkedList< Context_t >	m_Contexts;

	HContext m_hContext;
	Context_t m_DefaultContext;

#ifdef DEBUG
	int m_iDeleteCount, m_iDeletePanelCount;
#endif

	// message queue. holds all vgui messages generated by windows events
	CUtlLinkedList<MessageItem_t, ushort> m_MessageQueue;

	// secondary message queue, holds all vgui messages generated by vgui
	CUtlLinkedList<MessageItem_t, ushort> m_SecondaryQueue;

	// timing queue, holds all the messages that have to arrive at a specified time
	CUtlPriorityQueue<MessageItem_t> m_DelayedMessageQueue;

	// List of pop-up panels based on the type enum above (draw order vs last clicked)
	CUtlVector<VPANEL>	m_PopupList;

	class TitleEntry
	{
	public:
		TitleEntry()
		{
			panel = NULL;
			title[0] = 0;
		}

		vgui::VPANEL panel;
		wchar_t	title[128];
	};

	CUtlVector< TitleEntry >	m_Titles;

	int		GetTitleEntry(vgui::VPANEL panel);

	void InternalThinkTraverse(VPANEL panel);
	void InternalSolveTraverse(VPANEL panel);
	void InternalSchemeSettingsTraverse(VPANEL panel, bool forceApplySchemeSettings);

	bool m_bInThink : 1;
	VPANEL m_CurrentThinkPanel;

	// Root panel
	vgui::VPANEL m_pEmbeddedPanel;
	vgui::Panel* m_pDefaultEmbeddedPanel;
	vgui::VPANEL m_pRestrictedPanel;

	bool m_bNeedsKeyboard : 1;
	bool m_bNeedsMouse : 1;

	bool					m_cursorAlwaysVisible;
	vgui::HCursor			_currentCursor;

	// The attached HWND
	void* m_HWnd;
	// Is the app gonna call HandleInputEvent?
	bool m_bAppDrivesInput : 1;
	int m_nLastInputPollCount;

	CUtlDict< vgui::IImage*, unsigned short >	m_FileTypeImages;
};





namespace vgui{
	// <vgui/IInputInternal.h> header
	extern class vgui::IInputInternal* g_pInput;
	// <vgui/IScheme.h> header
	extern class vgui::ISchemeManager* g_pScheme;
	// <vgui/ISurface.h> header
	extern class IMatSystemSurface* g_pSurface;
	// <vgui/ISystem.h> header
	extern class vgui::ISystem* g_pSystem;

	bool VGui_InternalLoadInterfaces(CreateInterfaceFn* factoryList, int numFactories);

}

// <vgui/IVGui.h> header
extern class CVGui g_VGui;

// <vgui/IPanel.h> header
//extern class IPanel *g_pIPanel;

// methods
void vgui_strcpy(char* dst, int dstLen, const char* src);

#endif // VGUI_INTERNAL_H
