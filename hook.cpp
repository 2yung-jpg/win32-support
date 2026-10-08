// ==WindhawkMod==
// @id              universal-win32-dark-mode
// @name            Universal Win32 Dark Mode (best effort)
// @description     Dark title bars, menu bars, popup menus, status bars, dialogs, tabs, toolbars, group boxes and common controls for classic Win32 apps (incl. Win+R Run dialog)
// @version         0.6.0
// @author          you
// @architecture    x86-64
// @architecture    x86
// @include         *
// @exclude         chrome.exe
// @exclude         msedge.exe
// @exclude         firefox.exe
// @exclude         brave.exe
// @exclude         opera.exe
// @exclude         vivaldi.exe
// @exclude         Code.exe
// @exclude         Discord.exe
// @exclude         slack.exe
// @exclude         Teams.exe
// @exclude         ms-teams.exe
// @exclude         Spotify.exe
// @exclude         ApplicationFrameHost.exe
// @exclude         TextInputHost.exe
// @exclude         SearchHost.exe
// @exclude         StartMenuExperienceHost.exe
// @exclude         ShellExperienceHost.exe
// @compilerOptions -ldwmapi -luxtheme -lcomctl32 -luser32 -lgdi32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Universal Win32 Dark Mode (best effort)

Win32 dark mode is opt-in per app, so there is no real universal switch. This
mod injects into the processes it is enabled for and:

* turns on dark title bars
* makes popup/context menus dark (`SetPreferredAppMode`)
* draws **menu bars** dark (UAH messages, like the Notepad dark mode mod)
* custom-paints **status bars**, **tab controls** and **group boxes** dark (or
  uses Windows' own `DarkMode_DarkTheme` where available, see below)
* dark progress bars and toolbars (custom draw)
* applies `DarkMode_*` visual styles to buttons, edits, combo/list boxes,
  list/tree views, headers, scrollbars, trackbars, date pickers, month
  calendars, rich edits and tooltips
* recolors list views, tree views and rich edits, dark list/tree checkboxes
  (Windows 11), dark SysLink colors
* subclasses framed top-level windows and standard dialogs (`#32770`) so
  `WM_CTLCOLOR*` returns dark brushes / light text
* (optional, on by default) hooks `GetSysColor` / `GetSysColorBrush` so that
  classic apps which paint with system colors get a dark palette

## Making it system-wide
The mod is compiled for **both x86-64 and x86**, so 32-bit apps are covered
too. For the mod to reach every process, also check in Windhawk:
* Settings > Advanced settings: make sure the process exclusion list does not
  contain the apps you care about, and that no custom *include* list is set
  for this mod (Mod > Advanced tab).
* Run Windhawk as administrator (or install its service) so it can inject
  into elevated processes such as Task Manager.
* Already-running apps need a restart (new processes are injected
  automatically).

## TaskDialogs (experimental)
Dialogs such as Notepad's save prompt are TaskDialogs, which are drawn by
DirectUI inside comctl32 and ignore normal control theming. The mod hooks the
uxtheme drawing functions for the "TaskDialog" visual style class and
substitutes dark panels and light text. Every TaskDialog part that is drawn is
written to the Windhawk log, so unthemed parts can be identified.

## Native dark theme (Windows 11 24H2 / 25H2, revision 6899+)
Recent Windows 11 builds ship a `DarkMode_DarkTheme` visual style. When the
"Prefer native dark theme" option is on and the build supports it, edits,
combo boxes, tab controls, status bars and headers use it instead of the
custom painting. Turn it off if something looks wrong.

## explorer.exe
Explorer is included so that the Win+R Run dialog and other shell dialogs
become dark. Inside explorer.exe only dialogs (`#32770`) and the autocomplete
dropdown are themed; the taskbar, desktop and File Explorer windows are left
alone. The system color hook is not applied inside explorer.exe.

## Task Manager
On Windows 10 (and Windows 11 before 22H2) the classic Task Manager is
themed: title bar, menu bar, tabs, group boxes, sunken graph frames and the
Details/Services/Users/Startup lists. The Processes and Performance graph
panes are DirectUI/custom-drawn and paint themselves, so they cannot be
recolored. On Windows 11 22H2+ Task Manager is WinUI; use its own
Settings > Mode > Dark instead.

## Limitations
* Anything owner-drawn or with hardcoded colors is not recolored.
* Up-down controls on Windows 10 and date pickers need subclass painting that
  is not included here; they get the visual style only.
* Toolbars inside rebars may stay light.
* The system color hook can make apps that draw dark-on-light content with
  system colors look odd (image editors etc.). Turn it off or exclude them.
* The experimental "dark window background" option can make apps that paint
  black text on the class background unreadable. Leave it off unless needed.
* Exclude apps that already have their own dark mode (browsers, Electron, UWP).
* Requires Windows 10 1809 or newer.

## Credits
Techniques and code adapted from m417z's "Dark Mode for Notepad" Windhawk mod,
Notepad++ (DarkMode.cpp), adzm/win32-darkmode, Process Hacker (status bar) and
ozone10/win32-darkmodelib (control theming rules, tooltips, checkboxes, native
dark theme detection, title bar handling), which is MPL-2.0 licensed
(Copyright (c) 2025-2026 ozone10; https://mozilla.org/MPL/2.0/). Code derived
from darkmodelib stays under that license. Refer to the other projects for
their licenses.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- forceDark: true
  $name: Force dark mode
  $description: Force dark menus/controls even if Windows is set to light mode. If off, follows the system setting.
- darkMenuBar: true
  $name: Dark menu bars
  $description: Draw menu bars dark (windows with a classic menu).
- darkStatusBar: true
  $name: Dark status bars
  $description: Dark msctls_statusbar32 controls.
- recolorLists: true
  $name: Recolor list views, tree views and rich edits
  $description: Dark background and light text on SysListView32 / SysTreeView32 / RichEdit controls.
- recolorDialogs: true
  $name: Dark dialogs and control colors
  $description: Dark brushes/light text for standard dialogs and framed windows, plus dark tab controls, group boxes and toolbars.
- hookSysColors: true
  $name: Dark system colors (GetSysColor hook)
  $description: Make GetSysColor/GetSysColorBrush return a dark palette so classic apps that paint with system colors become dark. Not applied in explorer.exe. Turn off if an app looks wrong.
- darkTaskDialogs: true
  $name: Dark TaskDialogs (experimental)
  $description: Hook uxtheme so TaskDialogs (e.g. Notepad's "Do you want to save changes?") draw dark panels and light text. Check the Windhawk log if it has no effect.
- nativeDarkTheme: true
  $name: Prefer native dark theme
  $description: On Windows 11 24H2/25H2 (revision 6899+) use the built-in DarkMode_DarkTheme for edits, combo boxes, tabs, status bars and headers instead of custom painting.
- darkWindowBackground: false
  $name: Dark window background (experimental)
  $description: Also fill the background of framed non-dialog windows dark. Can break apps that paint dark text on the default background.
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <richedit.h>
#include <uxtheme.h>
#include <vsstyle.h>
#include <vssym32.h>
#include <windowsx.h>

#include <wctype.h>

#include <map>
#include <mutex>
#include <set>

// ---------------------------------------------------------------------------
// Constants / undocumented API types
// ---------------------------------------------------------------------------

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

static const COLORREF kBgColor = RGB(32, 32, 32);
static const COLORREF kCtlColor = RGB(45, 45, 45);
static const COLORREF kStatusColor = RGB(43, 43, 43);
static const COLORREF kBorderColor = RGB(70, 70, 70);
static const COLORREF kTextColor = RGB(240, 240, 240);
static const COLORREF kDisabledTextColor = RGB(128, 128, 128);
static const COLORREF kLinkTextColor = RGB(86, 156, 246);
static const COLORREF kAccentBlue = RGB(0x00, 0x78, 0xD7);

enum PreferredAppMode { PAM_Default = 0, PAM_AllowDark = 1, PAM_ForceDark = 2 };

struct WcaData {
    int attrib;
    PVOID pvData;
    SIZE_T cbData;
};
#define WCA_USEDARKMODECOLORS_ID 26

using SetPreferredAppMode_t = int(WINAPI*)(int);             // ordinal 135 (1903+)
using AllowDarkModeForApp_t = bool(WINAPI*)(bool);           // ordinal 135 (1809)
using AllowDarkModeForWindow_t = bool(WINAPI*)(HWND, bool);  // ordinal 133
using FlushMenuThemes_t = void(WINAPI*)();                   // ordinal 136
using RefreshImmersiveColorPolicyState_t = void(WINAPI*)();  // ordinal 104
using SetWindowCompositionAttribute_t = BOOL(WINAPI*)(HWND, WcaData*);
using RtlGetNtVersionNumbers_t = void(WINAPI*)(LPDWORD, LPDWORD, LPDWORD);

static SetPreferredAppMode_t g_SetPreferredAppMode;
static AllowDarkModeForApp_t g_AllowDarkModeForApp;
static AllowDarkModeForWindow_t g_AllowDarkModeForWindow;
static FlushMenuThemes_t g_FlushMenuThemes;
static RefreshImmersiveColorPolicyState_t g_RefreshImmersiveColorPolicyState;
static SetWindowCompositionAttribute_t g_SetWindowCompositionAttribute;

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------

static struct {
    bool forceDark;
    bool darkMenuBar;
    bool darkStatusBar;
    bool recolorLists;
    bool recolorDialogs;
    bool hookSysColors;
    bool darkTaskDialogs;
    bool nativeDarkTheme;
    bool darkWindowBackground;
} g_settings;

static DWORD g_build;
static bool g_darkThemeAvailable;  // DarkMode_DarkTheme exists (Win11 24H2/25H2 rev 6899+)
static HBRUSH g_bgBrush;
static HBRUSH g_ctlBrush;
static UINT g_subclassMsg;  // wParam TRUE: install (via hook), FALSE: remove
static bool g_unloading;
static bool g_isExplorer;
static bool g_isTaskManager;

static std::mutex g_mutex;
static std::set<HWND> g_subclassed;
static std::map<DWORD, HHOOK> g_hooks;  // per-thread WH_CALLWNDPROCRET hooks

static const UINT_PTR kIdWindow = 0xDA2C0001;
static const UINT_PTR kIdStatus = 0xDA2C0002;
static const UINT_PTR kIdTab = 0xDA2C0003;
static const UINT_PTR kIdGroup = 0xDA2C0004;

// ---------------------------------------------------------------------------
// Settings / app mode / title bar
// ---------------------------------------------------------------------------

static void LoadSettings() {
    g_settings.forceDark = Wh_GetIntSetting(L"forceDark") != 0;
    g_settings.darkMenuBar = Wh_GetIntSetting(L"darkMenuBar") != 0;
    g_settings.darkStatusBar = Wh_GetIntSetting(L"darkStatusBar") != 0;
    g_settings.recolorLists = Wh_GetIntSetting(L"recolorLists") != 0;
    g_settings.recolorDialogs = Wh_GetIntSetting(L"recolorDialogs") != 0;
    g_settings.hookSysColors = Wh_GetIntSetting(L"hookSysColors") != 0;
    g_settings.darkTaskDialogs = Wh_GetIntSetting(L"darkTaskDialogs") != 0;
    g_settings.nativeDarkTheme = Wh_GetIntSetting(L"nativeDarkTheme") != 0;
    g_settings.darkWindowBackground =
        Wh_GetIntSetting(L"darkWindowBackground") != 0;
}

// Windows 11 24H2 (26100) / 25H2 (26200) with revision 6899+, or later builds
// (same rule as darkmodelib's doesWin11SupportDarkThemeStyle()).
static bool DetectDarkThemeAvailable() {
    DWORD ubr = 0, size = sizeof(ubr);
    if (RegGetValueW(HKEY_LOCAL_MACHINE,
                     L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"UBR",
                     RRF_RT_REG_DWORD, nullptr, &ubr, &size) != ERROR_SUCCESS) {
        ubr = 0;
    }
    return g_build > 26200 ||
           ((g_build == 26200 || g_build == 26100) && ubr >= 6899);
}

static inline bool UseNativeTheme() {
    return g_settings.nativeDarkTheme && g_darkThemeAvailable;
}

static void ApplyAppMode(bool dark) {
    if (g_build >= 18362) {
        if (g_SetPreferredAppMode) {
            g_SetPreferredAppMode(dark ? (g_settings.forceDark ? PAM_ForceDark
                                                               : PAM_AllowDark)
                                       : PAM_Default);
        }
    } else if (g_AllowDarkModeForApp) {
        g_AllowDarkModeForApp(dark);
    }
    if (g_RefreshImmersiveColorPolicyState) g_RefreshImmersiveColorPolicyState();
    if (g_FlushMenuThemes) g_FlushMenuThemes();
}

static void AllowDarkMode(HWND hwnd, bool allow) {
    if (g_AllowDarkModeForWindow) g_AllowDarkModeForWindow(hwnd, allow);
}

static void SetTitleBarDark(HWND hwnd, BOOL dark) {
    if (g_build >= 19041) {
        DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark,
                              sizeof(dark));
    } else if (g_build >= 18362 && g_SetWindowCompositionAttribute) {
        WcaData data = {WCA_USEDARKMODECOLORS_ID, &dark, sizeof(dark)};
        g_SetWindowCompositionAttribute(hwnd, &data);
    } else {
        SetPropW(hwnd, L"UseImmersiveDarkModeColors",
                 reinterpret_cast<HANDLE>(static_cast<INT_PTR>(dark)));
    }
}

// On Windows 10 the title bar only picks up the change after a redraw
// (darkmodelib does the same NCACTIVATE toggle).
static void RefreshTitleBarWin10(HWND hwnd) {
    if (g_build >= 22000) return;
    bool active = (hwnd == GetActiveWindow()) && (hwnd == GetForegroundWindow());
    SendMessageW(hwnd, WM_NCACTIVATE, !active, 0);
    SendMessageW(hwnd, WM_NCACTIVATE, active, 0);
}

// ---------------------------------------------------------------------------
// System color hook (GetSysColor / GetSysColorBrush)
//
// Classic apps paint a lot with system colors (COLOR_WINDOW, COLOR_BTNFACE,
// COLOR_WINDOWTEXT, ...). Returning a dark palette from these two functions is
// the closest thing to a "universal" switch for them. Caption colors are left
// alone (the title bar is handled through DWM).
// ---------------------------------------------------------------------------

static bool MapSysColor(int idx, COLORREF* out) {
    switch (idx) {
        case COLOR_SCROLLBAR:
        case COLOR_MENU:
        case COLOR_WINDOW:
        case COLOR_INFOBK:
            *out = kCtlColor;
            return true;
        case COLOR_APPWORKSPACE:
        case COLOR_BTNFACE:  // == COLOR_3DFACE
            *out = kBgColor;
            return true;
        case COLOR_WINDOWFRAME:
        case COLOR_BTNSHADOW:  // == COLOR_3DSHADOW
            *out = kBorderColor;
            return true;
        case COLOR_MENUTEXT:
        case COLOR_WINDOWTEXT:
        case COLOR_BTNTEXT:
        case COLOR_INFOTEXT:
            *out = kTextColor;
            return true;
        case COLOR_GRAYTEXT:
            *out = kDisabledTextColor;
            return true;
        case COLOR_BTNHIGHLIGHT:  // == COLOR_3DHILIGHT
            *out = RGB(60, 60, 60);
            return true;
        case COLOR_3DDKSHADOW:
            *out = RGB(20, 20, 20);
            return true;
        case COLOR_3DLIGHT:
            *out = RGB(55, 55, 55);
            return true;
        case COLOR_HOTLIGHT:
            *out = kLinkTextColor;
            return true;
    }
    return false;
}

static inline bool SysColorHookActive() {
    return g_settings.hookSysColors && !g_isExplorer && !g_unloading;
}

using GetSysColor_t = decltype(&GetSysColor);
static GetSysColor_t GetSysColor_Original;
static DWORD WINAPI GetSysColor_Hook(int idx) {
    COLORREF c;
    if (SysColorHookActive() && MapSysColor(idx, &c)) return c;
    return GetSysColor_Original(idx);
}

static std::mutex g_brushMutex;
static HBRUSH g_sysBrushes[32];  // intentionally never freed: apps cache them

using GetSysColorBrush_t = decltype(&GetSysColorBrush);
static GetSysColorBrush_t GetSysColorBrush_Original;
static HBRUSH WINAPI GetSysColorBrush_Hook(int idx) {
    COLORREF c;
    if (SysColorHookActive() && idx >= 0 && idx < 32 && MapSysColor(idx, &c)) {
        std::lock_guard<std::mutex> lock(g_brushMutex);
        if (!g_sysBrushes[idx]) g_sysBrushes[idx] = CreateSolidBrush(c);
        if (g_sysBrushes[idx]) return g_sysBrushes[idx];
    }
    return GetSysColorBrush_Original(idx);
}

// ---------------------------------------------------------------------------
// Small helpers ported from darkmodelib
// ---------------------------------------------------------------------------

// Tooltips owned by a control (toolbar, list/tree view, tab, trackbar) are
// separate windows and need their own dark style.
static void ThemeTooltips(HWND hwnd, UINT getTooltipsMsg) {
    HWND tips = (HWND)SendMessageW(hwnd, getTooltipsMsg, 0, 0);
    if (tips && IsWindow(tips)) SetWindowTheme(tips, L"DarkMode_Explorer", nullptr);
}

// Dark mode draws a flat 1px border; the sunken client edge stays light.
static void ReplaceClientEdgeWithBorder(HWND hwnd) {
    LONG_PTR st = GetWindowLongPtrW(hwnd, GWL_STYLE);
    LONG_PTR ex = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    if (!(st & WS_BORDER) && !(ex & WS_EX_CLIENTEDGE)) return;

    bool changed = false;
    if (!(st & WS_BORDER)) {
        SetWindowLongPtrW(hwnd, GWL_STYLE, st | WS_BORDER);
        changed = true;
    }
    if (ex & WS_EX_CLIENTEDGE) {
        SetWindowLongPtrW(hwnd, GWL_EXSTYLE, ex & ~(LONG_PTR)WS_EX_CLIENTEDGE);
        changed = true;
    }
    if (changed) {
        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE |
                         SWP_FRAMECHANGED);
    }
}

// Repaints the outermost sunken/static edge of a window in dark colors.
// Used for Task Manager's graph frames, whose edges are drawn by the system
// with hardcoded light colors.
static void PaintDarkEdge(HWND hwnd) {
    LONG_PTR ex = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    int width = (ex & WS_EX_CLIENTEDGE) ? 2 : ((ex & WS_EX_STATICEDGE) ? 1 : 0);
    if (!width) return;

    HDC hdc = GetWindowDC(hwnd);
    if (!hdc) return;

    RECT rc;
    GetWindowRect(hwnd, &rc);
    OffsetRect(&rc, -rc.left, -rc.top);

    HBRUSH dc = (HBRUSH)GetStockObject(DC_BRUSH);
    SetDCBrushColor(hdc, kBorderColor);
    FrameRect(hdc, &rc, dc);
    if (width == 2) {
        InflateRect(&rc, -1, -1);
        SetDCBrushColor(hdc, kCtlColor);
        FrameRect(hdc, &rc, dc);
    }
    ReleaseDC(hwnd, hdc);
}

// List/tree view checkboxes live in a state image list and are always drawn
// light. Redraw them with the dark button theme (Windows 11 only).
static void DarkViewCheckboxes(HWND hwnd, HIMAGELIST himl, bool treeView,
                               bool treeExtended) {
    if (g_build < 22000 || !himl) return;

    HDC hdc = GetDC(nullptr);
    HTHEME theme = OpenThemeData(hwnd, L"DarkMode_Explorer::Button");
    if (!theme) {
        ReleaseDC(nullptr, hdc);
        return;
    }

    SIZE sz = {};
    GetThemePartSize(theme, hdc, BP_CHECKBOX, CBS_UNCHECKEDNORMAL, nullptr,
                     TS_DRAW, &sz);
    const RECT rc = {0, 0, sz.cx, sz.cy};

    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP color = CreateCompatibleBitmap(hdc, sz.cx, sz.cy);
    HBITMAP mask = CreateCompatibleBitmap(hdc, sz.cx, sz.cy);
    HGDIOBJ oldBmp = SelectObject(mdc, color);

    int idx = treeView ? 1 : 0;  // tree view state images start at index 1
    auto addState = [&](int state) {
        DrawThemeBackground(theme, mdc, BP_CHECKBOX, state, &rc, nullptr);
        SelectObject(mdc, oldBmp);  // bitmap must not be selected for the icon
        ICONINFO ii = {};
        ii.fIcon = TRUE;
        ii.hbmColor = color;
        ii.hbmMask = mask;
        HICON icon = CreateIconIndirect(&ii);
        if (icon) {
            ImageList_ReplaceIcon(himl, idx, icon);
            DestroyIcon(icon);
            idx++;
        }
        SelectObject(mdc, color);
    };

    addState(CBS_UNCHECKEDNORMAL);
    addState(CBS_CHECKEDNORMAL);

    if (treeView && treeExtended) {
        DWORD ex = TreeView_GetExtendedStyle(hwnd);
        // image list order: partial, dimmed, exclusion
        if (ex & TVS_EX_PARTIALCHECKBOXES) addState(CBS_MIXEDNORMAL);
        if (ex & TVS_EX_DIMMEDCHECKBOXES) addState(CBS_IMPLICITPRESSED);
        if (ex & TVS_EX_EXCLUSIONCHECKBOXES) addState(CBS_EXCLUDEDNORMAL);
    }

    SelectObject(mdc, oldBmp);
    DeleteObject(mask);
    DeleteObject(color);
    DeleteDC(mdc);
    CloseThemeData(theme);
    ReleaseDC(nullptr, hdc);
}

static void DarkListViewCheckboxes(HWND hwnd) {
    if (!(ListView_GetExtendedListViewStyle(hwnd) & LVS_EX_CHECKBOXES)) return;
    DarkViewCheckboxes(hwnd, ListView_GetImageList(hwnd, LVSIL_STATE), false,
                       false);
}

static void DarkTreeViewCheckboxes(HWND hwnd) {
    bool simple = (GetWindowLongPtrW(hwnd, GWL_STYLE) & TVS_CHECKBOXES) != 0;
    bool extended =
        (TreeView_GetExtendedStyle(hwnd) &
         (TVS_EX_PARTIALCHECKBOXES | TVS_EX_EXCLUSIONCHECKBOXES |
          TVS_EX_DIMMEDCHECKBOXES)) != 0;
    if (!simple && !extended) return;
    DarkViewCheckboxes(hwnd, TreeView_GetImageList(hwnd, TVSIL_STATE), true,
                       !simple && extended);
}

// SysLink items only follow WM_CTLCOLORSTATIC when they use default colors.
static void EnableSysLinkCtlColor(HWND hwnd) {
    LITEM item = {};
    item.iLink = 0;
    item.mask = LIF_ITEMINDEX | LIF_STATE;
    item.state = LIS_DEFAULTCOLORS;
    item.stateMask = LIS_DEFAULTCOLORS;
    while (SendMessageW(hwnd, LM_SETITEM, 0, (LPARAM)&item) == TRUE) item.iLink++;
}

// ---------------------------------------------------------------------------
// Subclass from any thread (technique from m417z's Notepad dark mode mod)
// ---------------------------------------------------------------------------

struct SubclassParam {
    SUBCLASSPROC proc;
    UINT_PTR id;
    DWORD_PTR ref;
    BOOL result;
};

static LRESULT CALLBACK SubclassInstallHookProc(int code, WPARAM wParam,
                                                LPARAM lParam) {
    if (code == HC_ACTION) {
        const CWPSTRUCT* cwp = (const CWPSTRUCT*)lParam;
        if (cwp->message == g_subclassMsg && cwp->wParam) {
            SubclassParam* p = (SubclassParam*)cwp->lParam;
            p->result = SetWindowSubclass(cwp->hwnd, p->proc, p->id, p->ref);
        }
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

static BOOL SetWindowSubclassFromAnyThread(HWND hwnd, SUBCLASSPROC proc,
                                           UINT_PTR id, DWORD_PTR ref) {
    DWORD tid = GetWindowThreadProcessId(hwnd, nullptr);
    if (!tid) return FALSE;

    if (tid == GetCurrentThreadId()) {
        return SetWindowSubclass(hwnd, proc, id, ref);
    }

    HHOOK hook =
        SetWindowsHookExW(WH_CALLWNDPROC, SubclassInstallHookProc, nullptr, tid);
    if (!hook) return FALSE;

    SubclassParam param = {proc, id, ref, FALSE};
    DWORD_PTR res = 0;
    SendMessageTimeoutW(hwnd, g_subclassMsg, TRUE, (LPARAM)&param,
                        SMTO_ABORTIFHUNG, 2000, &res);
    UnhookWindowsHookEx(hook);  // after this a late message can't touch param

    return param.result;
}

// ---------------------------------------------------------------------------
// UAH menu bar drawing (adzm/win32-darkmode, Notepad++)
// ---------------------------------------------------------------------------

#define WM_UAHDRAWMENU 0x0091
#define WM_UAHDRAWMENUITEM 0x0092

typedef union tagUAHMENUITEMMETRICS {
    struct {
        DWORD cx;
        DWORD cy;
    } rgsizeBar[2];
    struct {
        DWORD cx;
        DWORD cy;
    } rgsizePopup[4];
} UAHMENUITEMMETRICS;

typedef struct tagUAHMENUPOPUPMETRICS {
    DWORD rgcx[4];
    DWORD fUpdateMaxWidths : 2;
} UAHMENUPOPUPMETRICS;

typedef struct tagUAHMENU {
    HMENU hmenu;
    HDC hdc;
    DWORD dwFlags;
} UAHMENU;

typedef struct tagUAHMENUITEM {
    int iPosition;
    UAHMENUITEMMETRICS umim;
    UAHMENUPOPUPMETRICS umpm;
} UAHMENUITEM;

typedef struct UAHDRAWMENUITEM {
    DRAWITEMSTRUCT dis;
    UAHMENU um;
    UAHMENUITEM umi;
} UAHDRAWMENUITEM;

static HTHEME g_menuTheme = nullptr;

static bool UAHDarkModeWndProc(HWND hWnd, UINT message, WPARAM wParam,
                               LPARAM lParam, LRESULT* lr) {
    switch (message) {
        case WM_UAHDRAWMENU: {
            UAHMENU* pUDM = (UAHMENU*)lParam;
            RECT rc = {0};

            {
                MENUBARINFO mbi = {sizeof(mbi)};
                GetMenuBarInfo(hWnd, OBJID_MENU, 0, &mbi);

                RECT rcWindow;
                GetWindowRect(hWnd, &rcWindow);

                rc = mbi.rcBar;
                OffsetRect(&rc, -rcWindow.left, -rcWindow.top);
                rc.top -= 1;
            }

            if (!g_menuTheme) g_menuTheme = OpenThemeData(hWnd, L"Menu");

            DrawThemeBackground(g_menuTheme, pUDM->hdc, MENU_POPUPITEM,
                                MPI_NORMAL, &rc, nullptr);
            *lr = 0;
            return true;
        }
        case WM_UAHDRAWMENUITEM: {
            UAHDRAWMENUITEM* pUDMI = (UAHDRAWMENUITEM*)lParam;

            wchar_t menuString[256] = {0};
            MENUITEMINFOW mii = {sizeof(mii), MIIM_STRING};
            mii.dwTypeData = menuString;
            mii.cch = (sizeof(menuString) / 2) - 1;
            GetMenuItemInfoW(pUDMI->um.hmenu, pUDMI->umi.iPosition, TRUE, &mii);

            DWORD dwFlags = DT_CENTER | DT_SINGLELINE | DT_VCENTER;

            int iTextStateID = 0;
            int iBackgroundStateID = 0;
            if ((pUDMI->dis.itemState & ODS_INACTIVE) |
                (pUDMI->dis.itemState & ODS_DEFAULT)) {
                iTextStateID = MPI_NORMAL;
                iBackgroundStateID = MPI_NORMAL;
            }
            if (pUDMI->dis.itemState & ODS_HOTLIGHT) {
                iTextStateID = MPI_HOT;
                iBackgroundStateID = MPI_HOT;
            }
            if (pUDMI->dis.itemState & ODS_SELECTED) {
                iTextStateID = MPI_HOT;
                iBackgroundStateID = MPI_HOT;
            }
            if ((pUDMI->dis.itemState & ODS_GRAYED) ||
                (pUDMI->dis.itemState & ODS_DISABLED)) {
                iTextStateID = MPI_DISABLED;
                iBackgroundStateID = MPI_DISABLED;
            }
            if (pUDMI->dis.itemState & ODS_NOACCEL) dwFlags |= DT_HIDEPREFIX;

            if (!g_menuTheme) g_menuTheme = OpenThemeData(hWnd, L"Menu");

            DrawThemeBackground(g_menuTheme, pUDMI->um.hdc, MENU_POPUPITEM,
                                iBackgroundStateID, &pUDMI->dis.rcItem, nullptr);
            DrawThemeText(g_menuTheme, pUDMI->um.hdc, MENU_POPUPITEM,
                          iTextStateID, menuString, mii.cch, dwFlags, 0,
                          &pUDMI->dis.rcItem);
            *lr = 0;
            return true;
        }
        case WM_THEMECHANGED: {
            if (g_menuTheme) {
                CloseThemeData(g_menuTheme);
                g_menuTheme = nullptr;
            }
            return false;
        }
        default:
            return false;
    }
}

static void DrawMenuBarBottomLine(HWND hWnd) {
    MENUBARINFO mbi = {sizeof(mbi)};
    if (!GetMenuBarInfo(hWnd, OBJID_MENU, 0, &mbi)) return;

    RECT rcClient = {0};
    GetClientRect(hWnd, &rcClient);
    MapWindowPoints(hWnd, nullptr, (POINT*)&rcClient, 2);

    RECT rcWindow = {0};
    GetWindowRect(hWnd, &rcWindow);
    OffsetRect(&rcClient, -rcWindow.left, -rcWindow.top);

    RECT rcLine = rcClient;
    rcLine.bottom = rcLine.top;
    rcLine.top--;

    HDC hdc = GetWindowDC(hWnd);
    SetDCBrushColor(hdc, RGB(60, 60, 60));
    FillRect(hdc, &rcLine, (HBRUSH)GetStockObject(DC_BRUSH));
    ReleaseDC(hWnd, hdc);
}

// ---------------------------------------------------------------------------
// Window subclass (framed top-level windows + #32770 dialogs + Task Manager)
// ---------------------------------------------------------------------------

static LRESULT CALLBACK WindowSubclassProc(HWND hwnd, UINT msg, WPARAM wParam,
                                           LPARAM lParam, UINT_PTR id,
                                           DWORD_PTR /*ref*/) {
    if (msg == g_subclassMsg && !wParam) {
        RemoveWindowSubclass(hwnd, WindowSubclassProc, id);
        if (!(GetWindowLongW(hwnd, GWL_STYLE) & WS_CHILD)) {
            SetTitleBarDark(hwnd, FALSE);
        }
        AllowDarkMode(hwnd, false);
        RedrawWindow(hwnd, nullptr, nullptr,
                     RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
        return 0;
    }

    if (g_settings.darkMenuBar) {
        LRESULT lr = 0;
        if (UAHDarkModeWndProc(hwnd, msg, wParam, lParam, &lr)) return lr;
    }

    switch (msg) {
        case WM_PAINT: {
            // Standard shell dialogs such as Win+R can paint their own
            // background during WM_PAINT, after WM_ERASEBKGND has already
            // returned. Paint the dialog client area ourselves first. Child
            // controls (Edit, Button, Static, etc.) remain separate windows
            // and will paint normally on top of it.
            wchar_t cls[64] = L"";
            if (g_isExplorer && GetClassNameW(hwnd, cls, ARRAYSIZE(cls)) &&
                !_wcsicmp(cls, L"#32770") && g_settings.recolorDialogs) {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                if (hdc) {
                    RECT rc;
                    GetClientRect(hwnd, &rc);
                    FillRect(hdc, &rc, g_bgBrush);
                    EndPaint(hwnd, &ps);
                    return 0;
                }
            }
            break;
        }

        case WM_PRINTCLIENT: {
            wchar_t cls[64] = L"";
            if (g_isExplorer && GetClassNameW(hwnd, cls, ARRAYSIZE(cls)) &&
                !_wcsicmp(cls, L"#32770") && g_settings.recolorDialogs) {
                RECT rc;
                GetClientRect(hwnd, &rc);
                FillRect((HDC)wParam, &rc, g_bgBrush);
                return 0;
            }
            break;
        }

        case WM_NCACTIVATE:
        case WM_NCPAINT: {
            bool isChild = (GetWindowLongW(hwnd, GWL_STYLE) & WS_CHILD) != 0;

            // Task Manager: sunken/static edges of child windows (the graph
            // frames) are drawn by the system in light colors. Repaint them.
            if (msg == WM_NCPAINT && g_isTaskManager && isChild) {
                LRESULT lr = DefSubclassProc(hwnd, msg, wParam, lParam);
                PaintDarkEdge(hwnd);
                return lr;
            }

            // GetMenu() returns the control ID for child windows, so only
            // look at real top-level windows here.
            if (!isChild && g_settings.darkMenuBar && GetMenu(hwnd)) {
                LRESULT lr = DefSubclassProc(hwnd, msg, wParam, lParam);
                DrawMenuBarBottomLine(hwnd);
                return lr;
            }
            break;
        }

        case WM_CTLCOLORDLG:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORMSGBOX:
        case WM_CTLCOLORSCROLLBAR: {
            if (!g_settings.recolorDialogs) break;
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, kTextColor);
            SetBkColor(hdc, kBgColor);
            return (LRESULT)g_bgBrush;
        }

        case WM_CTLCOLORSTATIC: {
            if (!g_settings.recolorDialogs) break;
            HDC hdc = (HDC)wParam;
            HWND ctl = (HWND)lParam;

            // darkmodelib: disabled static text is dimmed, SysLink uses the
            // link color (items must use LIS_DEFAULTCOLORS to honor this).
            bool enabled = !ctl || IsWindowEnabled(ctl);
            wchar_t cls[32] = L"";
            if (ctl) GetClassNameW(ctl, cls, ARRAYSIZE(cls));
            bool isLink = !_wcsicmp(cls, WC_LINK);

            SetTextColor(hdc, !enabled ? kDisabledTextColor
                                       : (isLink ? kLinkTextColor : kTextColor));
            SetBkColor(hdc, kBgColor);
            return (LRESULT)g_bgBrush;
        }

        case WM_CTLCOLOREDIT: {
            if (!g_settings.recolorDialogs) break;
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, kTextColor);
            SetBkColor(hdc, kCtlColor);
            // Do NOT select a font here: the control already selected its own
            // font into this DC, and replacing it shrinks the text.
            return (LRESULT)g_ctlBrush;
        }

        case WM_CTLCOLORLISTBOX: {
            if (!g_settings.recolorDialogs) break;
            HDC hdc = (HDC)wParam;
            HWND ctl = (HWND)lParam;
            SetTextColor(hdc, kTextColor);

            // darkmodelib: the list of a combo box uses the window
            // background; standalone list boxes use the control background
            // while enabled and the window background when disabled.
            bool inCombo = ctl && (GetWindowLongW(ctl, GWL_STYLE) & LBS_COMBOBOX);
            bool useCtl = !inCombo && ctl && IsWindowEnabled(ctl);
            SetBkColor(hdc, useCtl ? kCtlColor : kBgColor);
            return (LRESULT)(useCtl ? g_ctlBrush : g_bgBrush);
        }

        case WM_NOTIFY: {
            if (!g_settings.recolorDialogs) break;

            NMHDR* nm = (NMHDR*)lParam;
            if (!nm || nm->code != NM_CUSTOMDRAW) break;

            wchar_t cls[64] = L"";
            if (!GetClassNameW(nm->hwndFrom, cls, ARRAYSIZE(cls))) break;

            if (!_wcsicmp(cls, TOOLBARCLASSNAMEW)) {
                NMTBCUSTOMDRAW* tb = (NMTBCUSTOMDRAW*)lParam;
                switch (tb->nmcd.dwDrawStage) {
                    case CDDS_PREPAINT:
                        FillRect(tb->nmcd.hdc, &tb->nmcd.rc, g_bgBrush);
                        return CDRF_NOTIFYITEMDRAW;
                    case CDDS_ITEMPREPAINT:
                        tb->clrText = kTextColor;
                        tb->clrTextHighlight = kTextColor;
                        tb->clrBtnFace = kCtlColor;
                        tb->clrBtnHighlight = kBorderColor;
                        tb->clrHighlightHotTrack = kBorderColor;
                        return TBCDRF_USECDCOLORS;
                }
                break;
            }

            if (!_wcsicmp(cls, L"SysListView32")) {
                NMLVCUSTOMDRAW* lv = (NMLVCUSTOMDRAW*)lParam;
                switch (lv->nmcd.dwDrawStage) {
                    case CDDS_PREPAINT:
                        return CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
                    case CDDS_ITEMPREPAINT:
                        lv->clrText = kTextColor;
                        lv->clrTextBk = kBgColor;
                        return CDRF_NOTIFYSUBITEMDRAW;
                    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
                        lv->clrText = kTextColor;
                        lv->clrTextBk = kBgColor;
                        return CDRF_NEWFONT;
                }
                break;
            }

            if (!_wcsicmp(cls, L"SysHeader32")) {
                NMCUSTOMDRAW* cd = (NMCUSTOMDRAW*)lParam;
                switch (cd->dwDrawStage) {
                    case CDDS_PREPAINT:
                        return CDRF_NOTIFYITEMDRAW;
                    case CDDS_ITEMPREPAINT:
                        SetDCBrushColor(cd->hdc, kCtlColor);
                        FillRect(cd->hdc, &cd->rc,
                                 (HBRUSH)GetStockObject(DC_BRUSH));
                        SetBkMode(cd->hdc, TRANSPARENT);
                        SetTextColor(cd->hdc, kTextColor);
                        return CDRF_NEWFONT;
                }
                break;
            }
            break;
        }

        case WM_ERASEBKGND: {
            wchar_t cls[64] = L"";
            if (!GetClassNameW(hwnd, cls, ARRAYSIZE(cls))) break;
            bool isDlg = !_wcsicmp(cls, L"#32770");

            // Task Manager's main window and several of its child windows
            // paint their own light background. Do not let that white erase
            // pass through.
            if ((g_isTaskManager) ||
                (g_isExplorer && isDlg) ||
                (isDlg && g_settings.recolorDialogs) ||
                (!isDlg && g_settings.darkWindowBackground)) {
                RECT rc;
                GetClientRect(hwnd, &rc);
                FillRect((HDC)wParam, &rc, g_bgBrush);
                return 1;
            }
            break;
        }

        case WM_NCDESTROY: {
            {
                std::lock_guard<std::mutex> lock(g_mutex);
                g_subclassed.erase(hwnd);
            }
            RemoveWindowSubclass(hwnd, WindowSubclassProc, id);
            break;
        }
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

// ---------------------------------------------------------------------------
// Group box subclass (custom paint)
//
// BS_GROUPBOX draws an etched frame using hardcoded light 3D colors, which is
// the "white outline" around the panels of the classic Task Manager. Paint a
// flat dark frame and light caption text instead.
// ---------------------------------------------------------------------------

static LRESULT CALLBACK GroupBoxSubclassProc(HWND hwnd, UINT msg, WPARAM wParam,
                                             LPARAM lParam, UINT_PTR id,
                                             DWORD_PTR /*ref*/) {
    if (msg == g_subclassMsg && !wParam) {
        RemoveWindowSubclass(hwnd, GroupBoxSubclassProc, id);
        HWND parent = GetParent(hwnd);
        InvalidateRect(hwnd, nullptr, TRUE);
        if (parent) InvalidateRect(parent, nullptr, TRUE);
        return 0;
    }

    switch (msg) {
        case WM_ERASEBKGND:
            // The interior must stay transparent (child controls and the
            // parent background show through); everything is done in WM_PAINT.
            return TRUE;

        case WM_SETTEXT: {
            LRESULT lr = DefSubclassProc(hwnd, msg, wParam, lParam);
            // Let the parent erase the old caption.
            HWND parent = GetParent(hwnd);
            if (parent) {
                RECT r;
                GetWindowRect(hwnd, &r);
                MapWindowPoints(nullptr, parent, (POINT*)&r, 2);
                InvalidateRect(parent, &r, TRUE);
            }
            return lr;
        }

        case WM_PAINT:
        case WM_PRINTCLIENT: {
            PAINTSTRUCT ps;
            HDC hdc = (msg == WM_PAINT) ? BeginPaint(hwnd, &ps) : (HDC)wParam;
            if (!hdc) return 0;

            RECT rc;
            GetClientRect(hwnd, &rc);

            HFONT font = (HFONT)SendMessageW(hwnd, WM_GETFONT, 0, 0);
            HGDIOBJ oldFont = SelectObject(
                hdc, font ? font : GetStockObject(DEFAULT_GUI_FONT));

            wchar_t text[256] = L"";
            GetWindowTextW(hwnd, text, ARRAYSIZE(text));
            bool hasText = text[0] != L'\0';

            TEXTMETRICW tm = {};
            GetTextMetricsW(hdc, &tm);
            int th = tm.tmHeight;

            UINT flags = DT_SINGLELINE | DT_LEFT | DT_TOP | DT_NOCLIP;
            if (SendMessageW(hwnd, WM_QUERYUISTATE, 0, 0) & UISF_HIDEACCEL)
                flags |= DT_HIDEPREFIX;

            RECT textRc = {rc.left + 9, rc.top, rc.left + 9, rc.top + th};
            if (hasText) {
                RECT calc = {0, 0, 0, 0};
                DrawTextW(hdc, text, -1, &calc,
                          DT_CALCRECT | DT_SINGLELINE | DT_LEFT |
                              (flags & DT_HIDEPREFIX));
                textRc.right = textRc.left + (calc.right - calc.left);
            }
            RECT gap = {textRc.left - 2, rc.top, textRc.right + 2, rc.top + th};

            // Frame (flat, 1px), leaving a gap for the caption.
            RECT frame = rc;
            frame.top += th / 2;
            int saved = SaveDC(hdc);
            if (hasText) {
                ExcludeClipRect(hdc, gap.left, gap.top, gap.right, gap.bottom);
            }
            SetDCBrushColor(hdc, kBorderColor);
            FrameRect(hdc, &frame, (HBRUSH)GetStockObject(DC_BRUSH));
            RestoreDC(hdc, saved);

            if (hasText) {
                // Caption background = whatever the parent paints there.
                if (FAILED(DrawThemeParentBackground(hwnd, hdc, &gap))) {
                    FillRect(hdc, &gap, g_bgBrush);
                }
                bool enabled = IsWindowEnabled(hwnd) != FALSE;
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, enabled ? kTextColor : kDisabledTextColor);
                DrawTextW(hdc, text, -1, &textRc, flags);
            }

            SelectObject(hdc, oldFont);
            if (msg == WM_PAINT) EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_NCDESTROY: {
            {
                std::lock_guard<std::mutex> lock(g_mutex);
                g_subclassed.erase(hwnd);
            }
            RemoveWindowSubclass(hwnd, GroupBoxSubclassProc, id);
            break;
        }
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

// ---------------------------------------------------------------------------
// Status bar subclass (custom paint, adapted from Process Hacker)
// Only used when the native dark theme is not available/enabled.
// ---------------------------------------------------------------------------

struct StatusCtx {
    HTHEME theme;
};

static void PaintStatusBar(HWND hwnd, StatusCtx* ctx) {
    RECT rc;
    GetClientRect(hwnd, &rc);

    PAINTSTRUCT ps;
    if (!BeginPaint(hwnd, &ps)) return;

    HDC hdc = CreateCompatibleDC(ps.hdc);
    HBITMAP bmp = CreateCompatibleBitmap(ps.hdc, rc.right, rc.bottom);
    HGDIOBJ oldBmp = SelectObject(hdc, bmp);

    HFONT font = (HFONT)SendMessageW(hwnd, WM_GETFONT, 0, 0);
    if (!font) font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    HGDIOBJ oldFont = SelectObject(hdc, font);

    SetDCBrushColor(hdc, kStatusColor);
    FillRect(hdc, &rc, (HBRUSH)GetStockObject(DC_BRUSH));

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, kTextColor);

    INT coords[128];
    int count = (int)SendMessageW(hwnd, SB_GETPARTS, ARRAYSIZE(coords),
                                  (LPARAM)coords);
    for (int i = 0; i < count; i++) {
        RECT r = {0};
        if (!SendMessageW(hwnd, SB_GETRECT, i, (LPARAM)&r)) continue;

        wchar_t text[512] = L"";
        LRESULT res = SendMessageW(hwnd, SB_GETTEXTW, i, (LPARAM)text);
        if (HIWORD(res) & SBT_OWNERDRAW) continue;

        const wchar_t* t = text;
        UINT flags = DT_SINGLELINE | DT_VCENTER | DT_HIDEPREFIX;
        if (t[0] == L'\t' && t[1] == L'\t') {
            flags |= DT_RIGHT;
            t += 2;
        } else if (t[0] == L'\t') {
            flags |= DT_CENTER;
            t += 1;
        } else {
            flags |= DT_LEFT;
        }

        r.left += 4;
        r.right -= 4;
        DrawTextW(hdc, t, -1, &r, flags);
    }

    if (GetWindowLongW(hwnd, GWL_STYLE) & SBARS_SIZEGRIP) {
        RECT grip;
        grip.left = rc.right - GetSystemMetrics(SM_CXHSCROLL);
        grip.top = rc.bottom - GetSystemMetrics(SM_CYVSCROLL);
        grip.right = rc.right;
        grip.bottom = rc.bottom;
        if (ctx->theme) {
            DrawThemeBackground(ctx->theme, hdc, SP_GRIPPER, 0, &grip, &grip);
        } else {
            DrawFrameControl(hdc, &grip, DFC_SCROLL, DFCS_SCROLLSIZEGRIP);
        }
    }

    BitBlt(ps.hdc, 0, 0, rc.right, rc.bottom, hdc, 0, 0, SRCCOPY);

    SelectObject(hdc, oldFont);
    SelectObject(hdc, oldBmp);
    DeleteObject(bmp);
    DeleteDC(hdc);
    EndPaint(hwnd, &ps);
}

static LRESULT CALLBACK StatusSubclassProc(HWND hwnd, UINT msg, WPARAM wParam,
                                           LPARAM lParam, UINT_PTR id,
                                           DWORD_PTR ref) {
    StatusCtx* ctx = (StatusCtx*)ref;

    if (msg == g_subclassMsg && !wParam) {
        RemoveWindowSubclass(hwnd, StatusSubclassProc, id);
        if (ctx->theme) CloseThemeData(ctx->theme);
        delete ctx;
        InvalidateRect(hwnd, nullptr, TRUE);
        return 0;
    }

    switch (msg) {
        case WM_ERASEBKGND:
            if (g_settings.darkStatusBar && !SendMessageW(hwnd, SB_ISSIMPLE, 0, 0))
                return TRUE;
            break;

        case WM_PAINT:
            if (g_settings.darkStatusBar && !SendMessageW(hwnd, SB_ISSIMPLE, 0, 0)) {
                PaintStatusBar(hwnd, ctx);
                return 0;
            }
            break;

        case WM_THEMECHANGED:
            if (ctx->theme) CloseThemeData(ctx->theme);
            ctx->theme = OpenThemeData(hwnd, VSCLASS_STATUS);
            break;

        case WM_NCDESTROY: {
            {
                std::lock_guard<std::mutex> lock(g_mutex);
                g_subclassed.erase(hwnd);
            }
            RemoveWindowSubclass(hwnd, StatusSubclassProc, id);
            if (ctx->theme) CloseThemeData(ctx->theme);
            delete ctx;
            return DefSubclassProc(hwnd, msg, wParam, lParam);
        }
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

// ---------------------------------------------------------------------------
// Tab control subclass (custom paint)
// Only used when the native dark theme is not available/enabled.
// ---------------------------------------------------------------------------

static LRESULT CALLBACK TabSubclassProc(HWND hwnd, UINT msg, WPARAM wParam,
                                        LPARAM lParam, UINT_PTR id,
                                        DWORD_PTR /*ref*/) {
    if (msg == g_subclassMsg && !wParam) {
        RemoveWindowSubclass(hwnd, TabSubclassProc, id);
        InvalidateRect(hwnd, nullptr, TRUE);
        return 0;
    }

    switch (msg) {
        case WM_ERASEBKGND:
            if (g_settings.recolorDialogs) return TRUE;
            break;

        case WM_PAINT:
        case WM_PRINTCLIENT: {
            if (!g_settings.recolorDialogs) break;

            PAINTSTRUCT ps;
            HDC hdc = (msg == WM_PAINT) ? BeginPaint(hwnd, &ps) : (HDC)wParam;
            if (!hdc) return 0;

            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, g_bgBrush);

            HFONT font = (HFONT)SendMessageW(hwnd, WM_GETFONT, 0, 0);
            HGDIOBJ oldFont = SelectObject(
                hdc, font ? font : GetStockObject(DEFAULT_GUI_FONT));
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, kTextColor);

            HBRUSH dc = (HBRUSH)GetStockObject(DC_BRUSH);

            // pane border
            RECT pane = rc;
            TabCtrl_AdjustRect(hwnd, FALSE, &pane);
            InflateRect(&pane, 2, 2);
            SetDCBrushColor(hdc, kBorderColor);
            FrameRect(hdc, &pane, dc);

            int count = TabCtrl_GetItemCount(hwnd);
            int sel = TabCtrl_GetCurSel(hwnd);
            for (int i = 0; i < count; i++) {
                RECT r;
                if (!TabCtrl_GetItemRect(hwnd, i, &r)) continue;

                SetDCBrushColor(hdc, i == sel ? kBgColor : kCtlColor);
                FillRect(hdc, &r, dc);
                SetDCBrushColor(hdc, kBorderColor);
                FrameRect(hdc, &r, dc);

                wchar_t text[128] = L"";
                TCITEMW it = {};
                it.mask = TCIF_TEXT;
                it.pszText = text;
                it.cchTextMax = ARRAYSIZE(text);
                SendMessageW(hwnd, TCM_GETITEMW, i, (LPARAM)&it);

                DrawTextW(hdc, text, -1, &r,
                          DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_HIDEPREFIX);
            }

            SelectObject(hdc, oldFont);
            if (msg == WM_PAINT) EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_NCDESTROY: {
            {
                std::lock_guard<std::mutex> lock(g_mutex);
                g_subclassed.erase(hwnd);
            }
            RemoveWindowSubclass(hwnd, TabSubclassProc, id);
            break;
        }
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

// ---------------------------------------------------------------------------
// Theming a single window
// ---------------------------------------------------------------------------

static void InstallWindowSubclass(HWND hwnd) {
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if (!g_subclassed.insert(hwnd).second) return;
    }
    if (!SetWindowSubclassFromAnyThread(hwnd, WindowSubclassProc, kIdWindow, 0)) {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_subclassed.erase(hwnd);
    }
}

static void InstallGroupBoxSubclass(HWND hwnd) {
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if (!g_subclassed.insert(hwnd).second) return;
    }
    if (!SetWindowSubclassFromAnyThread(hwnd, GroupBoxSubclassProc, kIdGroup,
                                        0)) {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_subclassed.erase(hwnd);
    } else {
        InvalidateRect(hwnd, nullptr, TRUE);
    }
}

static void InstallStatusSubclass(HWND hwnd) {
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if (!g_subclassed.insert(hwnd).second) return;
    }
    StatusCtx* ctx = new StatusCtx{OpenThemeData(hwnd, VSCLASS_STATUS)};
    if (!SetWindowSubclassFromAnyThread(hwnd, StatusSubclassProc, kIdStatus,
                                        (DWORD_PTR)ctx)) {
        if (ctx->theme) CloseThemeData(ctx->theme);
        delete ctx;
        std::lock_guard<std::mutex> lock(g_mutex);
        g_subclassed.erase(hwnd);
    } else {
        InvalidateRect(hwnd, nullptr, FALSE);
    }
}

static void InstallTabSubclass(HWND hwnd) {
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if (!g_subclassed.insert(hwnd).second) return;
    }
    if (!SetWindowSubclassFromAnyThread(hwnd, TabSubclassProc, kIdTab, 0)) {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_subclassed.erase(hwnd);
    } else {
        InvalidateRect(hwnd, nullptr, TRUE);
    }
}

// Inside explorer.exe only dialogs (Run, Properties, ...) and the
// autocomplete dropdown are themed; taskbar/desktop/File Explorer are not.
static bool IsAllowedExplorerWindow(HWND hwnd) {
    HWND root = GetAncestor(hwnd, GA_ROOT);
    if (!root) root = hwnd;
    wchar_t c[48];
    if (!GetClassNameW(root, c, ARRAYSIZE(c))) return false;
    return !_wcsicmp(c, L"#32770") || !_wcsicmp(c, L"Auto-Suggest Dropdown");
}

static bool IsDarkTargetWindow(HWND hwnd) {
    if (!IsWindow(hwnd)) return false;
    if (g_isTaskManager) return true;
    if (g_isExplorer) return IsAllowedExplorerWindow(hwnd);
    return true;
}

static void ThemeWindow(HWND hwnd) {
    if (g_unloading || !IsWindow(hwnd)) return;

    wchar_t cls[64];
    if (!GetClassNameW(hwnd, cls, ARRAYSIZE(cls))) return;

    if (!IsDarkTargetWindow(hwnd)) return;

    // Diagnostic: shows up in the Windhawk log window. Useful to see which
    // windows an installer / app actually creates and whether they get themed.
    Wh_Log(L"ThemeWindow hwnd=%p class=%s style=%08X parent=%p", hwnd, cls,
           (unsigned)GetWindowLongW(hwnd, GWL_STYLE), GetParent(hwnd));

    LONG style = GetWindowLongW(hwnd, GWL_STYLE);
    bool topLevel = !(style & WS_CHILD);
    bool framed = topLevel && (style & WS_CAPTION) == WS_CAPTION;
    bool isDialog = !_wcsicmp(cls, L"#32770");
    bool native = UseNativeTheme();

    if (framed) SetTitleBarDark(hwnd, TRUE);
    AllowDarkMode(hwnd, true);

    if (g_isTaskManager) {
        // Give classic/common controls a dark class background. This is
        // especially important for the intermediate Task Manager panes.
        SetClassLongPtrW(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)g_bgBrush);
    }

    if (g_isTaskManager && topLevel) {
        SetClassLongPtrW(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)g_bgBrush);
        InvalidateRect(hwnd, nullptr, TRUE);
    } else if (g_isExplorer && isDialog) {
        SetClassLongPtrW(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)g_bgBrush);
        InvalidateRect(hwnd, nullptr, TRUE);
        UpdateWindow(hwnd);
    }

    const wchar_t* theme = nullptr;

    // When true, the window is deliberately left in classic (unthemed) mode
    // (or themed manually below) so the final SetWindowTheme call must not
    // overwrite it with a DarkMode_* theme.
    bool keepClassic = false;

    // Set when a custom-paint subclass is still needed for this control.
    bool useCustomPaint = !native;

    // Set for BS_GROUPBOX buttons (custom-painted by GroupBoxSubclassProc).
    bool installGroup = false;

    if (!_wcsicmp(cls, L"Button")) {
        // Push buttons, check boxes and radios all use the Explorer dark
        // style (darkmodelib additionally subclasses check boxes on
        // Windows 10; not included here).
        theme = L"DarkMode_Explorer";
        if ((style & 0x0F) == BS_GROUPBOX && g_settings.recolorDialogs) {
            installGroup = true;
            Wh_Log(L"GroupBox detected hwnd=%p", hwnd);
        }
    } else if (!_wcsicmp(cls, L"ScrollBar") || !_wcsicmp(cls, L"msctls_updown32") ||
               !_wcsicmp(cls, L"tooltips_class32")) {
        theme = L"DarkMode_Explorer";
    } else if (!_wcsicmp(cls, L"ListBox") || !_wcsicmp(cls, L"ComboLBox")) {
        theme = L"DarkMode_Explorer";  // dark scroll bars
    } else if (!_wcsicmp(cls, L"Edit")) {
        bool hasScroll = (style & (WS_VSCROLL | WS_HSCROLL)) != 0;
        if (hasScroll) {
            theme = L"DarkMode_Explorer";  // dark scroll bars
        } else {
            theme = native ? L"DarkMode_DarkTheme" : L"CFD";
        }
        if (g_isExplorer) {
            // Standard EDIT controls get their background from the parent's
            // WM_CTLCOLOREDIT brush. Classic mode avoids the shell theme
            // painting a white edit rectangle over that brush.
            SetWindowTheme(hwnd, L"", L"");
            keepClassic = true;
            InvalidateRect(hwnd, nullptr, TRUE);
            UpdateWindow(hwnd);
        }
    } else if (!_wcsicmp(cls, L"ComboBox")) {
        // darkmodelib: dark scroll bar for the drop-down list, light arrow
        // for the box itself, and no initial selection highlight.
        COMBOBOXINFO cbi = {sizeof(cbi)};
        if (GetComboBoxInfo(hwnd, &cbi) && cbi.hwndList) {
            SetWindowTheme(cbi.hwndList, L"DarkMode_Explorer", nullptr);
            if ((style & CBS_DROPDOWNLIST) == CBS_SIMPLE) {
                ReplaceClientEdgeWithBorder(cbi.hwndList);
            }
        }
        theme = native ? L"DarkMode_DarkTheme" : L"CFD";
        if ((style & CBS_DROPDOWNLIST) != CBS_DROPDOWNLIST) {
            SendMessageW(hwnd, CB_SETEDITSEL, 0, 0);
        }
    } else if (!_wcsicmp(cls, L"ComboBoxEx32")) {
        theme = L"CFD";
    } else if (!_wcsicmp(cls, L"SysHeader32")) {
        theme = native ? L"DarkMode_DarkTheme" : L"ItemsView";
        if (g_isTaskManager) {
            // Classic header so the NM_CUSTOMDRAW dark fill is used.
            SetWindowTheme(hwnd, L"", L"");
            keepClassic = true;
            InvalidateRect(hwnd, nullptr, TRUE);
        }
    } else if (!_wcsicmp(cls, L"SysListView32")) {
        theme = L"Explorer";  // AllowDarkModeForWindow above makes it dark
        if (g_isTaskManager) {
            // Classic ListView drawing is more reliable in Task Manager than
            // relying on its application theme.
            SetWindowTheme(hwnd, L"", L"");
            keepClassic = true;
        }
        if (g_settings.recolorLists) {
            ListView_SetBkColor(hwnd, kBgColor);
            ListView_SetTextBkColor(hwnd, kBgColor);
            ListView_SetTextColor(hwnd, kTextColor);
            ListView_SetExtendedListViewStyle(
                hwnd, ListView_GetExtendedListViewStyle(hwnd) | LVS_EX_DOUBLEBUFFER);
            ThemeTooltips(hwnd, LVM_GETTOOLTIPS);
            DarkListViewCheckboxes(hwnd);
            if (!g_isTaskManager) ReplaceClientEdgeWithBorder(hwnd);
            InvalidateRect(hwnd, nullptr, TRUE);
        }
    } else if (!_wcsicmp(cls, L"SysTreeView32")) {
        theme = L"DarkMode_Explorer";
        if (g_settings.recolorLists) {
            TreeView_SetBkColor(hwnd, kBgColor);
            TreeView_SetTextColor(hwnd, kTextColor);
            // Explorer-style hot tracking, as darkmodelib does for dark trees
            LONG_PTR st = GetWindowLongPtrW(hwnd, GWL_STYLE);
            if (!(st & TVS_TRACKSELECT)) {
                SetWindowLongPtrW(hwnd, GWL_STYLE, st | TVS_TRACKSELECT);
            }
            ThemeTooltips(hwnd, TVM_GETTOOLTIPS);
            DarkTreeViewCheckboxes(hwnd);
        }
    } else if (!_wcsnicmp(cls, L"RichEdit", 8)) {
        theme = nullptr;  // handled manually: only the scroll bar sub-class
        keepClassic = true;
        if (g_settings.recolorLists) {
            // darkmodelib: read-only rich edits use the window background
            bool readOnly = (style & ES_READONLY) != 0;
            SendMessageW(hwnd, EM_SETBKGNDCOLOR, 0,
                         (LPARAM)(readOnly ? kBgColor : kCtlColor));
            CHARFORMATW cf = {};
            cf.cbSize = sizeof(cf);
            cf.dwMask = CFM_COLOR;
            cf.crTextColor = kTextColor;
            SendMessageW(hwnd, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM)&cf);
            SetWindowTheme(hwnd, nullptr, L"DarkMode_Explorer::ScrollBar");
            ReplaceClientEdgeWithBorder(hwnd);
        }
    } else if (!_wcsicmp(cls, L"msctls_trackbar32")) {
        theme = L"DarkMode_Explorer";
        LONG_PTR st = GetWindowLongPtrW(hwnd, GWL_STYLE);
        if (!(st & TBS_TRANSPARENTBKGND)) {
            SetWindowLongPtrW(hwnd, GWL_STYLE, st | TBS_TRANSPARENTBKGND);
            SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                             SWP_NOACTIVATE | SWP_FRAMECHANGED);
        }
        ThemeTooltips(hwnd, TBM_GETTOOLTIPS);
    } else if (!_wcsicmp(cls, L"SysDateTimePick32")) {
        theme = L"DarkMode_Explorer";
    } else if (!_wcsicmp(cls, L"SysMonthCal32")) {
        // darkmodelib: colors can only be set with visual styles disabled.
        SetWindowTheme(hwnd, L"", L"");
        keepClassic = true;
        MonthCal_SetColor(hwnd, MCSC_BACKGROUND, kBgColor);
        MonthCal_SetColor(hwnd, MCSC_MONTHBK, kCtlColor);
        MonthCal_SetColor(hwnd, MCSC_TEXT, kTextColor);
        MonthCal_SetColor(hwnd, MCSC_TITLEBK, kAccentBlue);
        MonthCal_SetColor(hwnd, MCSC_TITLETEXT, kTextColor);
        MonthCal_SetColor(hwnd, MCSC_TRAILINGTEXT, kDisabledTextColor);
    } else if (!_wcsicmp(cls, WC_LINK)) {
        EnableSysLinkCtlColor(hwnd);
    } else if (!_wcsicmp(cls, L"SysTabControl32")) {
        if (native) {
            theme = L"DarkMode_DarkTheme";
        } else {
            theme = L"DarkMode_Explorer";
            if (g_settings.recolorDialogs) InstallTabSubclass(hwnd);
        }
        ThemeTooltips(hwnd, TCM_GETTOOLTIPS);
    } else if (!_wcsicmp(cls, L"msctls_progress32")) {
        // Marquee bars (and non-native systems) use classic mode so the
        // colors apply; azure fill matches darkmodelib on recent Windows 11.
        if (native && !(style & PBS_MARQUEE)) {
            SetWindowTheme(hwnd, L"DarkMode_CopyEngine", nullptr);
        } else {
            SetWindowTheme(hwnd, L"", L"");
            SendMessageW(hwnd, PBM_SETBKCOLOR, 0, (LPARAM)kCtlColor);
            SendMessageW(hwnd, PBM_SETBARCOLOR, 0,
                         (LPARAM)(native ? RGB(0x60, 0xCD, 0xFF)
                                         : RGB(6, 176, 37)));
        }
        keepClassic = true;
    } else if (!_wcsicmp(cls, TOOLBARCLASSNAMEW)) {
        SetWindowTheme(hwnd, L"", L"");  // classic mode so custom-draw colors apply
        keepClassic = true;
        // darkmodelib: hide the light separator line above panel toolbars
        COLORSCHEME scheme = {};
        scheme.dwSize = sizeof(scheme);
        scheme.clrBtnHighlight = kBgColor;
        scheme.clrBtnShadow = kBgColor;
        SendMessageW(hwnd, TB_SETCOLORSCHEME, 0, (LPARAM)&scheme);
        ThemeTooltips(hwnd, TB_GETTOOLTIPS);
    } else if (!_wcsicmp(cls, REBARCLASSNAMEW)) {
        ThemeTooltips(hwnd, RB_GETTOOLTIPS);
    } else if (!_wcsicmp(cls, STATUSCLASSNAMEW)) {
        if (native && g_settings.darkStatusBar) {
            theme = L"DarkMode_DarkTheme";
            useCustomPaint = false;
        }
    } else if (style & (WS_VSCROLL | WS_HSCROLL)) {
        theme = L"DarkMode_Explorer";  // generic window with scrollbars
    }

    if (theme && !keepClassic) SetWindowTheme(hwnd, theme, nullptr);

    // Task Manager has intermediate container windows between the main
    // frame and the ListView/Header controls. WM_NOTIFY (including
    // NM_CUSTOMDRAW) is delivered to those parents, not necessarily to the
    // top-level window. Subclass every Task Manager child so we can actually
    // intercept and recolor those notifications.
    if (installGroup) {
        InstallGroupBoxSubclass(hwnd);
    } else if (g_isTaskManager || isDialog || framed) {
        InstallWindowSubclass(hwnd);
    } else if (!_wcsicmp(cls, STATUSCLASSNAMEW) && g_settings.darkStatusBar &&
               useCustomPaint) {
        InstallStatusSubclass(hwnd);
    }
}

// ---------------------------------------------------------------------------
// Per-thread hook: themes every window after WM_CREATE / WM_INITDIALOG
// ---------------------------------------------------------------------------

static LRESULT CALLBACK CwpRetProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION && !g_unloading) {
        const CWPRETSTRUCT* c = (const CWPRETSTRUCT*)lParam;
        if (c->message == WM_CREATE || c->message == WM_INITDIALOG) {
            ThemeWindow(c->hwnd);
        }
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

static void EnsureThreadHook(DWORD tid) {
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_unloading || g_hooks.count(tid)) return;
    HHOOK h = SetWindowsHookExW(WH_CALLWNDPROCRET, CwpRetProc, nullptr, tid);
    if (h) g_hooks[tid] = h;
}

static inline void EnsureCurrentThreadHook() {
    EnsureThreadHook(GetCurrentThreadId());
}

// ---------------------------------------------------------------------------
// Function hooks (only used to attach the per-thread hook early)
// ---------------------------------------------------------------------------

using CreateWindowExW_t = decltype(&CreateWindowExW);
static CreateWindowExW_t CreateWindowExW_Original;
static HWND WINAPI CreateWindowExW_Hook(DWORD ex, LPCWSTR cls, LPCWSTR name,
                                        DWORD style, int x, int y, int w, int h,
                                        HWND parent, HMENU menu, HINSTANCE inst,
                                        LPVOID param) {
    EnsureCurrentThreadHook();
    return CreateWindowExW_Original(ex, cls, name, style, x, y, w, h, parent,
                                    menu, inst, param);
}

using CreateWindowExA_t = decltype(&CreateWindowExA);
static CreateWindowExA_t CreateWindowExA_Original;
static HWND WINAPI CreateWindowExA_Hook(DWORD ex, LPCSTR cls, LPCSTR name,
                                        DWORD style, int x, int y, int w, int h,
                                        HWND parent, HMENU menu, HINSTANCE inst,
                                        LPVOID param) {
    EnsureCurrentThreadHook();
    return CreateWindowExA_Original(ex, cls, name, style, x, y, w, h, parent,
                                    menu, inst, param);
}

using DialogBoxParamW_t = decltype(&DialogBoxParamW);
static DialogBoxParamW_t DialogBoxParamW_Original;
static INT_PTR WINAPI DialogBoxParamW_Hook(HINSTANCE i, LPCWSTR t, HWND p,
                                           DLGPROC d, LPARAM l) {
    EnsureCurrentThreadHook();
    return DialogBoxParamW_Original(i, t, p, d, l);
}

using DialogBoxIndirectParamW_t = decltype(&DialogBoxIndirectParamW);
static DialogBoxIndirectParamW_t DialogBoxIndirectParamW_Original;
static INT_PTR WINAPI DialogBoxIndirectParamW_Hook(HINSTANCE i,
                                                   LPCDLGTEMPLATEW t, HWND p,
                                                   DLGPROC d, LPARAM l) {
    EnsureCurrentThreadHook();
    return DialogBoxIndirectParamW_Original(i, t, p, d, l);
}

using CreateDialogParamW_t = decltype(&CreateDialogParamW);
static CreateDialogParamW_t CreateDialogParamW_Original;
static HWND WINAPI CreateDialogParamW_Hook(HINSTANCE i, LPCWSTR t, HWND p,
                                           DLGPROC d, LPARAM l) {
    EnsureCurrentThreadHook();
    return CreateDialogParamW_Original(i, t, p, d, l);
}

using CreateDialogIndirectParamW_t = decltype(&CreateDialogIndirectParamW);
static CreateDialogIndirectParamW_t CreateDialogIndirectParamW_Original;
static HWND WINAPI CreateDialogIndirectParamW_Hook(HINSTANCE i,
                                                   LPCDLGTEMPLATEW t, HWND p,
                                                   DLGPROC d, LPARAM l) {
    EnsureCurrentThreadHook();
    return CreateDialogIndirectParamW_Original(i, t, p, d, l);
}

using MessageBoxW_t = decltype(&MessageBoxW);
static MessageBoxW_t MessageBoxW_Original;
static int WINAPI MessageBoxW_Hook(HWND hw, LPCWSTR text, LPCWSTR caption,
                                   UINT type) {
    EnsureCurrentThreadHook();
    return MessageBoxW_Original(hw, text, caption, type);
}

using MessageBoxIndirectW_t = decltype(&MessageBoxIndirectW);
static MessageBoxIndirectW_t MessageBoxIndirectW_Original;
static int WINAPI MessageBoxIndirectW_Hook(const MSGBOXPARAMSW* p) {
    EnsureCurrentThreadHook();
    return MessageBoxIndirectW_Original(p);
}

// ANSI variants: many installers (e.g. non-Unicode NSIS builds) use these.
using DialogBoxParamA_t = decltype(&DialogBoxParamA);
static DialogBoxParamA_t DialogBoxParamA_Original;
static INT_PTR WINAPI DialogBoxParamA_Hook(HINSTANCE i, LPCSTR t, HWND p,
                                           DLGPROC d, LPARAM l) {
    EnsureCurrentThreadHook();
    return DialogBoxParamA_Original(i, t, p, d, l);
}

using DialogBoxIndirectParamA_t = decltype(&DialogBoxIndirectParamA);
static DialogBoxIndirectParamA_t DialogBoxIndirectParamA_Original;
static INT_PTR WINAPI DialogBoxIndirectParamA_Hook(HINSTANCE i,
                                                   LPCDLGTEMPLATEA t, HWND p,
                                                   DLGPROC d, LPARAM l) {
    EnsureCurrentThreadHook();
    return DialogBoxIndirectParamA_Original(i, t, p, d, l);
}

using CreateDialogParamA_t = decltype(&CreateDialogParamA);
static CreateDialogParamA_t CreateDialogParamA_Original;
static HWND WINAPI CreateDialogParamA_Hook(HINSTANCE i, LPCSTR t, HWND p,
                                           DLGPROC d, LPARAM l) {
    EnsureCurrentThreadHook();
    return CreateDialogParamA_Original(i, t, p, d, l);
}

using CreateDialogIndirectParamA_t = decltype(&CreateDialogIndirectParamA);
static CreateDialogIndirectParamA_t CreateDialogIndirectParamA_Original;
static HWND WINAPI CreateDialogIndirectParamA_Hook(HINSTANCE i,
                                                   LPCDLGTEMPLATEA t, HWND p,
                                                   DLGPROC d, LPARAM l) {
    EnsureCurrentThreadHook();
    return CreateDialogIndirectParamA_Original(i, t, p, d, l);
}

using MessageBoxA_t = decltype(&MessageBoxA);
static MessageBoxA_t MessageBoxA_Original;
static int WINAPI MessageBoxA_Hook(HWND hw, LPCSTR text, LPCSTR caption,
                                   UINT type) {
    EnsureCurrentThreadHook();
    return MessageBoxA_Original(hw, text, caption, type);
}

// ---------------------------------------------------------------------------
// TaskDialog theming (experimental)
//
// TaskDialogs (comctl32 v6) are drawn by DirectUI using the "TaskDialog"
// visual style class. Track the HTHEMEs opened for that class and replace the
// panel backgrounds / text colors when they are drawn.
// ---------------------------------------------------------------------------

static std::mutex g_tdMutex;
static std::set<HTHEME> g_taskDialogThemes;

static bool ClassListHasTaskDialog(LPCWSTR list) {
    if (!list) return false;
    wchar_t buf[128];
    size_t i = 0;
    for (; list[i] && i < ARRAYSIZE(buf) - 1; i++) buf[i] = towlower(list[i]);
    buf[i] = 0;
    return wcsstr(buf, L"taskdialog") != nullptr;
}

static void TrackTaskDialogTheme(HTHEME t, LPCWSTR classList) {
    if (!t || !ClassListHasTaskDialog(classList)) return;
    std::lock_guard<std::mutex> lock(g_tdMutex);
    g_taskDialogThemes.insert(t);
    Wh_Log(L"TaskDialog theme opened: %p (%s)", t, classList);
}

static bool IsTaskDialogTheme(HTHEME t) {
    if (!t || !g_settings.darkTaskDialogs || g_unloading) return false;
    std::lock_guard<std::mutex> lock(g_tdMutex);
    return g_taskDialogThemes.count(t) != 0;
}

// Returns true if the part was painted here.
static bool PaintTaskDialogPart(HDC hdc, int part, LPCRECT rc, LPCRECT clip) {
    HBRUSH brush = nullptr;
    switch (part) {
        case TDLG_PRIMARYPANEL:
            brush = g_bgBrush;
            break;
        case TDLG_SECONDARYPANEL:
        case TDLG_FOOTNOTEAREA:
        case TDLG_EXPANDEDFOOTERAREA:
            brush = g_ctlBrush;
            break;
        default:
            Wh_Log(L"TaskDialog part not themed: %d", part);
            return false;
    }
    if (!brush || !rc) return false;
    RECT r = *rc;
    if (clip && !IntersectRect(&r, rc, clip)) return true;
    FillRect(hdc, &r, brush);
    return true;
}

using OpenThemeData_t = HTHEME(WINAPI*)(HWND, LPCWSTR);
static OpenThemeData_t OpenThemeData_Original;
static HTHEME WINAPI OpenThemeData_Hook(HWND hwnd, LPCWSTR list) {
    HTHEME t = OpenThemeData_Original(hwnd, list);
    TrackTaskDialogTheme(t, list);
    return t;
}

using OpenThemeDataForDpi_t = HTHEME(WINAPI*)(HWND, LPCWSTR, UINT);
static OpenThemeDataForDpi_t OpenThemeDataForDpi_Original;
static HTHEME WINAPI OpenThemeDataForDpi_Hook(HWND hwnd, LPCWSTR list,
                                              UINT dpi) {
    HTHEME t = OpenThemeDataForDpi_Original(hwnd, list, dpi);
    TrackTaskDialogTheme(t, list);
    return t;
}

using OpenThemeDataEx_t = HTHEME(WINAPI*)(HWND, LPCWSTR, DWORD);
static OpenThemeDataEx_t OpenThemeDataEx_Original;
static HTHEME WINAPI OpenThemeDataEx_Hook(HWND hwnd, LPCWSTR list,
                                          DWORD flags) {
    HTHEME t = OpenThemeDataEx_Original(hwnd, list, flags);
    TrackTaskDialogTheme(t, list);
    return t;
}

using CloseThemeData_t = HRESULT(WINAPI*)(HTHEME);
static CloseThemeData_t CloseThemeData_Original;
static HRESULT WINAPI CloseThemeData_Hook(HTHEME t) {
    {
        std::lock_guard<std::mutex> lock(g_tdMutex);
        g_taskDialogThemes.erase(t);
    }
    return CloseThemeData_Original(t);
}

using DrawThemeBackground_t = HRESULT(WINAPI*)(HTHEME, HDC, int, int, LPCRECT,
                                               LPCRECT);
static DrawThemeBackground_t DrawThemeBackground_Original;
static HRESULT WINAPI DrawThemeBackground_Hook(HTHEME t, HDC hdc, int part,
                                               int state, LPCRECT rc,
                                               LPCRECT clip) {
    if (IsTaskDialogTheme(t) && PaintTaskDialogPart(hdc, part, rc, clip)) {
        return S_OK;
    }
    return DrawThemeBackground_Original(t, hdc, part, state, rc, clip);
}

using DrawThemeBackgroundEx_t = HRESULT(WINAPI*)(HTHEME, HDC, int, int,
                                                 LPCRECT, const DTBGOPTS*);
static DrawThemeBackgroundEx_t DrawThemeBackgroundEx_Original;
static HRESULT WINAPI DrawThemeBackgroundEx_Hook(HTHEME t, HDC hdc, int part,
                                                 int state, LPCRECT rc,
                                                 const DTBGOPTS* opts) {
    if (IsTaskDialogTheme(t) && PaintTaskDialogPart(hdc, part, rc, nullptr)) {
        return S_OK;
    }
    return DrawThemeBackgroundEx_Original(t, hdc, part, state, rc, opts);
}

using DrawThemeText_t = HRESULT(WINAPI*)(HTHEME, HDC, int, int, LPCWSTR, int,
                                         DWORD, DWORD, LPCRECT);
static DrawThemeText_t DrawThemeText_Original;
static HRESULT WINAPI DrawThemeText_Hook(HTHEME t, HDC hdc, int part, int state,
                                         LPCWSTR text, int cch, DWORD flags,
                                         DWORD flags2, LPCRECT rc) {
    if (IsTaskDialogTheme(t) && text && rc) {
        COLORREF old = SetTextColor(hdc, kTextColor);
        int oldMode = SetBkMode(hdc, TRANSPARENT);
        RECT r = *rc;
        DrawTextW(hdc, text, cch, &r, flags);
        SetBkMode(hdc, oldMode);
        SetTextColor(hdc, old);
        return S_OK;
    }
    return DrawThemeText_Original(t, hdc, part, state, text, cch, flags, flags2,
                                  rc);
}

using DrawThemeTextEx_t = HRESULT(WINAPI*)(HTHEME, HDC, int, int, LPCWSTR, int,
                                           DWORD, LPRECT, const DTTOPTS*);
static DrawThemeTextEx_t DrawThemeTextEx_Original;
static HRESULT WINAPI DrawThemeTextEx_Hook(HTHEME t, HDC hdc, int part,
                                           int state, LPCWSTR text, int cch,
                                           DWORD flags, LPRECT rc,
                                           const DTTOPTS* opts) {
    if (IsTaskDialogTheme(t)) {
        DTTOPTS o = {};
        if (opts) {
            memcpy(&o, opts, opts->dwSize < sizeof(o) ? opts->dwSize : sizeof(o));
        }
        o.dwSize = sizeof(o);
        o.dwFlags |= DTT_TEXTCOLOR;
        o.crText = kTextColor;
        return DrawThemeTextEx_Original(t, hdc, part, state, text, cch, flags,
                                        rc, &o);
    }
    return DrawThemeTextEx_Original(t, hdc, part, state, text, cch, flags, rc,
                                    opts);
}

using GetThemeColor_t = HRESULT(WINAPI*)(HTHEME, int, int, int, COLORREF*);
static GetThemeColor_t GetThemeColor_Original;
static HRESULT WINAPI GetThemeColor_Hook(HTHEME t, int part, int state,
                                         int prop, COLORREF* out) {
    if (out && prop == TMT_TEXTCOLOR && IsTaskDialogTheme(t)) {
        *out = kTextColor;
        return S_OK;
    }
    return GetThemeColor_Original(t, part, state, prop, out);
}

static void HookUxThemeFunction(HMODULE ux, const char* name, void* hook,
                                void** original) {
    void* fn = (void*)GetProcAddress(ux, name);
    if (fn) Wh_SetFunctionHook(fn, hook, original);
}

// ---------------------------------------------------------------------------
// Existing windows
// ---------------------------------------------------------------------------

static BOOL CALLBACK EnumChildThemeProc(HWND hwnd, LPARAM) {
    ThemeWindow(hwnd);
    return TRUE;
}

static BOOL CALLBACK EnumTopThemeProc(HWND hwnd, LPARAM) {
    DWORD pid = 0;
    DWORD tid = GetWindowThreadProcessId(hwnd, &pid);
    if (pid != GetCurrentProcessId()) return TRUE;

    if (g_isExplorer && !IsAllowedExplorerWindow(hwnd)) return TRUE;

    EnsureThreadHook(tid);

    ThemeWindow(hwnd);
    EnumChildWindows(hwnd, EnumChildThemeProc, 0);

    // Windows 10 only repaints the caption after an activation change.
    if ((GetWindowLongW(hwnd, GWL_STYLE) & WS_CAPTION) == WS_CAPTION) {
        RefreshTitleBarWin10(hwnd);
    }

    RedrawWindow(hwnd, nullptr, nullptr,
                 RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
    return TRUE;
}

// ---------------------------------------------------------------------------
// Mod entry points
// ---------------------------------------------------------------------------

BOOL Wh_ModInit() {
    LoadSettings();

    DWORD major = 0, minor = 0, build = 0;
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    auto getVer = ntdll ? (RtlGetNtVersionNumbers_t)GetProcAddress(
                              ntdll, "RtlGetNtVersionNumbers")
                        : nullptr;
    if (getVer) getVer(&major, &minor, &build);
    g_build = build & ~0xF0000000;
    if (g_build < 17763) {
        Wh_Log(L"Unsupported Windows build %lu", g_build);
        return FALSE;
    }
    g_darkThemeAvailable = DetectDarkThemeAvailable();

    {
        wchar_t exe[MAX_PATH] = L"";
        GetModuleFileNameW(nullptr, exe, ARRAYSIZE(exe));
        const wchar_t* base = wcsrchr(exe, L'\\');
        g_isExplorer = !_wcsicmp(base ? base + 1 : exe, L"explorer.exe");
        g_isTaskManager = !_wcsicmp(base ? base + 1 : exe, L"Taskmgr.exe");
    }

    HMODULE ux =
        LoadLibraryExW(L"uxtheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!ux) return FALSE;
    auto ord135 = GetProcAddress(ux, MAKEINTRESOURCEA(135));
    if (g_build < 18362)
        g_AllowDarkModeForApp = (AllowDarkModeForApp_t)ord135;
    else
        g_SetPreferredAppMode = (SetPreferredAppMode_t)ord135;
    g_AllowDarkModeForWindow =
        (AllowDarkModeForWindow_t)GetProcAddress(ux, MAKEINTRESOURCEA(133));
    g_FlushMenuThemes =
        (FlushMenuThemes_t)GetProcAddress(ux, MAKEINTRESOURCEA(136));
    g_RefreshImmersiveColorPolicyState =
        (RefreshImmersiveColorPolicyState_t)GetProcAddress(
            ux, MAKEINTRESOURCEA(104));

    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (user32) {
        g_SetWindowCompositionAttribute =
            (SetWindowCompositionAttribute_t)GetProcAddress(
                user32, "SetWindowCompositionAttribute");
    }

    g_subclassMsg = RegisterWindowMessageW(
        L"Windhawk_SetWindowSubclassFromAnyThread_universal-win32-dark-mode");
    g_bgBrush = CreateSolidBrush(kBgColor);
    g_ctlBrush = CreateSolidBrush(kCtlColor);
    g_unloading = false;

    ApplyAppMode(true);

    Wh_SetFunctionHook((void*)CreateWindowExW, (void*)CreateWindowExW_Hook,
                       (void**)&CreateWindowExW_Original);
    Wh_SetFunctionHook((void*)CreateWindowExA, (void*)CreateWindowExA_Hook,
                       (void**)&CreateWindowExA_Original);
    Wh_SetFunctionHook((void*)DialogBoxParamW, (void*)DialogBoxParamW_Hook,
                       (void**)&DialogBoxParamW_Original);
    Wh_SetFunctionHook((void*)DialogBoxIndirectParamW,
                       (void*)DialogBoxIndirectParamW_Hook,
                       (void**)&DialogBoxIndirectParamW_Original);
    Wh_SetFunctionHook((void*)CreateDialogParamW, (void*)CreateDialogParamW_Hook,
                       (void**)&CreateDialogParamW_Original);
    Wh_SetFunctionHook((void*)CreateDialogIndirectParamW,
                       (void*)CreateDialogIndirectParamW_Hook,
                       (void**)&CreateDialogIndirectParamW_Original);
    Wh_SetFunctionHook((void*)MessageBoxW, (void*)MessageBoxW_Hook,
                       (void**)&MessageBoxW_Original);
    Wh_SetFunctionHook((void*)MessageBoxIndirectW,
                       (void*)MessageBoxIndirectW_Hook,
                       (void**)&MessageBoxIndirectW_Original);

    // ANSI variants (non-Unicode installers and older apps)
    Wh_SetFunctionHook((void*)DialogBoxParamA, (void*)DialogBoxParamA_Hook,
                       (void**)&DialogBoxParamA_Original);
    Wh_SetFunctionHook((void*)DialogBoxIndirectParamA,
                       (void*)DialogBoxIndirectParamA_Hook,
                       (void**)&DialogBoxIndirectParamA_Original);
    Wh_SetFunctionHook((void*)CreateDialogParamA, (void*)CreateDialogParamA_Hook,
                       (void**)&CreateDialogParamA_Original);
    Wh_SetFunctionHook((void*)CreateDialogIndirectParamA,
                       (void*)CreateDialogIndirectParamA_Hook,
                       (void**)&CreateDialogIndirectParamA_Original);
    Wh_SetFunctionHook((void*)MessageBoxA, (void*)MessageBoxA_Hook,
                       (void**)&MessageBoxA_Original);

    // System color palette (checked at call time against the setting)
    Wh_SetFunctionHook((void*)GetSysColor, (void*)GetSysColor_Hook,
                       (void**)&GetSysColor_Original);
    Wh_SetFunctionHook((void*)GetSysColorBrush, (void*)GetSysColorBrush_Hook,
                       (void**)&GetSysColorBrush_Original);

    // TaskDialog theming (experimental; checked at call time)
    HookUxThemeFunction(ux, "OpenThemeData", (void*)OpenThemeData_Hook,
                        (void**)&OpenThemeData_Original);
    HookUxThemeFunction(ux, "OpenThemeDataForDpi",
                        (void*)OpenThemeDataForDpi_Hook,
                        (void**)&OpenThemeDataForDpi_Original);
    HookUxThemeFunction(ux, "OpenThemeDataEx", (void*)OpenThemeDataEx_Hook,
                        (void**)&OpenThemeDataEx_Original);
    HookUxThemeFunction(ux, "CloseThemeData", (void*)CloseThemeData_Hook,
                        (void**)&CloseThemeData_Original);
    HookUxThemeFunction(ux, "DrawThemeBackground",
                        (void*)DrawThemeBackground_Hook,
                        (void**)&DrawThemeBackground_Original);
    HookUxThemeFunction(ux, "DrawThemeBackgroundEx",
                        (void*)DrawThemeBackgroundEx_Hook,
                        (void**)&DrawThemeBackgroundEx_Original);
    HookUxThemeFunction(ux, "DrawThemeText", (void*)DrawThemeText_Hook,
                        (void**)&DrawThemeText_Original);
    HookUxThemeFunction(ux, "DrawThemeTextEx", (void*)DrawThemeTextEx_Hook,
                        (void**)&DrawThemeTextEx_Original);
    HookUxThemeFunction(ux, "GetThemeColor", (void*)GetThemeColor_Hook,
                        (void**)&GetThemeColor_Original);

    EnumWindows(EnumTopThemeProc, 0);

    return TRUE;
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    ApplyAppMode(true);
    EnumWindows(EnumTopThemeProc, 0);
}

void Wh_ModUninit() {
    g_unloading = true;

    std::map<DWORD, HHOOK> hooks;
    std::set<HWND> subclassed;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        hooks.swap(g_hooks);
        subclassed = g_subclassed;
    }
    for (auto& kv : hooks) UnhookWindowsHookEx(kv.second);

    // Each subclass proc removes itself (on its own thread) on this message.
    for (HWND hwnd : subclassed) {
        if (IsWindow(hwnd)) {
            DWORD_PTR result;
            SendMessageTimeoutW(hwnd, g_subclassMsg, FALSE, 0,
                                SMTO_ABORTIFHUNG | SMTO_BLOCK, 500, &result);
        }
    }
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_subclassed.clear();
    }

    ApplyAppMode(false);

    if (g_menuTheme) {
        CloseThemeData(g_menuTheme);
        g_menuTheme = nullptr;
    }
    if (g_bgBrush) DeleteObject(g_bgBrush);
    if (g_ctlBrush) DeleteObject(g_ctlBrush);
    g_bgBrush = g_ctlBrush = nullptr;
    // g_sysBrushes are intentionally leaked: apps may still hold them.
}
