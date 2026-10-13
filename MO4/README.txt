CSOPESY - Semi-Major Output 2: Desktop-Style OS Mock-up
=======================================================
Group 5:  Alvarez | Sanidad | Viray | Quijano
Version date: 2026-10-09

ENTRY POINT
-----------
  src/main.cpp   (contains int main())

Stack: GLFW 3.4 + OpenGL 3 + Dear ImGui 1.91.9b. Both libraries are bundled
in external/, so no downloads or package installs are needed. Fonts are
embedded in src/fonts/, so the .exe needs no extra files.


HOW TO BUILD AND RUN
--------------------
Requirements: a C++17 compiler and CMake 3.16 or newer.

A) Visual Studio 2022 (recommended for the recorded test cases)
   1. Install the "Desktop development with C++" workload (includes CMake).
   2. File > Open > Folder...  and pick this folder (the one with CMakeLists.txt).
   3. Wait for "CMake generation finished" in the Output window.
   4. In the green Run dropdown on the toolbar, pick  csopesy_desktop.exe.
   5. Press F5 (or the green Run button).

B) Visual Studio Code
   1. Install the "C/C++" and "CMake Tools" extensions.
   2. Open this folder, pick a kit (MSVC or MinGW) when asked.
   3. Click "Build", then the Run (play) button in the status bar.

C) CLion: open the folder, then Run 'csopesy_desktop'.

D) Command line (any OS)
       cmake -S . -B build
       cmake --build build --config Release
   Then run  build/Release/csopesy_desktop.exe  (Visual Studio generator)
          or build/csopesy_desktop(.exe)        (MinGW / Linux / macOS)

   Linux needs the X11/GL dev packages, e.g. on Ubuntu:
       sudo apt install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl-dev

A prebuilt Windows executable is in bin/ as a fallback (64-bit, no DLLs needed).

Command-line option:  --skip-boot   starts directly on the desktop.


USING THE OS
------------
Boot      BIOS memory test -> CSOPESY splash/loading screen -> desktop.
          Press any key or click to skip the boot sequence.

Desktop   Procedural wallpaper drawn with ImGui draw commands (4 choices,
          animated), real-time clock in the top-right corner, desktop icons
          (double-click to open), right-click menu (wallpaper, apps, power).

Taskbar   Start button (app list + power options)
          File Explorer | Terminal | Task Manager | Settings buttons
            - click to open; click again to minimize; click to restore
            - underline = running, long accent underline = focused
          Tray: VOL (volume slider), NET (network info),
                clock/date (click for calendar), PWR (shut down / restart)

Apps
  File Explorer  Placeholder file system: Quick access + drives sidebar,
                 Back/Forward/Up, clickable breadcrumb path, search,
                 Icons/Details views, double-click folders to open and
                 .txt files to read them.
  Terminal       The MO3 marquee console as a window. Commands:
                 help, start_marquee, stop_marquee, set_text <text>,
                 set_speed <ms> (10-2000), clear, date, ver, exit.
                 Up/Down arrows recall previous commands.
  Task Manager   Processes tab: Apps + Background processes with PID, status,
                 CPU, Memory, Disk, Network (dummy values, heat-map shading,
                 sortable columns, End task, Run new task, update speed).
                 Performance tab: CPU / Memory / Disk / Ethernet graphs.
                 Minimized apps show as "Suspended" (0% CPU).
  Settings       Wallpaper, accent colour, 12/24-hour clock, seconds,
                 FPS counter, full screen (also F11), volume, About page.

Shutting down
  PWR button (or Start > Power options) > Shut down. The app plays a
  shutdown screen, closes every app and exits main() normally.
  The window's X button and Alt+F4 do NOT force-quit: they open the same
  Power Options dialog.


PARAMETERS THAT CAN BE CHANGED WHILE RUNNING (no recompiling)
-------------------------------------------------------------
  Wallpaper / animation / accent colour ....... Settings > Personalization
  12h or 24h clock, show seconds .............. Settings > Date & Time
  FPS counter, full screen, volume ............ Settings > System
  Marquee text and refresh speed .............. Terminal: set_text, set_speed
  Task Manager refresh rate ................... Task Manager: Update speed
  Window size ................................. drag the window edges (min 960x600)


PROJECT STRUCTURE
-----------------
  src/main.cpp          window, OpenGL context, ImGui setup, render loop
  src/Compositor.*      per-frame draw order + boot/desktop/shutdown states
  src/Boot.*            BIOS, splash and power-off screens
  src/Wallpaper.*       procedural wallpapers
  src/Taskbar.*         start menu, app buttons, system tray, calendar
  src/App.*             base class for windowed apps (open/close/minimize, PID)
  src/OS.*              shared OS state, clock helpers, theme
  src/Icons.*           vector icons drawn with ImDrawList
  src/apps/*.cpp        File Explorer, Terminal, Task Manager, Settings
  src/fonts/*.h         Roboto and Cousine fonts compiled into the program
  external/glfw         GLFW 3.4 source (zlib licence)
  external/imgui        Dear ImGui 1.91.9b source (MIT licence)
