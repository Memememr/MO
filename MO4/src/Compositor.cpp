// CSOPESY Desktop OS Emulator - Group 5
// Compositor.cpp - per-frame scene composition and the power state machine.
#include "Compositor.h"

#include "App.h"
#include "Boot.h"
#include "Icons.h"
#include "Taskbar.h"
#include "Wallpaper.h"
#include "imgui_internal.h"   // BringWindowToDisplayBack

#include <string>

Compositor::Compositor(OSState& os_, bool skipBoot) : os(os_) {
    if (skipBoot) EnterDesktop();
    else SetPhase(Phase::Bios);
}

void Compositor::SetPhase(Phase p) {
    phase      = p;
    phaseStart = os.now;
}

void Compositor::EnterDesktop() {
    SetPhase(Phase::Desktop);
    os.desktopStart = os.now;
    os.Notify("Welcome to CSOPESY OS", "Open an app from the taskbar or the desktop.");
}

bool Compositor::AnyInputPressed() const {
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) return true;
    for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_NamedKey_END; ++k)
        if (k != ImGuiKey_F11 && ImGui::IsKeyPressed((ImGuiKey)k, false)) return true;
    return false;
}

void Compositor::Frame() {
    if (ImGui::IsKeyPressed(ImGuiKey_F11, false)) os.toggleFullscreen = true;
    const float t = (float)(os.now - phaseStart);

    switch (phase) {
    case Phase::Bios:
        DrawBios(os, t);
        if (AnyInputPressed()) EnterDesktop();            // skip the whole boot sequence
        else if (t >= kBiosDuration) SetPhase(Phase::Splash);
        break;

    case Phase::Splash:
        DrawSplash(os, t);
        if (t >= kSplashDuration || AnyInputPressed()) EnterDesktop();
        break;

    case Phase::Desktop:
        DrawDesktop();
        if (os.requestShutdown) { os.requestShutdown = false; SetPhase(Phase::ShuttingDown); }
        if (os.requestRestart)  { os.requestRestart = false;  SetPhase(Phase::Restarting); }
        break;

    case Phase::ShuttingDown:
    case Phase::Restarting: {
        const bool restart = phase == Phase::Restarting;
        DrawDesktopLayer();
        DrawPowerOff(os, t, restart);
        if (t >= kShutdownDuration) {
            for (auto& a : os.apps) a->Close();   // "terminate all processes"
            os.activeApp = nullptr;
            os.toasts.clear();
            if (restart) SetPhase(Phase::Bios);
            else os.poweredOff = true;            // main loop exits cleanly
        }
        break;
    }
    }
}

void Compositor::DrawDesktop() {
    if (os.activeApp && (!os.activeApp->isOpen || os.activeApp->isMinimized)) os.activeApp = nullptr;

    DrawDesktopLayer();
    DrawDesktopIcons();
    for (auto& app : os.apps) app->Draw(os);
    DrawTaskbar(os);
    DrawPowerDialog();
    DrawToasts();

    // Fade in from the boot screen.
    const float k = 1.0f - (float)(os.now - os.desktopStart) / 0.7f;
    if (k > 0.0f)
        ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(0, 0), ImGui::GetIO().DisplaySize, IM_COL32(0, 0, 0, (int)(255 * k)));
}

// Layer 0: drawn on the background draw list, so it is always beneath every window.
void Compositor::DrawDesktopLayer() {
    ImDrawList*  dl = ImGui::GetBackgroundDrawList();
    const ImVec2 d  = ImGui::GetIO().DisplaySize;
    DrawWallpaper(dl, ImVec2(0, 0), d, os.settings.wallpaper, os.settings.animateWallpaper ? (float)os.now : 0.0f);

    // Real-time clock, top-right corner, refreshed every frame.
    const std::string clock = FormatLocalTime("%A, %b %d, %Y") + "  |  " + ClockString(os.settings);
    ImFont*      f  = os.fonts.mono;
    const ImVec2 ts = f->CalcTextSizeA(f->FontSize, FLT_MAX, 0.0f, clock.c_str());
    const ImVec2 p(d.x - ts.x - 26.0f, 12.0f);
    dl->AddRectFilled(ImVec2(p.x - 10, p.y - 6), ImVec2(p.x + ts.x + 10, p.y + ts.y + 6), IM_COL32(0, 0, 0, 170), 6.0f);
    dl->AddText(f, f->FontSize, p, IM_COL32(240, 240, 240, 255), clock.c_str());

    const char* status = "CSOPESY OS v1.0 | System Online";
    dl->AddText(f, f->FontSize, ImVec2(17, d.y - os.taskbarHeight - 29), IM_COL32(0, 0, 0, 160), status);
    dl->AddText(f, f->FontSize, ImVec2(16, d.y - os.taskbarHeight - 30), IM_COL32(90, 240, 120, 255), status);
}

// Layer 1: desktop shortcuts. Double-click opens the app, right-click opens a menu.
void Compositor::DrawDesktopIcons() {
    const ImVec2 work = os.WorkArea();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(work);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14, 14));
    ImGui::Begin("##desktop", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoNav |
                     ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar();
    ImGui::BringWindowToDisplayBack(ImGui::GetCurrentWindow());
    if (ImGui::IsWindowFocused()) os.activeApp = nullptr;

    ImDrawList*  dl = ImGui::GetWindowDrawList();
    const ImVec2 cell(88, 92);
    for (int i = 0; i < (int)os.apps.size(); ++i) {
        App& app = *os.apps[i];
        ImGui::PushID(i);
        const ImVec2 p = ImGui::GetCursorScreenPos();
        if (ImGui::InvisibleButton("##icon", cell)) selectedIcon = i;
        const bool hovered = ImGui::IsItemHovered();
        if (hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) app.Open(os);
        if (selectedIcon == i) {
            dl->AddRectFilled(p, ImVec2(p.x + cell.x, p.y + cell.y), os.Accent(0.35f), 6.0f);
            dl->AddRect(p, ImVec2(p.x + cell.x, p.y + cell.y), os.Accent(0.8f), 6.0f);
        } else if (hovered) {
            dl->AddRectFilled(p, ImVec2(p.x + cell.x, p.y + cell.y), IM_COL32(255, 255, 255, 36), 6.0f);
        }
        DrawIcon(dl, app.icon, ImVec2(p.x + cell.x * 0.5f, p.y + 32), 48.0f, os.Accent());

        const float  wrap = cell.x - 6.0f;
        const ImVec2 ts   = ImGui::CalcTextSize(app.title.c_str(), nullptr, false, wrap);
        const ImVec2 tp(p.x + (cell.x - ts.x) * 0.5f, p.y + 62);
        dl->AddText(nullptr, 0.0f, ImVec2(tp.x + 1, tp.y + 1), IM_COL32(0, 0, 0, 200), app.title.c_str(), nullptr, wrap);
        dl->AddText(nullptr, 0.0f, tp, IM_COL32_WHITE, app.title.c_str(), nullptr, wrap);
        if (hovered) ImGui::SetTooltip("Double-click to open %s", app.title.c_str());
        ImGui::PopID();
        ImGui::Dummy(ImVec2(0, 4));
    }
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered()) selectedIcon = -1;

    if (ImGui::BeginPopupContextWindow("##deskmenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
        if (ImGui::BeginMenu("Wallpaper")) {
            for (int i = 0; i < kWallpaperCount; ++i)
                if (ImGui::MenuItem(kWallpaperNames[i], nullptr, os.settings.wallpaper == i)) os.settings.wallpaper = i;
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Personalize...")) if (App* a = os.FindApp("settings.exe")) a->Open(os);
        if (ImGui::MenuItem("Task Manager"))   if (App* a = os.FindApp("taskmgr.exe")) a->Open(os);
        ImGui::Separator();
        if (ImGui::MenuItem("Power options...")) os.requestPowerMenu = true;
        ImGui::EndPopup();
    }
    ImGui::End();
}

// The only way to turn the OS off: Shut down -> fade out -> main loop exits.
void Compositor::DrawPowerDialog() {
    if (os.requestPowerMenu) {
        ImGui::OpenPopup("Power Options");
        os.requestPowerMenu = false;
    }
    const ImVec2 d = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(d.x * 0.5f, d.y * 0.45f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (!ImGui::BeginPopupModal("Power Options", nullptr,
                                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
        return;

    const ImVec2 p = ImGui::GetCursorScreenPos();
    DrawIcon(ImGui::GetWindowDrawList(), IconType::Power, ImVec2(p.x + 24, p.y + 24), 44.0f, os.Accent());
    ImGui::SetCursorScreenPos(ImVec2(p.x + 60, p.y + 2));
    ImGui::BeginGroup();
    ImGui::PushFont(os.fonts.uiLarge);
    ImGui::TextUnformatted("Turn off CSOPESY OS?");
    ImGui::PopFont();
    int running = 0;
    for (auto& a : os.apps) running += a->isOpen;
    if (running > 0) ImGui::TextDisabled("%d running application%s will be closed.", running, running == 1 ? "" : "s");
    else ImGui::TextDisabled("No applications are running.");
    ImGui::EndGroup();
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(190, 50, 50, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(225, 70, 70, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(160, 40, 40, 255));
    if (ImGui::Button("Shut down", ImVec2(120, 34))) {
        os.requestShutdown = true;
        ImGui::CloseCurrentPopup();
    }
    ImGui::PopStyleColor(3);
    ImGui::SameLine();
    if (ImGui::Button("Restart", ImVec2(120, 34))) {
        os.requestRestart = true;
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 34)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
}

// Notification toasts in the bottom-right corner, above the taskbar.
void Compositor::DrawToasts() {
    const double kLife = 4.5;
    for (size_t i = 0; i < os.toasts.size();) {
        if (os.now - os.toasts[i].start > kLife) os.toasts.erase(os.toasts.begin() + (long)i);
        else ++i;
    }
    // Toasts sit on the foreground layer; hide them while a menu/popup is open so they never cover it.
    if (ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel)) return;
    ImDrawList*  dl = ImGui::GetForegroundDrawList();
    const ImVec2 d  = ImGui::GetIO().DisplaySize;
    const float  w = 330.0f, h = 62.0f;
    float y = d.y - os.taskbarHeight - 12.0f;
    for (int i = (int)os.toasts.size() - 1; i >= 0; --i) {
        const Toast& t   = os.toasts[i];
        const float  age = (float)(os.now - t.start);
        const float  in  = ImClamp(age / 0.25f, 0.0f, 1.0f);
        const float  out = ImClamp(((float)kLife - age) / 0.5f, 0.0f, 1.0f);
        const float  x   = d.x - 12.0f - w + (1.0f - in) * (1.0f - in) * (w + 20.0f);   // slide in from the right
        const int    a   = (int)(255 * out);
        y -= h;
        dl->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), IM_COL32(30, 34, 46, (int)(240 * out)), 8.0f);
        dl->AddRectFilled(ImVec2(x, y), ImVec2(x + 4, y + h), os.Accent(out), 8.0f, ImDrawFlags_RoundCornersLeft);
        dl->AddText(ImVec2(x + 16, y + 10), IM_COL32(240, 242, 248, a), t.title.c_str());
        dl->PushClipRect(ImVec2(x, y), ImVec2(x + w - 8, y + h), true);
        dl->AddText(ImVec2(x + 16, y + 33), IM_COL32(170, 178, 196, a), t.text.c_str());
        dl->PopClipRect();
        y -= 8.0f;
    }
}
