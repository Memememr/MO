// CSOPESY Desktop OS Emulator - Group 5
// Settings.cpp - personalisation, date & time, system info.
// Every change applies immediately, no restart or recompile needed.
#include "Apps.h"
#include "../Icons.h"
#include "../Wallpaper.h"

#include <cstdio>

namespace {

class SettingsApp : public App {
public:
    SettingsApp() : App("Settings", "settings.exe", IconType::Settings, ImVec2(640, 500), 0.3f, 18.2f) {
        minSize = ImVec2(520, 380);
    }

protected:
    void DrawContent(OSState& os) override {
        if (!ImGui::BeginTabBar("##settings_tabs")) return;
        if (ImGui::BeginTabItem("Personalization")) { Personalization(os); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Date & Time"))     { DateTime(os);        ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("System"))          { System(os);          ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("About"))           { About(os);           ImGui::EndTabItem(); }
        ImGui::EndTabBar();
    }

private:
    static void Heading(OSState& os, const char* text) {
        ImGui::Spacing();
        ImGui::PushFont(os.fonts.uiLarge);
        ImGui::TextUnformatted(text);
        ImGui::PopFont();
    }

    void Personalization(OSState& os) {
        Settings& s = os.settings;
        Heading(os, "Background");
        ImGui::TextDisabled("Wallpapers are drawn every frame with ImGui draw commands.");

        const ImVec2 thumb(132, 76);
        ImDrawList*  dl = ImGui::GetWindowDrawList();
        for (int i = 0; i < kWallpaperCount; ++i) {
            if (i > 0) ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::PushID(i);
            const ImVec2 p = ImGui::GetCursorScreenPos();
            if (ImGui::InvisibleButton("##wp", thumb)) s.wallpaper = i;
            DrawWallpaper(dl, p, thumb, i, s.animateWallpaper ? (float)os.now : 0.0f);
            const bool sel = s.wallpaper == i;
            dl->AddRect(ImVec2(p.x - 2, p.y - 2), ImVec2(p.x + thumb.x + 2, p.y + thumb.y + 2),
                        sel ? os.Accent() : (ImGui::IsItemHovered() ? IM_COL32(255, 255, 255, 120) : IM_COL32(255, 255, 255, 30)),
                        4.0f, 0, sel ? 3.0f : 1.0f);
            ImGui::TextUnformatted(kWallpaperNames[i]);
            ImGui::PopID();
            ImGui::EndGroup();
        }
        ImGui::Checkbox("Animate wallpaper", &s.animateWallpaper);

        Heading(os, "Accent color");
        static const ImVec4 kPresets[] = {
            {0.26f, 0.59f, 0.98f, 1}, {0.00f, 0.75f, 0.65f, 1}, {0.46f, 0.80f, 0.30f, 1}, {0.98f, 0.70f, 0.20f, 1},
            {0.95f, 0.35f, 0.35f, 1}, {0.90f, 0.35f, 0.75f, 1}, {0.58f, 0.42f, 0.95f, 1},
        };
        for (int i = 0; i < IM_ARRAYSIZE(kPresets); ++i) {
            ImGui::PushID(i);
            if (i > 0) ImGui::SameLine();
            if (ImGui::ColorButton("##preset", kPresets[i], ImGuiColorEditFlags_NoTooltip, ImVec2(30, 30))) {
                s.accent = kPresets[i];
                ApplyTheme(s.accent);
            }
            ImGui::PopID();
        }
        ImGui::SameLine();
        if (ImGui::ColorEdit3("Custom", &s.accent.x, ImGuiColorEditFlags_NoInputs)) ApplyTheme(s.accent);
    }

    void DateTime(OSState& os) {
        Settings& s = os.settings;
        Heading(os, "Current time");
        ImGui::PushFont(os.fonts.uiHuge);
        ImGui::TextUnformatted(ClockString(s).c_str());
        ImGui::PopFont();
        ImGui::TextDisabled("%s", FormatLocalTime("%A, %B %d, %Y").c_str());
        ImGui::TextDisabled("Time zone: system local time");

        Heading(os, "Format");
        ImGui::Checkbox("Use 24-hour clock", &s.clock24h);
        ImGui::Checkbox("Show seconds", &s.showSeconds);
    }

    void System(OSState& os) {
        Settings& s = os.settings;
        Heading(os, "Display");
        const ImVec2 d = ImGui::GetIO().DisplaySize;
        ImGui::Text("Resolution: %.0f x %.0f", d.x, d.y);
        ImGui::Text("Frame rate: %.0f FPS (%.2f ms/frame)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
        if (ImGui::Button(os.isFullscreen ? "Exit full screen (F11)" : "Enter full screen (F11)")) os.toggleFullscreen = true;
        ImGui::Checkbox("Show FPS counter on the taskbar", &s.showFps);

        Heading(os, "Sound");
        ImGui::SetNextItemWidth(240);
        float pct = s.volume * 100.0f;
        if (ImGui::SliderFloat("Volume", &pct, 0.0f, 100.0f, "%.0f%%")) s.volume = pct / 100.0f;
        ImGui::Checkbox("Mute", &s.muted);

        Heading(os, "Power");
        ImGui::TextDisabled("CSOPESY OS can only be turned off from here or the PWR button.");
        if (ImGui::Button("Power options...")) os.requestPowerMenu = true;
    }

    void About(OSState& os) {
        ImGui::Spacing();
        const ImVec2 p = ImGui::GetCursorScreenPos();
        DrawIcon(ImGui::GetWindowDrawList(), IconType::Computer, ImVec2(p.x + 36, p.y + 34), 64.0f, os.Accent());
        ImGui::SetCursorScreenPos(ImVec2(p.x + 84, p.y + 4));
        ImGui::BeginGroup();
        ImGui::PushFont(os.fonts.uiLarge);
        ImGui::TextUnformatted("CSOPESY OS");
        ImGui::PopFont();
        ImGui::TextDisabled("Version 1.0  |  Semi-Major Output 2: Desktop-Style OS Mock-up");
        ImGui::EndGroup();
        ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + 78));
        ImGui::Separator();

        if (ImGui::BeginTable("##about", 2, ImGuiTableFlags_SizingStretchProp)) {
            auto row = [](const char* k, const char* v) {
                ImGui::TableNextColumn(); ImGui::TextDisabled("%s", k);
                ImGui::TableNextColumn(); ImGui::TextWrapped("%s", v);
            };
            row("Developed by", "Group 5: Alvarez, Sanidad, Viray, Quijano");
            row("Course", "CSOPESY - Introduction to Operating Systems");
            row("Version date", "2026-10-09");
            row("UI stack", "GLFW 3.4 + OpenGL 3 + Dear ImGui " IMGUI_VERSION);
            row("Renderer", os.glRenderer.c_str());
            row("OpenGL", os.glVersion.c_str());
            row("Up time", [&] {
                static char buf[32];
                const int t = (int)(os.now - os.desktopStart);
                std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", t / 3600, (t / 60) % 60, t % 60);
                return buf;
            }());
            ImGui::EndTable();
        }
    }
};

}  // namespace

std::unique_ptr<App> MakeSettings() { return std::make_unique<SettingsApp>(); }
