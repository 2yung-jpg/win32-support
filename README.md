# win32-support
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
