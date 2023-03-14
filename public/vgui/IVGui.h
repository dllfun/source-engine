//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#ifndef IVGUI_H
#define IVGUI_H

#ifdef _WIN32
#pragma once
#endif

#include "tier1/interface.h"
#include "tier1/utlvector.h"
#include <vgui/VGUI.h>
#include <vgui/IHTML.h> // CreateHTML, PaintHTML 

#include "appframework/IAppSystem.h"

#ifdef GetClassName
#undef GetClassName
#endif
#ifdef PostMessage
#undef PostMessage
#endif

class KeyValues;

namespace vgui
{
class SurfacePlat;
class IClientPanel;

//!! must be removed
class Panel;
// safe handle to a panel - can be converted to and from a VPANEL
typedef unsigned long HPanel;
typedef int HContext;

enum
{
	DEFAULT_VGUI_CONTEXT = ((vgui::HContext)~0)
};

// safe handle to a panel - can be converted to and from a VPANEL
typedef unsigned long HPanel;

//-----------------------------------------------------------------------------
// Purpose: Interface to core vgui components
//-----------------------------------------------------------------------------
class IVGui : public IAppSystem
{
public:
	// activates vgui message pump
	virtual void Start() = 0;

	// signals vgui to Stop running
	virtual void Stop() = 0;

	// returns true if vgui is current active
	virtual bool IsRunning() = 0;

	// runs a single frame of vgui
	virtual void RunFrame() = 0;

	// broadcasts "ShutdownRequest" "id" message to all top-level panels in the app
	virtual void ShutdownMessage(unsigned int shutdownID) = 0;

	// panel allocation
	virtual VPANEL AllocPanel() = 0;
	virtual void FreePanel(VPANEL panel) = 0;
	
	// debugging prints
	virtual void DPrintf(PRINTF_FORMAT_STRING const char *format, ...) = 0;
	virtual void DPrintf2(PRINTF_FORMAT_STRING const char *format, ...) = 0;
	virtual void SpewAllActivePanelNames() = 0;
	
	// safe-pointer handle methods
	virtual HPanel PanelToHandle(VPANEL panel) = 0;
	virtual VPANEL HandleToPanel(HPanel index) = 0;
	virtual void MarkPanelForDeletion(VPANEL panel) = 0;

	// makes panel receive a 'Tick' message every frame (~50ms, depending on sleep times/framerate)
	// panel is automatically removed from tick signal list when it's deleted
	virtual void AddTickSignal(VPANEL panel, int intervalMilliseconds = 0 ) = 0;
	virtual void RemoveTickSignal(VPANEL panel) = 0;

	// message sending
	virtual void PostMessage(VPANEL target, KeyValues *params, VPANEL from, float delaySeconds = 0.0f) = 0;

	// Creates/ destroys vgui contexts, which contains information
	// about which controls have mouse + key focus, for example.
	virtual HContext CreateContext() = 0;
	virtual void DestroyContext( HContext context ) = 0; 

	// Associates a particular panel with a vgui context
	// Associating NULL is valid; it disconnects the panel from the context
	virtual void AssociatePanelWithContext( HContext context, VPANEL pRoot ) = 0;

	// Activates a particular context, use DEFAULT_VGUI_CONTEXT
	// to get the one normally used by VGUI
	virtual void ActivateContext( HContext context ) = 0;

	// whether to sleep each frame or not, true = sleep
	virtual void SetSleep( bool state) = 0; 

	// data accessor for above
	virtual bool GetShouldVGuiControlSleep() = 0;

	// enables VR mode
	virtual void SetVRMode( bool bVRMode ) = 0;
	virtual bool GetVRMode() = 0;

	// add a tick signal like above, but to the head of the list of tick signals
	virtual void AddTickSignalToHead( VPANEL panel, int intervalMilliseconds = 0 ) = 0;

	virtual void Init(VPANEL vguiPanel, IClientPanel* panel) = 0;

	// methods
	virtual void SetPos(VPANEL vguiPanel, int x, int y) = 0;
	virtual void GetPos(VPANEL vguiPanel, int& x, int& y) = 0;
	virtual void SetSize(VPANEL vguiPanel, int wide, int tall) = 0;
	virtual void GetSize(VPANEL vguiPanel, int& wide, int& tall) = 0;
	virtual void SetMinimumSize(VPANEL vguiPanel, int wide, int tall) = 0;
	virtual void GetMinimumSize(VPANEL vguiPanel, int& wide, int& tall) = 0;
	virtual void SetZPos(VPANEL vguiPanel, int z) = 0;
	virtual int  GetZPos(VPANEL vguiPanel) = 0;

	virtual void GetAbsPos(VPANEL vguiPanel, int& x, int& y) = 0;
	virtual void GetClipRect(VPANEL vguiPanel, int& x0, int& y0, int& x1, int& y1) = 0;
	virtual void SetInset(VPANEL vguiPanel, int left, int top, int right, int bottom) = 0;
	virtual void GetInset(VPANEL vguiPanel, int& left, int& top, int& right, int& bottom) = 0;

	virtual void SetVisible(VPANEL vguiPanel, bool state) = 0;
	virtual bool IsVisible(VPANEL vguiPanel) = 0;
	virtual void SetParent(VPANEL vguiPanel, VPANEL newParent) = 0;
	virtual int GetChildCount(VPANEL vguiPanel) = 0;
	virtual VPANEL GetChild(VPANEL vguiPanel, int index) = 0;
	virtual CUtlVector< VPANEL >& GetChildren(VPANEL vguiPanel) = 0;
	virtual VPANEL GetParent(VPANEL vguiPanel) = 0;
	virtual void MoveToFront(VPANEL vguiPanel) = 0;
	virtual void MoveToBack(VPANEL vguiPanel) = 0;
	virtual bool HasParent(VPANEL vguiPanel, VPANEL potentialParent) = 0;
	virtual bool IsPopup(VPANEL vguiPanel) = 0;
	virtual void SetPopup(VPANEL vguiPanel, bool state) = 0;
	virtual bool IsFullyVisible(VPANEL vguiPanel) = 0;

	// gets the scheme this panel uses
	virtual HScheme GetScheme(VPANEL vguiPanel) = 0;
	// gets whether or not this panel should scale with screen resolution
	virtual bool IsProportional(VPANEL vguiPanel) = 0;
	// returns true if auto-deletion flag is set
	virtual bool IsAutoDeleteSet(VPANEL vguiPanel) = 0;
	// deletes the Panel * associated with the vpanel
	virtual void DeletePanel(VPANEL vguiPanel) = 0;

	// input interest
	virtual void SetKeyBoardInputEnabled(VPANEL vguiPanel, bool state) = 0;
	virtual void SetMouseInputEnabled(VPANEL vguiPanel, bool state) = 0;
	virtual bool IsKeyBoardInputEnabled(VPANEL vguiPanel) = 0;
	virtual bool IsMouseInputEnabled(VPANEL vguiPanel) = 0;

	// calculates the panels current position within the hierarchy
	virtual void Solve(VPANEL vguiPanel) = 0;

	// gets names of the object (for debugging purposes)
	virtual const char* GetName(VPANEL vguiPanel) = 0;
	virtual const char* GetClassName(VPANEL vguiPanel) = 0;

	// delivers a message to the panel
	virtual void SendMessage(VPANEL vguiPanel, KeyValues* params, VPANEL ifromPanel) = 0;

	// these pass through to the IClientPanel
	virtual void Think(VPANEL vguiPanel) = 0;
	virtual void PerformApplySchemeSettings(VPANEL vguiPanel) = 0;
	virtual void PaintTraverse(VPANEL vguiPanel, bool forceRepaint, bool allowForce = true) = 0;
	virtual void Repaint(VPANEL vguiPanel) = 0;
	virtual VPANEL IsWithinTraverse(VPANEL vguiPanel, int x, int y, bool traversePopups) = 0;
	virtual void OnChildAdded(VPANEL vguiPanel, VPANEL child) = 0;
	virtual void OnSizeChanged(VPANEL vguiPanel, int newWide, int newTall) = 0;

	virtual void InternalFocusChanged(VPANEL vguiPanel, bool lost) = 0;
	virtual bool RequestInfo(VPANEL vguiPanel, KeyValues* outputData) = 0;
	virtual void RequestFocus(VPANEL vguiPanel, int direction = 0) = 0;
	virtual bool RequestFocusPrev(VPANEL vguiPanel, VPANEL existingPanel) = 0;
	virtual bool RequestFocusNext(VPANEL vguiPanel, VPANEL existingPanel) = 0;
	virtual VPANEL GetCurrentKeyFocus(VPANEL vguiPanel) = 0;
	virtual int GetTabPosition(VPANEL vguiPanel) = 0;

	// used by ISurface to store platform-specific data
	virtual SurfacePlat* Plat(VPANEL vguiPanel) = 0;
	virtual void SetPlat(VPANEL vguiPanel, SurfacePlat* Plat) = 0;

	// returns a pointer to the vgui controls baseclass Panel *
	// destinationModule needs to be passed in to verify that the returned Panel * is from the same module
	// it must be from the same module since Panel * vtbl may be different in each module
	virtual Panel* GetPanel(VPANEL vguiPanel, const char* destinationModule) = 0;

	virtual bool IsEnabled(VPANEL vguiPanel) = 0;
	virtual void SetEnabled(VPANEL vguiPanel, bool state) = 0;

	// Used by the drag/drop manager to always draw on top
	virtual bool IsTopmostPopup(VPANEL vguiPanel) = 0;
	virtual void SetTopmostPopup(VPANEL vguiPanel, bool state) = 0;

	// sibling pins
	virtual void SetSiblingPin(VPANEL vguiPanel, VPANEL newSibling, byte iMyCornerToPin = 0, byte iSiblingCornerToPinTo = 0) = 0;

	// from surface
	virtual void CreatePopup(VPANEL panel, bool minimised, bool showTaskbarIcon = true, bool disabled = false, bool mouseInput = true, bool kbInput = true) = 0;

	virtual void MovePopupToFront(VPANEL panel) = 0;

	virtual void MovePopupToBack(VPANEL panel) = 0;

	virtual int GetPopupCount() = 0;

	virtual VPANEL GetPopup(int index) = 0;

	virtual void AddPanel(VPANEL panel) = 0;
	virtual void ReleasePanel(VPANEL panel) = 0;

	virtual void SetTitle(VPANEL panel, const wchar_t* title) = 0;

	virtual const wchar_t* GetTitle(VPANEL panel) = 0;

	virtual void BringToFront(VPANEL panel) = 0;

	virtual void SetForegroundWindow(VPANEL panel) = 0;

	virtual void SetTopLevelFocus(VPANEL panel) = 0;

	virtual void SetPanelVisible(VPANEL panel, bool state) = 0;

	virtual void SetMinimized(VPANEL panel, bool state) = 0;
	virtual bool IsMinimized(VPANEL panel) = 0;
	virtual void FlashWindow(VPANEL panel, bool state) = 0;

	virtual void SwapBuffers(VPANEL panel) = 0;
	virtual void Invalidate(VPANEL panel) = 0;

	virtual void ApplyChanges() = 0;
	virtual bool IsWithin(int x, int y) = 0;
	virtual bool HasFocus() = 0;

	virtual void SolveTraverse(VPANEL panel, bool forceApplySchemeSettings = false) = 0;

	virtual void SetAsToolBar(VPANEL panel, bool state) = 0;

	virtual void SetAsTopMost(VPANEL panel, bool state) = 0;

	// notify icons?!?
	virtual VPANEL GetNotifyPanel() = 0;
	virtual void SetNotifyIcon(VPANEL context, HTexture icon, VPANEL panelToReceiveMessages, const char* text) = 0;

	virtual bool RecreateContext(VPANEL panel) = 0;

	virtual void RestrictPaintToSinglePanel(VPANEL panel) = 0;
	virtual bool IsPanelUnderRestrictedPanel(VPANEL panel) = 0 ;
	virtual VPANEL GetRestrictPaintSinglePanel() = 0 ;
	virtual bool ShouldPaintChildPanel(VPANEL childPanel) = 0;

	virtual IHTML* CreateHTMLWindow(vgui::IHTMLEvents* events, VPANEL context) = 0;
	virtual void PaintHTMLWindow(vgui::IHTML* htmlwin) = 0;
	virtual void DeleteHTMLWindow(IHTML* htmlwin) = 0;
	virtual bool BHTMLWindowNeedsPaint(IHTML* htmlwin) = 0;

	virtual void SetModalPanel(VPANEL) = 0;
	virtual VPANEL GetModalPanel() = 0;

	virtual VPANEL GetTopmostPopup() = 0;

	virtual void CalculateMouseVisible() = 0;
	virtual bool NeedKBInput() = 0;

	virtual void SetPanelForInput(VPANEL vpanel) = 0;

	virtual bool IsCursorLocked(void) const = 0;

	virtual void SetCursor(HCursor cursor) = 0;

	virtual bool IsCursorVisible() = 0;

	virtual void SetCursorAlwaysVisible(bool visible) = 0;

	virtual void UnlockCursor() = 0;
	virtual void LockCursor() = 0;

	virtual void SetCursorPos(int x, int y) = 0;
	virtual void GetCursorPos(int& x, int& y) = 0;

	virtual vgui::HCursor CreateCursorFromFile(char const* curOrAniFile, char const* pPathID = 0) = 0;

	virtual void AttachToWindow(void* hwnd, bool bLetAppDriveInput) = 0;

	virtual void SetSoftwareCursor(bool bUseSoftwareCursor) = 0;
	virtual bool GetSoftwareCursor() = 0;
	virtual int GetSoftwareCursorTextureId(float* px, float* py) = 0;

	virtual void EnableWindowsMessages(bool bEnable) = 0;
	virtual bool HandleInputEvent(const InputEvent_t& event) = 0;

	virtual void SetEmbeddedPanel(vgui::VPANEL pEmbeddedPanel) = 0;
	virtual vgui::VPANEL GetEmbeddedPanel() = 0;

	virtual bool HasCursorPosFunctions() = 0;
};

#define VGUI_IVGUI_INTERFACE_VERSION "VGUI_ivgui008"

};


#endif // IVGUI_H
