// CSOPESY Desktop OS Emulator - Group 5
// OS.h - shared state of the desktop "operating system".
//
// Every subsystem (compositor, taskbar, apps) reads and writes this one struct.
// Nothing here is persisted to disk: the OS boots fresh every run.
#pragma once

#include "imgui.h"
#include "imgui_internal.h"   // ImMin / ImMax / ImClamp helpers

#include <memory>
#include <string>
#include <vector>

enum class IconType {
    Folder, Terminal, TaskManager, Settings, Power,
    Computer, Drive, File, TextFile, Image, Music, Video, Archive, Code
};

// User-adjustable parameters. All of these can be changed at runtime from the
// Settings app (or the Task Manager / Terminal) without recompiling.
struct Settings {
    int    wallpaper        = 0;     // 0 Rolling Hills, 1 Sunset, 2 Retro Grid, 3 Midnight Gradient
    bool   animateWallpaper = true;  // drifting clouds / scrolling grid
    ImVec4 accent           = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
    bool   clock24h         = false;
    bool   showSeconds      = true;
    bool   showFps          = false;
    float  volume           = 0.65f;
    bool   muted            = false;
};

struct Toast {
    std::string title;
    std::string text;
    double      start = 0.0;
};

struct Fonts {
    ImFont* ui        = nullptr;  // Roboto 17px - default UI font
    ImFont* uiLarge   = nullptr;  // Roboto 24px - headings
    ImFont* uiHuge    = nullptr;  // Roboto 44px - big numbers
    ImFont* mono      = nullptr;  // Cousine 16px - terminal, BIOS
    ImFont* monoLarge = nullptr;  // Cousine 22px - marquee band
};

class App;

struct OSState {
    Settings settings;
    Fonts    fonts;

    std::vector<std::unique_ptr<App>> apps;   // every installed application
    App* activeApp = nullptr;                  // app whose window has focus (nullptr = desktop)

    double now          = 0.0;   // seconds since program start (glfwGetTime)
    float  dt           = 0.0f;  // seconds since previous frame
    double desktopStart = 0.0;   // when the desktop finished booting
    int    nextPid      = 1240;  // dummy PID counter for the Task Manager
    float  taskbarHeight = 56.0f;

    std::vector<Toast> toasts;
    std::string glRenderer;
    std::string glVersion;

    // Requests raised by UI code and handled by the compositor / main loop.
    bool requestPowerMenu = false;
    bool requestShutdown  = false;
    bool requestRestart   = false;
    bool toggleFullscreen = false;
    bool isFullscreen     = false;
    bool poweredOff       = false;   // set by the compositor when shutdown finishes; ends the main loop

    void   Notify(const std::string& title, const std::string& text);
    App*   FindApp(const std::string& exe) const;
    ImU32  Accent(float alpha = 1.0f) const;
    ImVec2 WorkArea() const;   // display size minus the taskbar
};

// Local time formatted with strftime, e.g. FormatLocalTime("%I:%M %p").
std::string FormatLocalTime(const char* fmt);

// Time string honouring the 24h / seconds settings.
std::string ClockString(const Settings& s);

// Dark theme with the given accent colour (re-run whenever the accent changes).
void ApplyTheme(const ImVec4& accent);

// Right-aligns the next text item inside the current column / region.
void TextRightAligned(const char* text);
