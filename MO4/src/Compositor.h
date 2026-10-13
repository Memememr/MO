// CSOPESY Desktop OS Emulator - Group 5
// Compositor.h - decides what is drawn each frame, and in which order.
//
// Like the window manager / compositor of a real OS, it runs once per frame:
//   1. wallpaper + clock      (background draw list - always the bottom layer)
//   2. desktop icons          (full-screen window kept at the back)
//   3. application windows    (in focus order, managed by Dear ImGui)
//   4. taskbar                (forced to the front every frame)
//   5. modal dialogs, toasts and fade overlays (foreground)
// It also runs the boot -> desktop -> shutdown state machine.
#pragma once

#include "OS.h"

class Compositor {
public:
    Compositor(OSState& os, bool skipBoot);
    void Frame();   // build one frame of UI (call between ImGui::NewFrame and ImGui::Render)

private:
    enum class Phase { Bios, Splash, Desktop, ShuttingDown, Restarting };

    void SetPhase(Phase p);
    void EnterDesktop();
    void DrawDesktop();
    void DrawDesktopLayer();   // wallpaper, clock, status label
    void DrawDesktopIcons();
    void DrawPowerDialog();
    void DrawToasts();
    bool AnyInputPressed() const;

    OSState& os;
    Phase    phase      = Phase::Bios;
    double   phaseStart = 0.0;
    int      selectedIcon = -1;
};
