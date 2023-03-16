//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Core implementation of vgui
//
// $NoKeywords: $
//===========================================================================//


#if defined( WIN32 ) && !defined( _X360 )
//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

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


#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif

#undef GetCursorPos // protected_things.h defines this, and it makes it so we can't access g_pInput->GetCursorPos.

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#define VPANEL_NORMAL	((vgui::SurfacePlat *) NULL)
#define VPANEL_MINIMIZED ((vgui::SurfacePlat *) 0x00000001)

using namespace vgui;
static const int WARN_PANEL_NUMBER = 32768; // in DEBUG if more panels than this are created then throw an Assert, helps catch panel leaks

static bool g_bSpewFocus = false;

//-----------------------------------------------------------------------------
// Purpose: Single item in the message queue
//-----------------------------------------------------------------------------
struct MessageItem_t
{
	KeyValues *_params; // message data
						// _params->GetName() is the message name

	HPanel _messageTo;	// the panel this message is to be sent to
	HPanel _from;		// the panel this message is from (if any)
	float _arrivalTime;	// time at which the message should be passed on to the recipient

	int _messageID;		// incrementing message index
};


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool PriorityQueueComp(const MessageItem_t& x, const MessageItem_t& y) 
{
	if (x._arrivalTime > y._arrivalTime)
	{
		return true;
	}
	else if (x._arrivalTime < y._arrivalTime)
	{
		return false;
	}

	// compare messageID's to ensure we have the messages in the correct order
	return (x._messageID > y._messageID);
}


// returns true if the specified panel is a child of the current modal panel
// if no modal panel is set, then this always returns TRUE
//static bool IsChildOfModalSubTree(VPANEL panel)
//{
//	if (!panel)
//		return true;
//
//	VPANEL modalSubTree = g_pInput->GetModalSubTree();
//
//	if (modalSubTree)
//	{
//		bool restrictMessages = g_pInput->ShouldModalSubTreeReceiveMessages();
//
//		// If panel is child of modal subtree, the allow messages to route to it if restrict messages is set
//		bool isChildOfModal = g_pVGui->HasParent(panel, modalSubTree);
//		if (isChildOfModal)
//		{
//			return restrictMessages;
//		}
//		// If panel is not a child of modal subtree, then only allow messages if we're not restricting them to the modal subtree
//		else
//		{
//			return !restrictMessages;
//		}
//	}
//
//	return true;
//}

class CMatEmbeddedPanel : public vgui::Panel
{
	typedef vgui::Panel BaseClass;
public:
	CMatEmbeddedPanel();
	virtual void OnThink();

	VPANEL IsWithinTraverse(int x, int y, bool traversePopups);
};

//-----------------------------------------------------------------------------
// Make sure the panel is the same size as the viewport
//-----------------------------------------------------------------------------
CMatEmbeddedPanel::CMatEmbeddedPanel() : BaseClass(NULL, "MatSystemTopPanel")
{
	SetPaintBackgroundEnabled(false);

#if defined( _X360 )
	SetPos(0, 0);
	SetSize(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
#endif
}

void CMatEmbeddedPanel::OnThink()
{
	int x, y, width, height;
	CMatRenderContextPtr pRenderContext(g_pMaterialSystem);
	pRenderContext->GetViewport(x, y, width, height);
	SetSize(width, height);
	SetPos(x, y);
	Repaint();
}

VPANEL CMatEmbeddedPanel::IsWithinTraverse(int x, int y, bool traversePopups)
{
	VPANEL retval = BaseClass::IsWithinTraverse(x, y, traversePopups);
	if (retval == GetVPanel())
		return 0;
	return retval;
}

//-----------------------------------------------------------------------------
// Purpose: Implementation of core vgui functionality
//-----------------------------------------------------------------------------
class CVGui : public CTier3AppSystem< IVGui >
{
	typedef CTier3AppSystem< IVGui > BaseClass;

public:
	CVGui();
	~CVGui();

//-----------------------------------------------------------------------------
	// SRC specific stuff
	// Here's where the app systems get to learn about each other 
	virtual bool Connect( CreateInterfaceFn factory );
	virtual void Disconnect();

	// Here's where systems can access other interfaces implemented by this object
	// Returns NULL if it doesn't implement the requested interface
	virtual void *QueryInterface( const char *pInterfaceName );

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
	virtual HPanel PanelToHandle(VPANEL panel);
	virtual VPANEL HandleToPanel(HPanel index);
	virtual void MarkPanelForDeletion(VPANEL panel);

	virtual void AddTickSignal(VPANEL panel, int intervalMilliseconds = 0);
	virtual void AddTickSignalToHead( VPANEL panel, int intervalMilliseconds = 0 ) OVERRIDE;
	virtual void RemoveTickSignal(VPANEL panel );


	// message pump method
	virtual void PostMessage(VPANEL target, KeyValues *params, VPANEL from, float delaySeconds = 0.0f);

	virtual void SetSleep( bool state ) { m_bDoSleep = state; };
	virtual bool GetShouldVGuiControlSleep() { return m_bDoSleep; }

	virtual void DPrintf(const char *format, ...);
	virtual void DPrintf2(const char *format, ...);
	virtual void SpewAllActivePanelNames();

	// Creates/ destroys vgui contexts, which contains information
	// about which controls have mouse + key focus, for example.
	virtual HContext CreateContext();
	virtual void DestroyContext( HContext context ); 

	// Associates a particular panel with a vgui context
	// Associating NULL is valid; it disconnects the panel from the context
	virtual void AssociatePanelWithContext( HContext context, VPANEL pRoot );

	// Activates a particular input context, use DEFAULT_VGUI_CONTEXT
	// to get the one normally used by VGUI
	virtual void ActivateContext( HContext context );

	// enables VR mode
	virtual void SetVRMode( bool bVRMode ) OVERRIDE
	{
		m_bVRMode = bVRMode;
	}
	virtual bool GetVRMode() OVERRIDE
	{
		return m_bVRMode;
	}

	bool IsDispatchingMessages( void )
	{
		return m_InDispatcher;
	}

	virtual void Init(VPANEL vguiPanel, IClientPanel* panel)
	{
		((VPanel*)vguiPanel)->Init(panel);
	}

	// returns a pointer to the Client panel
	virtual IClientPanel* Client(VPANEL vguiPanel)
	{
		return ((VPanel*)vguiPanel)->Client();
	}

	// methods
	virtual void SetPos(VPANEL vguiPanel, int x, int y)
	{
		((VPanel*)vguiPanel)->SetPos(x, y);
	}

	virtual void GetPos(VPANEL vguiPanel, int& x, int& y)
	{
		((VPanel*)vguiPanel)->GetPos(x, y);
	}

	virtual void SetSize(VPANEL vguiPanel, int wide, int tall)
	{
		((VPanel*)vguiPanel)->SetSize(wide, tall);
	}

	virtual void GetSize(VPANEL vguiPanel, int& wide, int& tall)
	{
		((VPanel*)vguiPanel)->GetSize(wide, tall);
	}

	virtual void SetMinimumSize(VPANEL vguiPanel, int wide, int tall)
	{
		((VPanel*)vguiPanel)->SetMinimumSize(wide, tall);
	}

	virtual void GetMinimumSize(VPANEL vguiPanel, int& wide, int& tall)
	{
		((VPanel*)vguiPanel)->GetMinimumSize(wide, tall);
	}

	virtual void SetZPos(VPANEL vguiPanel, int z)
	{
		((VPanel*)vguiPanel)->SetZPos(z);
	}

	virtual int GetZPos(VPANEL vguiPanel)
	{
		return ((VPanel*)vguiPanel)->GetZPos();
	}

	virtual void GetAbsPos(VPANEL vguiPanel, int& x, int& y)
	{
		((VPanel*)vguiPanel)->GetAbsPos(x, y);
	}

	virtual void GetClipRect(VPANEL vguiPanel, int& x0, int& y0, int& x1, int& y1)
	{
		((VPanel*)vguiPanel)->GetClipRect(x0, y0, x1, y1);
	}

	virtual void SetInset(VPANEL vguiPanel, int left, int top, int right, int bottom)
	{
		((VPanel*)vguiPanel)->SetInset(left, top, right, bottom);
	}

	virtual void GetInset(VPANEL vguiPanel, int& left, int& top, int& right, int& bottom)
	{
		((VPanel*)vguiPanel)->GetInset(left, top, right, bottom);
	}

	virtual void SetVisible(VPANEL vguiPanel, bool state)
	{
		((VPanel*)vguiPanel)->SetVisible(state);
	}

	virtual void SetEnabled(VPANEL vguiPanel, bool state)
	{
		((VPanel*)vguiPanel)->SetEnabled(state);
	}

	virtual bool IsVisible(VPANEL vguiPanel)
	{
		return ((VPanel*)vguiPanel)->IsVisible();
	}

	virtual bool IsEnabled(VPANEL vguiPanel)
	{
		return ((VPanel*)vguiPanel)->IsEnabled();
	}

	// Used by the drag/drop manager to always draw on top
	virtual bool IsTopmostPopup(VPANEL vguiPanel)
	{
		return ((VPanel*)vguiPanel)->IsTopmostPopup();
	}

	virtual void SetTopmostPopup(VPANEL vguiPanel, bool state)
	{
		return ((VPanel*)vguiPanel)->SetTopmostPopup(state);
	}

	virtual void SetParent(VPANEL vguiPanel, VPANEL newParent)
	{
		((VPanel*)vguiPanel)->SetParent((VPanel*)newParent);
	}

	virtual int GetChildCount(VPANEL vguiPanel)
	{
		return ((VPanel*)vguiPanel)->GetChildCount();
	}

	virtual VPANEL GetChild(VPANEL vguiPanel, int index)
	{
		return (VPANEL)((VPanel*)vguiPanel)->GetChild(index);
	}

	virtual CUtlVector< VPANEL >& GetChildren(VPANEL vguiPanel)
	{
		return (CUtlVector< VPANEL > &)((VPanel*)vguiPanel)->GetChildren();
	}

	virtual VPANEL GetParent(VPANEL vguiPanel)
	{
		return (VPANEL)((VPanel*)vguiPanel)->GetParent();
	}

	virtual void MoveToFront(VPANEL vguiPanel)
	{
		((VPanel*)vguiPanel)->MoveToFront();
	}

	virtual void MoveToBack(VPANEL vguiPanel)
	{
		((VPanel*)vguiPanel)->MoveToBack();
	}

	virtual bool HasParent(VPANEL vguiPanel, VPANEL potentialParent)
	{
		if (!vguiPanel)
			return false;

		return ((VPanel*)vguiPanel)->HasParent((VPanel*)potentialParent);
	}

	virtual bool IsPopup(VPANEL vguiPanel)
	{
		return ((VPanel*)vguiPanel)->IsPopup();
	}

	virtual void SetPopup(VPANEL vguiPanel, bool state)
	{
		((VPanel*)vguiPanel)->SetPopup(state);
	}

	virtual bool IsFullyVisible(VPANEL vguiPanel)
	{
		return ((VPanel*)vguiPanel)->IsFullyVisible();
	}

	// calculates the panels current position within the hierarchy
	virtual void Solve(VPANEL vguiPanel)
	{
		((VPanel*)vguiPanel)->Solve();
	}

	// used by ISurface to store platform-specific data
	virtual SurfacePlat* Plat(VPANEL vguiPanel)
	{
		return ((VPanel*)vguiPanel)->Plat();
	}

	virtual void SetPlat(VPANEL vguiPanel, SurfacePlat* Plat)
	{
		((VPanel*)vguiPanel)->SetPlat(Plat);
	}

	virtual const char* GetName(VPANEL vguiPanel)
	{
		return ((VPanel*)vguiPanel)->GetName();
	}

	virtual const char* GetClassName(VPANEL vguiPanel)
	{
		return ((VPanel*)vguiPanel)->GetClassName();
	}

	virtual HScheme GetScheme(VPANEL vguiPanel)
	{
		return ((VPanel*)vguiPanel)->GetScheme();
	}

	virtual bool IsProportional(VPANEL vguiPanel)
	{
		return Client(vguiPanel)->IsProportional();
	}

	virtual bool IsAutoDeleteSet(VPANEL vguiPanel)
	{
		return Client(vguiPanel)->IsAutoDeleteSet();
	}

	virtual void DeletePanel(VPANEL vguiPanel)
	{
		Client(vguiPanel)->DeletePanel();
	}

	virtual void SendMessage(VPANEL vguiPanel, KeyValues* params, VPANEL ifrompanel)
	{
		((VPanel*)vguiPanel)->SendMessage(params, ifrompanel);
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

	virtual Panel* GetPanel(VPANEL vguiPanel, const char* moduleName)
	{
		if (!vguiPanel)
			return NULL;

		if (vguiPanel == g_pVGui->GetEmbeddedPanel())
			return NULL;

		// assert that the specified vpanel is from the same module as requesting the cast
		if (!vguiPanel || V_stricmp(GetModuleName(vguiPanel), moduleName))
		{
			// assert(!("GetPanel() used to retrieve a Panel * from a different dll than which which it was created. This is bad, you can't pass Panel * across dll boundaries else you'll break the versioning.  Please only use a VPANEL."));
			// this is valid for now
			return NULL;
		}
		return Client(vguiPanel)->GetPanel();
	}

	virtual const char* GetModuleName(VPANEL vguiPanel)
	{
		return Client(vguiPanel)->GetModuleName();
	}

	virtual void SetKeyBoardInputEnabled(VPANEL vguiPanel, bool state)
	{
		((VPanel*)vguiPanel)->SetKeyBoardInputEnabled(state);
	}

	virtual void SetMouseInputEnabled(VPANEL vguiPanel, bool state)
	{
		((VPanel*)vguiPanel)->SetMouseInputEnabled(state);
	}

	virtual bool IsMouseInputEnabled(VPANEL vguiPanel)
	{
		return ((VPanel*)vguiPanel)->IsMouseInputEnabled();
	}

	virtual bool IsKeyBoardInputEnabled(VPANEL vguiPanel)
	{
		return ((VPanel*)vguiPanel)->IsKeyBoardInputEnabled();
	}

	virtual void SetSiblingPin(VPANEL vguiPanel, VPANEL newSibling, byte iMyCornerToPin = 0, byte iSiblingCornerToPinTo = 0)
	{
		return ((VPanel*)vguiPanel)->SetSiblingPin((VPanel*)newSibling, iMyCornerToPin, iSiblingCornerToPinTo);
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

	virtual void SolveTraverse(vgui::VPANEL panel, bool forceApplySchemeSettings);

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
		char	panelname[ 64 ];
	};

	Tick_t* CreateNewTick( VPANEL panel, int intervalMilliseconds );

	// Returns the current context
	Context_t *GetContext( HContext context );

	void PanelCreated(VPanel *panel);
	void PanelDeleted(VPanel *panel);
	bool DispatchMessages();
	void DestroyAllContexts( );
	void ClearMessageQueues();
	inline bool IsReentrant() const 
	{ 
		return m_nReentrancyCount > 0; 
	}

	// safe panel handle stuff
	CUtlHandleTable< VPanel, 20 > m_HandleTable;
	int m_iCurrentMessageID;

	bool m_bRunning : 1;
	bool m_bDoSleep : 1;
	bool m_InDispatcher : 1;
	bool m_bDebugMessages : 1;
	bool m_bVRMode : 1;
	bool m_bCanRemoveTickSignal : 1;
	int m_nReentrancyCount;

	CUtlVector< Tick_t * > m_TickSignalVec;
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
	CUtlVector<vgui::HPanel>	m_PopupList;

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

CVGui g_VGui;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CVGui, IVGui, VGUI_IVGUI_INTERFACE_VERSION, g_VGui);

bool IsDispatchingMessageQueue( void )
{
	return g_VGui.IsDispatchingMessages();
}

namespace vgui
{
IVGui *g_pIVgui = &g_VGui;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CVGui::CVGui() : m_pEmbeddedPanel(NULL), m_DelayedMessageQueue(0, 4, PriorityQueueComp)
{
	m_bRunning = false;
	m_InDispatcher = false;
	m_bDebugMessages = false;
	m_bDoSleep = true;
	m_bVRMode = false;
	m_bCanRemoveTickSignal = true;
	m_nReentrancyCount = 0;
	m_hContext = DEFAULT_VGUI_CONTEXT;
	m_DefaultContext.m_hInputContext = DEFAULT_INPUT_CONTEXT;
	m_bInThink = false;
	m_pRestrictedPanel = NULL;
	m_bNeedsKeyboard = true;
	m_bNeedsMouse = true;
	m_cursorAlwaysVisible = false;
	m_HWnd = NULL;
	m_bAppDrivesInput = false;
	m_nLastInputPollCount = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CVGui::~CVGui()
{
#ifdef _DEBUG
	int nCount = m_HandleTable.GetHandleCount();
	int nActualCount = 0;
	for ( int i = 0; i < nCount; ++i )
	{
		UtlHandle_t h = m_HandleTable.GetHandleFromIndex( i );
		if ( m_HandleTable.IsHandleValid( h ) )
		{
			++nActualCount;
		}
	}

	if ( nActualCount > 0 )
	{
		Msg("Memory leak: panels left in CVGui::m_PanelList: %d\n", nActualCount );
	}
#endif // _DEBUG
}

//-----------------------------------------------------------------------------
// Purpose: Dumps out list of all active panels
//-----------------------------------------------------------------------------
void CVGui::SpewAllActivePanelNames()
{
	int nCount = m_HandleTable.GetHandleCount();
	for ( int i = 0; i < nCount; ++i )
	{
		UtlHandle_t h = m_HandleTable.GetHandleFromIndex( i );
		if ( m_HandleTable.IsHandleValid( h ) )
		{
			VPanel *pPanel = m_HandleTable.GetHandle( h );
			Msg("\tpanel '%s' of type '%s' leaked\n", ((VPanel*)pPanel)->GetName(), ((VPanel *)pPanel)->GetClassName());
		}
	}
}


//-----------------------------------------------------------------------------
// Creates/ destroys "input" contexts, which contains information
// about which controls have mouse + key focus, for example.
//-----------------------------------------------------------------------------
HContext CVGui::CreateContext()
{
	HContext i = m_Contexts.AddToTail();
	m_Contexts[i].m_hInputContext = g_pInput->CreateInputContext();
	return i;
}

void CVGui::DestroyContext( HContext context )
{
	Assert( context != DEFAULT_VGUI_CONTEXT );

	if ( m_hContext == context )
	{
		ActivateContext( DEFAULT_VGUI_CONTEXT );
	}

	g_pInput->DestroyInputContext( GetContext(context)->m_hInputContext );
	m_Contexts.Remove(context);
}

void CVGui::DestroyAllContexts( )
{
	HContext next;
	HContext i = m_Contexts.Head();
	while (i != m_Contexts.InvalidIndex())
	{
		next = m_Contexts.Next(i);
		DestroyContext( i );
		i = next;
	}
}


//-----------------------------------------------------------------------------
// Returns the current context
//-----------------------------------------------------------------------------
CVGui::Context_t *CVGui::GetContext( HContext context )
{
	if (context == DEFAULT_VGUI_CONTEXT)
		return &m_DefaultContext;
	return &m_Contexts[context];
}


//-----------------------------------------------------------------------------
// Associates a particular panel with a context
// Associating NULL is valid; it disconnects the panel from the context
//-----------------------------------------------------------------------------
void CVGui::AssociatePanelWithContext( HContext context, VPANEL pRoot )
{
	Assert( context != DEFAULT_VGUI_CONTEXT );
	g_pInput->AssociatePanelWithInputContext( GetContext(context)->m_hInputContext, pRoot );
}


//-----------------------------------------------------------------------------
// Activates a particular context, use DEFAULT_VGUI_CONTEXT
// to get the one normally used by VGUI
//-----------------------------------------------------------------------------
void CVGui::ActivateContext( HContext context )
{
	Assert( (context == DEFAULT_VGUI_CONTEXT) || m_Contexts.IsValidIndex(context) );

	if ( m_hContext != context )
	{
		// Clear out any messages queues that may be full...
		if ( !IsReentrant() )
		{
			DispatchMessages();
		}

		m_hContext = context;
		g_pInput->ActivateInputContext( GetContext(m_hContext)->m_hInputContext ); 

		if ( context != DEFAULT_VGUI_CONTEXT && !IsReentrant() )
		{
			g_pInput->RunFrame( );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Runs a single vgui frame, pumping all message to panels
//-----------------------------------------------------------------------------
void CVGui::RunFrame() 
{
	// NOTE: This can happen when running in Maya waiting for modal dialogs
	bool bIsReentrant = m_InDispatcher;
	if ( bIsReentrant )
	{
		++m_nReentrancyCount;
	}

#ifdef DEBUG
//  memory allocation debug helper
//	DPrintf( "Delete Count:%i,%i\n", m_iDeleteCount, m_iDeletePanelCount );
//	m_iDeleteCount = 	m_iDeletePanelCount = 0;
#endif

	// this will generate all key and mouse events as well as make a real repaint
	{
		VPROF( "surface()->RunFrame()" );
		//g_pSurface->RunFrame();
		int nPollCount = g_pInputSystem->GetPollCount();
		if (m_nLastInputPollCount != nPollCount) {

			// If this isn't true, we've lost input!
			if (!m_bAppDrivesInput && m_nLastInputPollCount != nPollCount - 1)
			{
				Assert(0);
				Warning("Vgui is losing input messages! Call brian!\n");
			}

			m_nLastInputPollCount = nPollCount;

			if (!m_bAppDrivesInput) {

				// Generate all input messages
				int nEventCount = g_pInputSystem->GetEventCount();
				const InputEvent_t* pEvents = g_pInputSystem->GetEventData();
				for (int i = 0; i < nEventCount; ++i)
				{
					HandleInputEvent(pEvents[i]);
				}
			}
		}
	}

	// give the system a chance to process
	{
		VPROF( "system()->RunFrame()" );
		g_pSystem->RunFrame();
	}

	// update cursor positions
	if ( IsPC() && !IsReentrant() )
	{
		VPROF( "update cursor positions" );
		int cursorX, cursorY;
		g_pInput->GetCursorPosition(cursorX, cursorY);

		// this does the actual work given a x,y and a surface
		g_pInput->UpdateMouseFocus(cursorX, cursorY);

	}

	if ( !bIsReentrant )
	{
		VPROF( "input()->RunFrame()" );
		g_pInput->RunFrame();
	}

	// messenging
	if ( !bIsReentrant )
	{
		VPROF( "messaging" );

		// send all the messages waiting in the queue
		DispatchMessages();

		// Do the OnTicks before purging messages, since in previous code they were posted after dispatch and wouldn't hit
		//  until next frame
		int time = g_pSystem->GetTimeMillis();

		m_bCanRemoveTickSignal = false;

		tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "%s - Ticks", __FUNCTION__ );
		// directly invoke tick all who asked to be ticked
		int count = m_TickSignalVec.Count();
		for (int i = count - 1; i >= 0; i-- )
		{
			Tick_t *t = m_TickSignalVec[i];
			if ( t->bMarkDeleted )
				continue;

			if ( t->interval != 0 )
			{
				if ( time < t->nexttick )
					continue;

				t->nexttick = time + t->interval;
			}
			
			g_pIVgui->Client(t->panel)->OnTick();
			tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "%s - Ticks: %s", __FUNCTION__, t->panel->Client()->GetName() );
		}

		m_bCanRemoveTickSignal = true;

		// get count again. panels could be added to tick vector in OnTick
		count = m_TickSignalVec.Count();

		// Remove all panels that tried to remove tick in OnTick
		for (int i = count - 1; i >= 0; i-- )
		{
			Tick_t *t = m_TickSignalVec[i];
			if ( t->bMarkDeleted )
			{
				m_TickSignalVec.Remove( i );
				delete t;
			}
		}
	}

	{
		VPROF( "SolveTraverse" );
		// make sure the hierarchy is up to date
		g_pIVgui->SolveTraverse(g_pVGui->GetEmbeddedPanel());
		g_pIVgui->ApplyChanges();
#ifdef WIN32
		Assert( IsX360() || ( IsPC() && _heapchk() == _HEAPOK ) );
#endif
	}

	if ( bIsReentrant )
	{
		--m_nReentrancyCount;
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
VPANEL CVGui::AllocPanel()
{
#ifdef DEBUG
	m_iDeleteCount++;
#endif

	VPanel *panel = new VPanel;
	PanelCreated(panel);
	return (VPANEL)panel;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CVGui::FreePanel(VPANEL ipanel)
{
	PanelDeleted((VPanel *)ipanel);
	delete (VPanel *)ipanel;
#ifdef DEBUG
	m_iDeleteCount--;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Returns the safe index of the panel
//-----------------------------------------------------------------------------
HPanel CVGui::PanelToHandle(VPANEL panel)
{
	if (panel)
		return ((VPanel*)panel)->GetHPanel();
	return INVALID_PANEL;
}


//-----------------------------------------------------------------------------
// Purpose: Returns the panel at the specified index
//-----------------------------------------------------------------------------
VPANEL CVGui::HandleToPanel(HPanel index)
{
	if ( !m_HandleTable.IsHandleValid( index ) )
	{
		return NULL;
	}
	return (VPANEL)m_HandleTable.GetHandle( (UtlHandle_t)index );
}


//-----------------------------------------------------------------------------
// Purpose: Called whenever a panel is constructed
//-----------------------------------------------------------------------------
void CVGui::PanelCreated(VPanel *panel)
{
	UtlHandle_t h = m_HandleTable.AddHandle();
	m_HandleTable.SetHandle( h, panel );

#if DUMP_PANEL_LIST
	int nCount = m_HandleTable.GetHandleCount();
	int nActualCount = 0;
	for ( int i = 0; i < nCount; ++i )
	{
		UtlHandle_t h = m_HandleTable.GetHandleFromIndex( i );
		if ( m_HandleTable.IsHandleValid( h ) )
		{
			++nActualCount;
		}
	}

	if ( nActualCount >= WARN_PANEL_NUMBER )
	{
		FILE *file1 = fopen("panellist.txt", "w");
		if (file1 != NULL)
		{
			fprintf(file1, "Too many panels...listing them all.\n");
			int panelIndex;
			for (panelIndex = 0; panelIndex < nCount; ++panelIndex)
			{
				UtlHandle_t h = m_HandleTable.GetHandleFromIndex( i );
				VPanel *pPanel = m_HandleTable.GetHandle( h );
				IClientPanel *ipanel = ( pPanel ) ? pPanel->Client() : NULL;
				if ( ipanel )
					fprintf(file1, "panel %d: name: %s   classname: %s\n", panelIndex, ipanel->GetName(), ipanel->GetClassName());
				else
					fprintf(file1, "panel %d: can't get ipanel\n", panelIndex);
			}

			fclose(file1);
		}
	}

	Assert( nActualCount < WARN_PANEL_NUMBER );
#endif // DUMP_PANEL_LIST

	panel->SetHPanel( h );

	g_pVGui->AddPanel((VPANEL)panel);
}

//-----------------------------------------------------------------------------
// Purpose: instantly stops the app from pointing to the focus'd object
//			used when an object is being deleted
//-----------------------------------------------------------------------------
void CVGui::PanelDeleted(VPanel *focus)
{
	Assert( focus );
	g_pVGui->ReleasePanel((VPANEL)focus);
	g_pInput->PanelDeleted((VPANEL)focus);

	// remove from safe handle list
	UtlHandle_t h = ((VPanel *)focus)->GetHPanel();

	Assert( m_HandleTable.IsHandleValid(h) );
	if ( m_HandleTable.IsHandleValid(h) )
	{
		m_HandleTable.RemoveHandle( h );
	}

	focus->SetHPanel( INVALID_PANEL );

	// remove from tick signal dar
	RemoveTickSignal( (VPANEL)focus );
}

//-----------------------------------------------------------------------------
// Purpose: Creates or updates a tick signal for a panel.  Returns NULL if already ticking.
//-----------------------------------------------------------------------------
CVGui::Tick_t* CVGui::CreateNewTick( VPANEL panel, int intervalMilliseconds )
{
	Tick_t *t;
	// See if it's already in list
	int count = m_TickSignalVec.Count();
	for (int i = 0; i < count; i++ )
	{
		Tick_t *t = m_TickSignalVec[i];
		if ( t->panel == panel )
		{
			// Go ahead and update intervals
			t->interval = intervalMilliseconds;
			t->nexttick = g_pSystem->GetTimeMillis() + t->interval;

			// Somebody added this panel back to the tick list, don't delete it
			t->bMarkDeleted = false;
			return NULL;
		}
	}

	// Add to list
	t = new Tick_t;

	t->panel = panel;
	t->interval = intervalMilliseconds;
	t->nexttick = g_pSystem->GetTimeMillis() + t->interval;
	t->bMarkDeleted = false;

	if ( strlen( g_pIVgui->Client(panel)->GetName() ) > 0 )
	{
		strncpy( t->panelname, g_pIVgui->Client(panel)->GetName(), sizeof( t->panelname ) );
	}
	else
	{
		strncpy( t->panelname, g_pIVgui->Client(panel)->GetClassName(), sizeof( t->panelname ) );
	}

	return t;
}

//-----------------------------------------------------------------------------
// Purpose: Adds the panel to the tail of a tick signal list, so the panel receives a message every frame
//-----------------------------------------------------------------------------
void CVGui::AddTickSignal(VPANEL panel, int intervalMilliseconds /*=0*/ )
{
	Tick_t* t = CreateNewTick( panel, intervalMilliseconds );

	if ( t )
	{
		// add the element to the end list 
		m_TickSignalVec.AddToTail( t );
		// panel is removed from list when deleted
	}
}

//-----------------------------------------------------------------------------
// Purpose: Adds the panel to the head of a tick signal list, so the panel receives a message every frame
//-----------------------------------------------------------------------------
void CVGui::AddTickSignalToHead(VPANEL panel, int intervalMilliseconds /*=0*/ )
{
	Tick_t* t = CreateNewTick( panel, intervalMilliseconds );

	if ( t )
	{
		// simply add the element to the head list 
		m_TickSignalVec.AddToHead( t );
		// panel is removed from list when deleted
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CVGui::RemoveTickSignal( VPANEL panel )
{
	VPANEL search = panel;

	// remove from tick signal dar
	int count = m_TickSignalVec.Count();

	for (int i = 0; i < count; i++ )
	{
		Tick_t *tick = m_TickSignalVec[i];
		if ( tick->panel == search )
		{
			if ( m_bCanRemoveTickSignal )
			{
				m_TickSignalVec.Remove( i );
				delete tick;
			}
			else
			{
				tick->bMarkDeleted = true;
			}
			
			return;
		}
	}
}



//-----------------------------------------------------------------------------
// Purpose: message pump
//			loops through and sends all active messages
//			note that more messages may be posted during the process
//-----------------------------------------------------------------------------
bool CVGui::DispatchMessages()
{
	int time = g_pSystem->GetTimeMillis();

	m_InDispatcher = true;
	bool doneWork = (m_MessageQueue.Count() > 12);

	bool bUsingDelayedQueue = (m_DelayedMessageQueue.Count() > 0);

	// Need two passes because we send the mouse move message after all
	// other messages are done, but the mouse move message may itself generate
	// some more messages
	int nPassCount = 0;
	while ( nPassCount < 2 )
	{
		while (m_MessageQueue.Count() > 0 || (m_SecondaryQueue.Count() > 0) || bUsingDelayedQueue)
		{
			// get the first message
			MessageItem_t *messageItem = NULL;
			int messageIndex = 0;

			// use the secondary queue until it empties. empty it after each message in the
			// primary queue. this makes primary messages completely resolve 
			bool bUsingSecondaryQueue = (m_SecondaryQueue.Count() > 0);
			if (bUsingSecondaryQueue)
			{
				doneWork = true;
				messageIndex = m_SecondaryQueue.Head();
				messageItem = &m_SecondaryQueue[messageIndex];
			}
			else if (bUsingDelayedQueue)
			{
				if (m_DelayedMessageQueue.Count() >0)
				{
					messageItem = (MessageItem_t*)&m_DelayedMessageQueue.ElementAtHead();
				}
				if (!messageItem || messageItem->_arrivalTime > time)
				{
					// no more items in the delayed message queue, move to the system queue
					bUsingDelayedQueue = false;
					continue;
				}
			}
			else
			{
				messageIndex = m_MessageQueue.Head();
				messageItem = &m_MessageQueue[messageIndex];
			}

			// message debug code 

			if ( m_bDebugMessages )
			{
				const char *qname = bUsingSecondaryQueue ? "Secondary" : "Primary";

				if (strcmp(messageItem->_params->GetName(), "Tick")
					&& strcmp(messageItem->_params->GetName(), "MouseFocusTicked") 
					&& strcmp(messageItem->_params->GetName(), "KeyFocusTicked")
					&& strcmp(messageItem->_params->GetName(), "CursorMoved"))
				{
					if (!stricmp(messageItem->_params->GetName(), "command"))
					{
						g_pIVgui->DPrintf2( "%s Queue dispatching command( %s, %s -- %i )\n", qname, messageItem->_params->GetName(), messageItem->_params->GetString("command"), messageItem->_messageID );
					}
					else
					{
						g_pIVgui->DPrintf2( "%s Queue dispatching( %s -- %i )\n", qname ,messageItem->_params->GetName(), messageItem->_messageID );
					}
				}
			}

			// send it
			KeyValues *params = messageItem->_params;

			// Deal with special internal cursor movement messages
			if ( messageItem->_messageTo == 0xFFFFFFFF )
			{
				if ( !Q_stricmp( params->GetName(), "SetCursorPosInternal" ) )
				{
					int nXPos = params->GetInt( "xpos", 0 );
					int nYPos = params->GetInt( "ypos", 0 );
					g_pInput->UpdateCursorPosInternal( nXPos, nYPos );
				}
			}
#ifdef _X360
			else if ( messageItem->_messageTo == 0xFFFFFFFE ) // special tag to always give message to the active key focus
			{
				VPanel *vto = (VPanel *) g_pInput->GetCalculatedFocus();
				if (vto)
				{
					vto->SendMessage(params, g_pIVgui->HandleToPanel(messageItem->_from));
				}
			}
#endif
			else
			{
				VPanel *vto = (VPanel *)g_pIVgui->HandleToPanel(messageItem->_messageTo);
				if (vto)
				{
					//			Msg("Sending message: %s to %s\n", params ? params->GetName() : "\"\"", vto->GetName() ? vto->GetName() : "\"\"");
					vto->SendMessage(params, g_pIVgui->HandleToPanel(messageItem->_from));
				}
			}

			// free the keyvalues memory
			// we can't reference the messageItem pointer anymore since the queue might have moved in memory
			if (params)
			{
				params->deleteThis();
			}

			// remove it from the queue
			if (bUsingSecondaryQueue)
			{
				m_SecondaryQueue.Remove(messageIndex);
			}
			else if (bUsingDelayedQueue)
			{
				m_DelayedMessageQueue.RemoveAtHead();
			}
			else
			{
				m_MessageQueue.Remove(messageIndex);
			}
		}

		++nPassCount;
		if ( nPassCount == 1 )
		{
			// Specifically post the current mouse position as a message
			g_pInput->PostCursorMessage();
		}
	}

	// Make sure the windows cursor is in the right place after processing input 
	// Needs to be done here because a message provoked by the cursor moved
	// message may move the cursor also
	g_pInput->HandleExplicitSetCursor( );

	m_InDispatcher = false;
	return doneWork;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CVGui::MarkPanelForDeletion(VPANEL panel)
{
	PostMessage(panel, new KeyValues("Delete"), NULL);
}

//-----------------------------------------------------------------------------
// Purpose: Adds a message to the queue to be sent to a user
//-----------------------------------------------------------------------------
void CVGui::PostMessage(VPANEL target, KeyValues *params, VPANEL from, float delay)
{
	// Ignore all messages in re-entrant mode
	if ( IsReentrant() )
	{
		Assert( 0 );
		if (params)
		{
			params->deleteThis();
		}
		return;
	}

	if (!target)
	{
		if (params)
		{
			params->deleteThis();
		}
		return;
	}

	MessageItem_t messageItem;
	 
#ifdef _X360
	// Special coded target that will always send the message to the key focus
	// this is needed since we might send two messages on a tice, and the first
	// could change the focus.
	if( target == (VPANEL) MESSAGE_CURRENT_KEYFOCUS )
	{
		messageItem._messageTo = 0xFFFFFFFE;
	}
	else
#endif	
	{
		messageItem._messageTo = (target != (VPANEL) MESSAGE_CURSOR_POS ) ? g_pIVgui->PanelToHandle(target) : 0xFFFFFFFF;
	}
	messageItem._params = params;
	Assert(params->GetName());
	messageItem._from = g_pIVgui->PanelToHandle(from);
	messageItem._arrivalTime = 0;
	messageItem._messageID = m_iCurrentMessageID++;

	/* message debug code
	//if ( stricmp(messageItem._params->GetName(),"CursorMoved") && stricmp(messageItem._params->GetName(),"KeyFocusTicked"))
	{
		g_pIVgui->DPrintf2( "posting( %s -- %i )\n", messageItem._params->GetName(), messageItem._messageID );
	}
	*/
				
	// add the message to the correct message queue
	if (delay > 0.0f)
	{
		messageItem._arrivalTime = g_pSystem->GetTimeMillis() + (delay * 1000);
		m_DelayedMessageQueue.Insert(messageItem);
	}
	else if (m_InDispatcher)
	{
		m_SecondaryQueue.AddToTail(messageItem);
	}
	else
	{
		m_MessageQueue.AddToTail(messageItem);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CVGui::ShutdownMessage(unsigned int shutdownID)
{
	// broadcast Shutdown to all the top level windows, and see if any take notice
	VPANEL panel = g_pVGui->GetEmbeddedPanel();
	for (int i = 0; i < g_pVGui->GetChildCount(panel); i++)
	{
		g_pIVgui->PostMessage(g_pVGui->GetChild(panel, i), new KeyValues("ShutdownRequest", "id", shutdownID), NULL);
	}

	// post to the top level window as well
	g_pIVgui->PostMessage(panel, new KeyValues("ShutdownRequest", "id", shutdownID), NULL);
}

//-----------------------------------------------------------------------------
// Purpose: Clears all the memory queues and free's their memory
//-----------------------------------------------------------------------------
void CVGui::ClearMessageQueues()
{
	Assert(!m_InDispatcher);

	{FOR_EACH_LL( m_MessageQueue, i )
	{
		if (m_MessageQueue[i]._params)
		{
			m_MessageQueue[i]._params->deleteThis();
		}
	}}
	m_MessageQueue.RemoveAll();

	// secondary message queue, holds all vgui messages generated by vgui
	{FOR_EACH_LL( m_SecondaryQueue, i )
	{
		if (m_SecondaryQueue[i]._params)
		{
			m_SecondaryQueue[i]._params->deleteThis();
		}
	}}
	m_SecondaryQueue.RemoveAll();

	// timing queue, holds all the messages that have to arrive at a specified time
	while (m_DelayedMessageQueue.Count() > 0)
	{
		if (m_DelayedMessageQueue.ElementAtHead()._params)
		{
			m_DelayedMessageQueue.ElementAtHead()._params->deleteThis();
		}
		m_DelayedMessageQueue.RemoveAtHead();
	}
}

/*
static void*(*staticMalloc)(size_t size)=malloc;
static void(*staticFree)(void* memblock)=free;

static int g_iMemoryBlocksAllocated = 0;

void *operator new(size_t size)
{
	g_iMemoryBlocksAllocated += 1;
	return staticMalloc(size);
}

void operator delete(void* memblock)
{
	if (!memblock)
		return;

	g_iMemoryBlocksAllocated -= 1;

	if (g_iMemoryBlocksAllocated < 0)
	{
		int x = 3;
	}

	staticFree(memblock);
}

void *operator new [] (size_t size)
{
	return staticMalloc(size);
}

void operator delete [] (void *pMem)
{
	staticFree(pMem);
}
*/

void CVGui::DPrintf(const char* format,...)
{
	char    buf[2048];
	va_list argList;

	va_start(argList,format);
	Q_vsnprintf(buf,sizeof( buf ), format,argList);
	va_end(argList);

#ifdef WIN32
	::OutputDebugString(buf);
#else
	Msg( "%s", buf );
#endif
}

void CVGui::DPrintf2(const char* format,...)
{
	char    buf[2048];
	va_list argList;
	static int ctr=0;

	Q_snprintf(buf,sizeof( buf ), "%d:",ctr++ );

	va_start(argList,format);
	Q_vsnprintf(buf+strlen(buf),sizeof( buf )-strlen(buf),format,argList);
	va_end(argList);

#ifdef WIN32
	::OutputDebugString(buf);
#else
	Msg( "%s", buf );
#endif
}

void vgui::vgui_strcpy(char* dst,int dstLen,const char* src)
{
	Assert(dst!=null);
	Assert(dstLen>=0);
	Assert(src!=null);

	int srcLen=strlen(src)+1;
	if(srcLen>dstLen)
	{
		srcLen=dstLen;
	}

	memcpy(dst,src,srcLen-1);
	dst[srcLen-1]=0;
}

//-----------------------------------------------------------------------------
	// HL2/TFC specific stuff
//-----------------------------------------------------------------------------
// Here's where the app systems get to learn about each other 
//-----------------------------------------------------------------------------
bool CVGui::Connect( CreateInterfaceFn factory )
{
	if ( !BaseClass::Connect( factory ) )
		return false;

	if ( !g_pFullFileSystem || !g_pVGuiLocalize )
	{
		Warning( "IVGui unable to connect to required interfaces!\n" );
		return false;
	}

	return VGui_InternalLoadInterfaces( &factory, 1 );
}

void CVGui::Disconnect()
{
	// FIXME: Blat out interface pointers
	BaseClass::Disconnect();
}


//-----------------------------------------------------------------------------
// Init, shutdown
//-----------------------------------------------------------------------------
InitReturnVal_t CVGui::Init()
{
	m_hContext = DEFAULT_VGUI_CONTEXT;
	m_bDebugMessages = CommandLine()->FindParm( "-vguimessages" ) ? true : false;

	InitReturnVal_t nRetVal = BaseClass::Init();
	if ( nRetVal != INIT_OK )
		return nRetVal;

	g_bSpewFocus = CommandLine()->FindParm("-vguifocus") ? true : false;

	m_pDefaultEmbeddedPanel = new CMatEmbeddedPanel;
	SetEmbeddedPanel(m_pDefaultEmbeddedPanel->GetVPanel());

	// Input system
	InitInput();

	// Initialize cursors
	InitCursors();

	return INIT_OK;
}

void CVGui::Shutdown()
{
	for (int i = m_FileTypeImages.First(); i != m_FileTypeImages.InvalidIndex(); i = m_FileTypeImages.Next(i))
	{
		delete m_FileTypeImages[i];
	}
	m_FileTypeImages.RemoveAll();

	g_pSystem->SaveUserConfigFile();

	DestroyAllContexts();
	ClearMessageQueues();

	g_pSystem->Shutdown();
	g_pScheme->Shutdown(true);

	if ( !g_pSurface->QueryInterface( MAT_SYSTEM_SURFACE_INTERFACE_VERSION ) )
	{
		g_pSurface->Shutdown();
	}

	m_Titles.Purge();

	Cursor_ClearUserCursors();

	BaseClass::Shutdown();
}

//-----------------------------------------------------------------------------
// Here's where systems can access other interfaces implemented by this object
// Returns NULL if it doesn't implement the requested interface
//-----------------------------------------------------------------------------
void *CVGui::QueryInterface( const char *pInterfaceName )
{
	// FIXME: Should this go here?
	// Access other global interfaces exposed by this system...
	CreateInterfaceFn vguiFactory = Sys_GetFactoryThis();
	return vguiFactory( pInterfaceName, NULL );
}

void CVGui::CreatePopup(VPANEL panel, bool minimized, bool showTaskbarIcon, bool disabled, bool mouseInput, bool kbInput)
{
	if (!g_pVGui->GetParent(panel))
	{
		g_pVGui->SetParent(panel, g_pVGui->GetEmbeddedPanel());
	}
	g_pVGui->SetPopup(panel, true);
	g_pVGui->SetKeyBoardInputEnabled(panel, kbInput);
	g_pVGui->SetMouseInputEnabled(panel, mouseInput);

	HPanel p = g_pIVgui->PanelToHandle(panel);

	if (m_PopupList.Find(p) == m_PopupList.InvalidIndex())
	{
		m_PopupList.AddToTail(p);
	}
	else
	{
		MovePopupToFront(panel);
	}
}

void CVGui::MovePopupToFront(VPANEL panel)
{
	HPanel p = g_pIVgui->PanelToHandle(panel);

	int index = m_PopupList.Find(p);
	if (index == m_PopupList.InvalidIndex())
		return;

	m_PopupList.Remove(index);
	m_PopupList.AddToTail(p);

	if (g_bSpewFocus)
	{
		char const* pName = g_pIVgui->GetName(panel);
		Msg("%s moved to front\n", pName ? pName : "(no name)");
	}

	// If the modal panel isn't a parent, restore it to the top, to prevent a hard lock
	if (g_pInput->GetAppModalSurface())
	{
		if (!g_pVGui->HasParent(panel, g_pInput->GetAppModalSurface()))
		{
			HPanel p = g_pIVgui->PanelToHandle(g_pInput->GetAppModalSurface());
			index = m_PopupList.Find(p);
			if (index != m_PopupList.InvalidIndex())
			{
				m_PopupList.Remove(index);
				m_PopupList.AddToTail(p);
			}
		}
	}

	g_pIVgui->PostMessage(panel, new KeyValues("OnMovedPopupToFront"), NULL);
}

void CVGui::MovePopupToBack(VPANEL panel)
{
	HPanel p = g_pVGui->PanelToHandle(panel);

	int index = m_PopupList.Find(p);
	if (index == m_PopupList.InvalidIndex())
	{
		return;
	}

	m_PopupList.Remove(index);
	m_PopupList.AddToHead(p);
}

void CVGui::RemovePopup(vgui::VPANEL panel)
{
	// Remove from popup list if needed and remove any dead popups while we're at it
	int c = GetPopupCount();

	for (int i = c - 1; i >= 0; i--)
	{
		VPANEL popup = GetPopup(i);
		if (popup && (popup != panel))
			continue;

		m_PopupList.Remove(i);
		break;
	}
}

int CVGui::GetPopupCount()
{
	return m_PopupList.Count();
}

VPANEL CVGui::GetPopup(int index)
{
	HPanel p = m_PopupList[index];
	VPANEL panel = g_pVGui->HandleToPanel(p);
	return panel;
}

void CVGui::ResetPopupList()
{
	m_PopupList.RemoveAll();
}

void CVGui::AddPopup(VPANEL panel)
{
	HPanel p = g_pVGui->PanelToHandle(panel);

	if (m_PopupList.Find(p) == m_PopupList.InvalidIndex())
	{
		m_PopupList.AddToTail(p);
	}
}

void CVGui::AddPopupsToList(VPANEL panel)
{
	if (!g_pVGui->IsVisible(panel))
		return;

	// Add to popup list as we visit popups
	// Note:  popup list is cleared in RunFrame which occurs before this call!!!
	if (g_pVGui->IsPopup(panel))
	{
		AddPopup(panel);
	}

	int count = g_pVGui->GetChildCount(panel);
	for (int i = 0; i < count; ++i)
	{
		VPANEL child = g_pVGui->GetChild(panel, i);
		AddPopupsToList(child);
	}
}

void CVGui::ReleasePanel(VPANEL panel)
{
	// Remove from popup list if needed and remove any dead popups while we're at it
	RemovePopup(panel);

	int entry = GetTitleEntry(panel);
	if (entry != -1)
	{
		m_Titles.Remove(entry);
	}
}

int CVGui::GetTitleEntry(vgui::VPANEL panel)
{
	for (int i = 0; i < m_Titles.Count(); i++)
	{
		TitleEntry* entry = &m_Titles[i];
		if (entry->panel == panel)
			return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CVGui::SetTitle(VPANEL panel, const wchar_t* title)
{
	int entry = GetTitleEntry(panel);
	if (entry == -1)
	{
		entry = m_Titles.AddToTail();
	}

	TitleEntry* e = &m_Titles[entry];
	Assert(e);
	wcsncpy(e->title, title, sizeof(e->title) / sizeof(wchar_t));
	e->panel = panel;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
wchar_t const* CVGui::GetTitle(VPANEL panel)
{
	int entry = GetTitleEntry(panel);
	if (entry != -1)
	{
		TitleEntry* e = &m_Titles[entry];
		return e->title;
	}

	return NULL;
}

void CVGui::BringToFront(VPANEL panel)
{
	// move panel to top of list
	g_pVGui->MoveToFront(panel);

	// move panel to top of popup list
	if (g_pVGui->IsPopup(panel))
	{
		MovePopupToFront(panel);
	}
}

void CVGui::AddPanel(VPANEL panel)
{
	if (g_pVGui->IsPopup(panel))
	{
		// turn it into a popup menu
		CreatePopup(panel, false);
	}
}

void CVGui::SetForegroundWindow(VPANEL panel)
{
	BringToFront(panel);
}

void CVGui::SetTopLevelFocus(VPANEL pSubFocus)
{
	// walk up the hierarchy until we find what popup panel belongs to
	while (pSubFocus)
	{
		if (g_pVGui->IsPopup(pSubFocus) && g_pVGui->IsMouseInputEnabled(pSubFocus))
		{
			BringToFront(pSubFocus);
			break;
		}

		pSubFocus = g_pVGui->GetParent(pSubFocus);
	}
}

void CVGui::SetPanelVisible(VPANEL panel, bool state)
{
}

void CVGui::SetMinimized(VPANEL panel, bool state)
{
	if (state)
	{
		g_pVGui->SetPlat(panel, VPANEL_MINIMIZED);
		g_pVGui->SetVisible(panel, false);
	}
	else
	{
		g_pVGui->SetPlat(panel, VPANEL_NORMAL);
	}
}

bool CVGui::IsMinimized(vgui::VPANEL panel)
{
	return (g_pVGui->Plat(panel) == VPANEL_MINIMIZED);

}

void CVGui::FlashWindow(VPANEL panel, bool state)
{
}

void CVGui::SwapBuffers(VPANEL panel)
{
}

void CVGui::Invalidate(VPANEL panel)
{
}

void CVGui::ApplyChanges()
{
}

bool CVGui::IsWithin(int x, int y)
{
	return true;
}

//-----------------------------------------------------------------------------
// Focus-related methods
//-----------------------------------------------------------------------------
bool CVGui::HasFocus()
{
	return true;
}

bool CVGui::RecreateContext(VPANEL panel)
{
	return false;
}

// notify icons?!?
VPANEL CVGui::GetNotifyPanel()
{
	return NULL;
}

void CVGui::SetNotifyIcon(VPANEL context, HTexture icon, VPANEL panelToReceiveMessages, const char* text)
{
}

//-----------------------------------------------------------------------------
// A bunch of methods needed for the windows version only
//-----------------------------------------------------------------------------
void CVGui::SetAsTopMost(VPANEL panel, bool state)
{
}

void CVGui::SetAsToolBar(VPANEL panel, bool state)		// removes the window's task bar entry (for context menu's, etc.)
{
}

//-----------------------------------------------------------------------------
// Purpose: Walks through the panel tree calling Solve() on them all, in order
//-----------------------------------------------------------------------------
void CVGui::SolveTraverse(VPANEL panel, bool forceApplySchemeSettings)
{
	{
		VPROF("InternalSchemeSettingsTraverse");
		tmZone(TELEMETRY_LEVEL1, TMZF_NONE, "%s - InternalSchemeSettingsTraverse", __FUNCTION__);
		InternalSchemeSettingsTraverse(panel, forceApplySchemeSettings);
	}

	{
		VPROF("InternalThinkTraverse");
		tmZone(TELEMETRY_LEVEL1, TMZF_NONE, "%s - InternalThinkTraverse", __FUNCTION__);
		InternalThinkTraverse(panel);
	}

	{
		VPROF("InternalSolveTraverse");
		tmZone(TELEMETRY_LEVEL1, TMZF_NONE, "%s - InternalSolveTraverse", __FUNCTION__);
		InternalSolveTraverse(panel);
	}
}

//-----------------------------------------------------------------------------
// Purpose: recurses the panels giving them a chance to do apply settings,
//-----------------------------------------------------------------------------
void CVGui::InternalSchemeSettingsTraverse(VPANEL panel, bool forceApplySchemeSettings)
{
	VPanel* RESTRICT vp = (VPanel*)panel;

	vp->TraverseLevel(1);
	tmZone(TELEMETRY_LEVEL1, TMZF_NONE, "%s - %s", __FUNCTION__, vp->GetName());

	CUtlVector< VPanel* >& children = vp->GetChildren();

	// apply to the children...
	for (int i = 0; i < children.Count(); ++i)
	{
		VPanel* child = children[i];
		if (forceApplySchemeSettings || child->IsVisible())
		{
			InternalSchemeSettingsTraverse((VPANEL)child, forceApplySchemeSettings);
		}
	}
	// and then the parent
	vp->Client()->PerformApplySchemeSettings();

	vp->TraverseLevel(-1);
}

//-----------------------------------------------------------------------------
// Purpose: recurses the panels giving them a chance to do a user-defined think,
//			PerformLayout and ApplySchemeSettings
//			must be done child before parent
//-----------------------------------------------------------------------------
void CVGui::InternalThinkTraverse(VPANEL panel)
{
	VPanel* RESTRICT vp = (VPanel*)panel;

	vp->TraverseLevel(1);
	tmZone(TELEMETRY_LEVEL1, TMZF_NONE, "%s - %s", __FUNCTION__, vp->GetName());

	// think the parent
	vp->Client()->Think();

	CUtlVector< VPanel* >& children = vp->GetChildren();

	// WARNING: Some of the think functions add/remove children, so make sure we
	//  explicitly check for children.Count().
	for (int i = 0; i < children.Count(); ++i)
	{
		VPanel* child = children[i];
		if (child->IsVisible())
		{
			InternalThinkTraverse((VPANEL)child);
		}
	}

	vp->TraverseLevel(-1);
}

void CVGui::InternalSolveTraverse(VPANEL panel)
{
	VPanel * RESTRICT vp = (VPanel *)panel;

	vp->TraverseLevel( 1 );
	tmZone( TELEMETRY_LEVEL1, TMZF_NONE, "%s - %s", __FUNCTION__, vp->GetName() );

	// solve the parent
	vp->Solve();
	
	CUtlVector< VPanel * > &children = vp->GetChildren();

	// WARNING: Some of the think functions add/remove children, so make sure we
	//  explicitly check for children.Count().
	for ( int i = 0; i < children.Count(); ++i )
	{
		VPanel *child = children[ i ];
		if (child->IsVisible())
		{
			InternalSolveTraverse( (VPANEL)child );
		}
	}

	vp->TraverseLevel( -1 );
}

IHTML* CVGui::CreateHTMLWindow(vgui::IHTMLEvents* events, VPANEL context)
{
	Assert(!"CMatSystemSurface::CreateHTMLWindow");
	return NULL;
}


void CVGui::DeleteHTMLWindow(IHTML* htmlwin)
{
}



void CVGui::PaintHTMLWindow(IHTML* htmlwin)
{
}

bool CVGui::BHTMLWindowNeedsPaint(IHTML* htmlwin)
{
	return false;
}

void CVGui::SetModalPanel(VPANEL)
{
}

VPANEL CVGui::GetModalPanel()
{
	return 0;
}

VPANEL CVGui::GetTopmostPopup()
{
	return 0;
}

bool CVGui::IsInThink(VPANEL panel)
{
	if (m_bInThink)
	{
		if (panel == m_CurrentThinkPanel) // HasParent() returns true if you pass yourself in
		{
			return false;
		}

		return g_pVGui->HasParent(panel, m_CurrentThinkPanel);
	}
	return false;
}

void CVGui::RestrictPaintToSinglePanel(VPANEL panel)
{
	if (panel && m_pRestrictedPanel && m_pRestrictedPanel == g_pInput->GetAppModalSurface())
	{
		return;	// don't restrict drawing to a panel other than the modal one - that's a good way to hang the game.
	}

	m_pRestrictedPanel = panel;

	if (!g_pInput->GetAppModalSurface())
	{
		g_pInput->SetAppModalSurface(panel);	// if painting is restricted to this panel, it had better be modal, or else you can get in some bad state...
	}
}

//-----------------------------------------------------------------------------
// Is a panel under the restricted panel?
//-----------------------------------------------------------------------------
bool CVGui::IsPanelUnderRestrictedPanel(VPANEL panel)
{
	if (!m_pRestrictedPanel)
		return true;

	while (panel)
	{
		if (panel == m_pRestrictedPanel)
			return true;

		panel = g_pVGui->GetParent(panel);
	}
	return false;
}

bool CVGui::ShouldPaintChildPanel(VPANEL childPanel)
{
	if (m_pRestrictedPanel && (m_pRestrictedPanel != childPanel) &&
		!g_pVGui->HasParent(childPanel, m_pRestrictedPanel))
	{
		return false;
	}

	bool isPopup = g_pVGui->IsPopup(childPanel);
	return !isPopup;
}

VPANEL CVGui::GetRestrictPaintSinglePanel() {
	return m_pRestrictedPanel;
}

void CVGui::CalculateMouseVisible()
{
	int i;
	m_bNeedsMouse = false;
	m_bNeedsKeyboard = false;

	if (g_pInput->GetMouseCapture() != 0)
		return;

	int c = g_pVGui->GetPopupCount();

	//VPANEL modalSubTree = g_pInput->GetModalSubTree();
	//if (modalSubTree)
	//{
	//	for (i = 0; i < c; i++)
	//	{
	//		VPanel* pop = (VPanel*)g_pVGui->GetPopup(i);
	//		bool isChildOfModalSubPanel = IsChildOfModalSubTree((VPANEL)pop);
	//		if (!isChildOfModalSubPanel)
	//			continue;

	//		bool isVisible = pop->IsVisible();
	//		VPanel* p = pop->GetParent();

	//		while (p && isVisible)
	//		{
	//			if (p->IsVisible() == false)
	//			{
	//				isVisible = false;
	//				break;
	//			}
	//			p = p->GetParent();
	//		}

	//		if (isVisible)
	//		{
	//			m_bNeedsMouse = m_bNeedsMouse || pop->IsMouseInputEnabled();
	//			m_bNeedsKeyboard = m_bNeedsKeyboard || pop->IsKeyBoardInputEnabled();

	//			// Seen enough!!!
	//			if (m_bNeedsMouse && m_bNeedsKeyboard)
	//				break;
	//		}
	//	}
	//}
	//else
	//{
		for (i = 0; i < c; i++)
		{
			VPANEL pop = g_pVGui->GetPopup(i);

			bool isVisible = g_pVGui->IsVisible(pop);
			VPANEL p = g_pVGui->GetParent(pop);

			while (p && isVisible)
			{
				if (g_pVGui->IsVisible(p) == false)
				{
					isVisible = false;
					break;
				}
				p = g_pVGui->GetParent(p);
			}

			if (isVisible)
			{
				m_bNeedsMouse = m_bNeedsMouse || g_pVGui->IsMouseInputEnabled(pop);
				m_bNeedsKeyboard = m_bNeedsKeyboard || g_pVGui->IsKeyBoardInputEnabled(pop);

				// Seen enough!!!
				if (m_bNeedsMouse && m_bNeedsKeyboard)
					break;
			}
		}
	//}

	if (m_bNeedsMouse)
	{
		// NOTE: We must unlock the cursor *before* the set call here.
		// Failing to do this causes s_bCursorVisible to not be set correctly
		// (UnlockCursor fails to set it correctly)
		UnlockCursor();
		if (_currentCursor == vgui::dc_none)
		{
			SetCursor(vgui::dc_arrow);
		}
	}
	else
	{
		SetCursor(vgui::dc_none);
		LockCursor();
	}
}

bool CVGui::NeedKBInput()
{
	return m_bNeedsKeyboard;
}

void CVGui::SetPanelForInput(VPANEL vpanel)
{
	g_pInput->AssociatePanelWithInputContext(DEFAULT_INPUT_CONTEXT, vpanel);
	if (vpanel)
	{
		m_bNeedsKeyboard = true;
	}
	else
	{
		m_bNeedsKeyboard = false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: unlocks the cursor state
//-----------------------------------------------------------------------------
bool CVGui::IsCursorLocked() const
{
	return ::IsCursorLocked();
}

void CVGui::SetCursor(HCursor hCursor)
{
	if (IsCursorLocked())
		return;

	if (_currentCursor != hCursor)
	{
		_currentCursor = hCursor;
		CursorSelect(hCursor);
	}
}

bool CVGui::IsCursorVisible()
{
	return m_cursorAlwaysVisible || (_currentCursor != dc_none);
}

void CVGui::SetCursorAlwaysVisible(bool visible)
{
	m_cursorAlwaysVisible = visible;
	CursorSelect(visible ? dc_alwaysvisible_push : dc_alwaysvisible_pop);
}

void CVGui::SetCursorPos(int x, int y)
{
	CursorSetPos(m_HWnd, x, y);
}

void CVGui::GetCursorPos(int& x, int& y)
{
	CursorGetPos(m_HWnd, x, y);
}

//-----------------------------------------------------------------------------
// Hook needed to Get input to work
//-----------------------------------------------------------------------------
void CVGui::AttachToWindow(void* hWnd, bool bLetAppDriveInput)
{
	InputDetachFromWindow(m_HWnd);
	m_HWnd = hWnd;
	if (hWnd)
	{
		InputAttachToWindow(hWnd);
		m_bAppDrivesInput = bLetAppDriveInput;
	}
	else
	{
		// Never call RunFrame stuff
		m_bAppDrivesInput = true;
	}
}

void CVGui::UnlockCursor()
{
	::LockCursor(false);
}

void CVGui::LockCursor()
{
	::LockCursor(true);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *curOrAniFile - 
// Output : vgui::HCursor
//-----------------------------------------------------------------------------
vgui::HCursor CVGui::CreateCursorFromFile(char const* curOrAniFile, char const* pPathID)
{
	return Cursor_CreateCursorFromFile(curOrAniFile, pPathID);
}

//-----------------------------------------------------------------------------
// Purpose: Handle switching in and out of "render to fullscreen" mode. We don't
//			actually support this mode in tools.
//-----------------------------------------------------------------------------
void CVGui::SetSoftwareCursor(bool bUseSoftwareCursor)
{
	EnableSoftwareCursor(bUseSoftwareCursor);
}

bool CVGui::GetSoftwareCursor() {
	return ShouldDrawSoftwareCursor();
}

int CVGui::GetSoftwareCursorTextureId(float* px, float* py) {
	return GetSoftwareCursorTexture(px, py);
}

bool CVGui::HandleInputEvent(const InputEvent_t& event)
{
	if (!m_bAppDrivesInput)
	{
		g_pInput->UpdateButtonState(event);
	}

	return InputHandleInputEvent(event);
}

void CVGui::EnableWindowsMessages(bool bEnable)
{
	EnableInput(bEnable);
}

void CVGui::SetEmbeddedPanel(VPANEL pEmbeddedPanel)
{
	m_pEmbeddedPanel = pEmbeddedPanel;
	g_pVGui->Client(pEmbeddedPanel)->RequestFocus(0);
}

//-----------------------------------------------------------------------------
// hierarchy root
//-----------------------------------------------------------------------------
VPANEL CVGui::GetEmbeddedPanel()
{
	return m_pEmbeddedPanel;
}

#if defined( WIN32 ) && !defined( _X360 )
static bool GetIconSize(ICONINFO& iconInfo, int& w, int& h)
{
	w = h = 0;

	HBITMAP bitmap = iconInfo.hbmColor;
	BITMAP bm;
	if (0 == GetObject((HGDIOBJ)bitmap, sizeof(BITMAP), (LPVOID)&bm))
	{
		return false;
	}

	w = bm.bmWidth;
	h = bm.bmHeight;

	return true;
}

// If rgba is NULL, bufsize gets filled in w/ # of bytes required
static bool GetIconBits(HDC hdc, ICONINFO& iconInfo, int& w, int& h, unsigned char* rgba, size_t& bufsize)
{
	if (!iconInfo.hbmColor || !iconInfo.hbmMask)
		return false;

	if (!rgba)
	{
		if (!GetIconSize(iconInfo, w, h))
			return false;

		bufsize = (size_t)((w * h) << 2);
		return true;
	}

	bool bret = false;

	Assert(w > 0);
	Assert(h > 0);
	Assert(bufsize == (size_t)((w * h) << 2));

	DWORD* maskData = new DWORD[w * h];
	DWORD* colorData = new DWORD[w * h];
	DWORD* output = (DWORD*)rgba;

	BITMAPINFO bmInfo;

	memset(&bmInfo, 0, sizeof(bmInfo));
	bmInfo.bmiHeader.biSize = sizeof(bmInfo.bmiHeader);
	bmInfo.bmiHeader.biWidth = w;
	bmInfo.bmiHeader.biHeight = h;
	bmInfo.bmiHeader.biPlanes = 1;
	bmInfo.bmiHeader.biBitCount = 32;
	bmInfo.bmiHeader.biCompression = BI_RGB;

	// Get the info about the bits
	if (GetDIBits(hdc, iconInfo.hbmMask, 0, h, maskData, &bmInfo, DIB_RGB_COLORS) == h &&
		GetDIBits(hdc, iconInfo.hbmColor, 0, h, colorData, &bmInfo, DIB_RGB_COLORS) == h)
	{
		bret = true;

		for (int row = 0; row < h; ++row)
		{
			// Invert
			int r = (h - row - 1);
			int rowstart = r * w;

			DWORD* color = &colorData[rowstart];
			DWORD* mask = &maskData[rowstart];
			DWORD* outdata = &output[row * w];

			for (int col = 0; col < w; ++col)
			{
				unsigned char* cr = (unsigned char*)&color[col];

				// Set alpha
				cr[3] = mask[col] == 0 ? 0xff : 0x00;

				// Swap blue and red
				unsigned char t = cr[2];
				cr[2] = cr[0];
				cr[0] = t;

				*(unsigned int*)&outdata[col] = *(unsigned int*)cr;
			}
		}
	}

	delete[] colorData;
	delete[] maskData;

	return bret;
}

static bool ShouldMakeUnique(char const* extension)
{
	if (!Q_stricmp(extension, "cur"))
		return true;
	if (!Q_stricmp(extension, "ani"))
		return true;
	return false;
}
#endif // !_X360

vgui::IImage* CVGui::GetIconImageForFullPath(char const* pFullPath)
{
	vgui::IImage* newIcon = NULL;

#if defined( WIN32 ) && !defined( _X360 )
	SHFILEINFO info = { 0 };
	DWORD_PTR dwResult = SHGetFileInfo(
		pFullPath,
		0,
		&info,
		sizeof(info),
		SHGFI_TYPENAME | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_SHELLICONSIZE
	);
	if (dwResult)
	{
		if (info.szTypeName[0] != 0)
		{
			char ext[32];
			Q_ExtractFileExtension(pFullPath, ext, sizeof(ext));

			char lookup[512];
			Q_snprintf(lookup, sizeof(lookup), "%s", ShouldMakeUnique(ext) ? pFullPath : info.szTypeName);

			// Now check the dictionary
			unsigned short idx = m_FileTypeImages.Find(lookup);
			if (idx == m_FileTypeImages.InvalidIndex())
			{
				ICONINFO iconInfo;
				if (0 != GetIconInfo(info.hIcon, &iconInfo))
				{
					int w, h;
					size_t bufsize = 0;

					HDC hdc = ::GetDC(reinterpret_cast<HWND>(m_HWnd));

					if (GetIconBits(hdc, iconInfo, w, h, NULL, bufsize))
					{
						byte* bits = new byte[bufsize];
						if (bits && GetIconBits(hdc, iconInfo, w, h, bits, bufsize))
						{
							newIcon = new MemoryBitmap(bits, w, h);
						}
						delete[] bits;
					}

					::ReleaseDC(reinterpret_cast<HWND>(m_HWnd), hdc);
				}

				idx = m_FileTypeImages.Insert(lookup, newIcon);
			}

			newIcon = m_FileTypeImages[idx];
		}

		DestroyIcon(info.hIcon);
	}
#endif
	return newIcon;
}

void CVGui::EnableMouseCapture(VPANEL panel, bool state)
{
#ifdef WIN32
	if (state)
	{
		::SetCapture(reinterpret_cast<HWND>(m_HWnd));
	}
	else
	{
		::ReleaseCapture();
	}
#elif defined( POSIX )
	// SetCapture on Win32 makes all the mouse messages (move and button up/down) head to
	//	the captured window. From what I can tell, this routine is called for modal dialogs
	//	when you click down on a button. However the current behavior is to highlight the
	//	buttons when you're over them, and trigger when you mouse up over the top - so I
	//	don't believe that SetCapture is needed on Windows, and Linux is behaving exactly
	//	the same as Win32 in all the tests I've run so far. (I've clicked on a lot of dialogs).
	// I talked with Alfred about this and we haven't done any SetCapture stuff on OSX ever
	//  and he says nobody has ever reported any regressions.
	// So I've removed the Assert. 8/32/2012 - mikesart.
#else
#error
#endif
}