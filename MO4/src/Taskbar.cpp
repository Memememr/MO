// CSOPESY Desktop OS Emulator - Group 5
// Taskbar.cpp - start menu, app buttons with running indicators, system tray.
#include "Taskbar.h"

#include "App.h"
#include "Icons.h"
#include "imgui_internal.h"   // BringWindowToDisplayFront

#include <cstdio>
#include <ctime>

namespace {

ImVec2 gPopupAnchor;   // bottom-right corner the current tray popup hangs from

void OpenAnchoredPopup(const char* id, bool alignRight) {
    gPopupAnchor = alignRight ? ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y - 8.0f)
                              : ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y - 8.0f);
    ImGui::OpenPopup(id);
}

bool BeginAnchoredPopup(const char* id, bool alignRight) {
    ImGui::SetNextWindowPos(gPopupAnchor, ImGuiCond_Always, ImVec2(alignRight ? 1.0f : 0.0f, 1.0f));
    return ImGui::BeginPopup(id, ImGuiWindowFlags_NoMove);
}

// Flat button used for the tray items. Text is centred; supports two lines.
bool TrayButton(const char* id, const char* line1, const char* line2, float width, ImU32 col) {
    const ImVec2 p = ImGui::GetCursorScreenPos();
    const ImVec2 size(width, ImGui::GetContentRegionAvail().y);
    const bool   pressed = ImGui::InvisibleButton(id, size);
    ImDrawList*  dl      = ImGui::GetWindowDrawList();
    if (ImGui::IsItemHovered())
        dl->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), IM_COL32(255, 255, 255, ImGui::IsItemActive() ? 40 : 22), 6.0f);

    const float lh = ImGui::GetTextLineHeight();
    float y = p.y + (size.y - (line2 ? lh * 2.0f : lh)) * 0.5f;
    for (const char* line : {line1, line2}) {
        if (!line) continue;
        const float w = ImGui::CalcTextSize(line).x;
        dl->AddText(ImVec2(p.x + (size.x - w) * 0.5f, y), col, line);
        y += lh;
        col = IM_COL32(170, 178, 196, 255);   // second line is dimmer
    }
    ImGui::SameLine();
    return pressed;
}

// One button per application. Underline = running; long accent underline = focused.
void AppButton(OSState& os, App& app, float size) {
    ImGui::PushID(app.exe.c_str());
    const ImVec2 p       = ImGui::GetCursorScreenPos();
    const bool   pressed = ImGui::InvisibleButton("##app", ImVec2(size, size));
    const bool   hovered = ImGui::IsItemHovered();
    const bool   active  = os.activeApp == &app;
    ImDrawList*  dl      = ImGui::GetWindowDrawList();

    if (hovered || active)
        dl->AddRectFilled(p, ImVec2(p.x + size, p.y + size), IM_COL32(255, 255, 255, active ? 34 : 22), 6.0f);
    const float bump = (hovered && ImGui::IsItemActive()) ? 1.0f : 0.0f;
    DrawIcon(dl, app.icon, ImVec2(p.x + size * 0.5f, p.y + size * 0.46f + bump), size * 0.60f, os.Accent());
    if (app.isOpen) {
        const float w = active ? 18.0f : 6.0f, cx = p.x + size * 0.5f;
        dl->AddRectFilled(ImVec2(cx - w * 0.5f, p.y + size - 4.0f), ImVec2(cx + w * 0.5f, p.y + size - 1.0f),
                          active ? os.Accent() : IM_COL32(170, 178, 196, 255), 2.0f);
    }
    if (hovered)
        ImGui::SetTooltip("%s%s", app.title.c_str(),
                          !app.isOpen ? "" : app.isMinimized ? "  (minimized)" : "  (running)");

    if (pressed) {
        if (!app.isOpen)          app.Open(os);
        else if (app.isMinimized) app.Restore();
        else if (active)          { app.Minimize(); os.activeApp = nullptr; }
        else                      app.Restore();   // bring to front
    }
    ImGui::PopID();
    ImGui::SameLine();
}

void StartButton(OSState& os, float size) {
    const ImVec2 p = ImGui::GetCursorScreenPos();
    if (ImGui::InvisibleButton("##start", ImVec2(size, size))) OpenAnchoredPopup("##startmenu", false);
    const bool  hovered = ImGui::IsItemHovered();
    ImDrawList* dl      = ImGui::GetWindowDrawList();
    if (hovered) dl->AddRectFilled(p, ImVec2(p.x + size, p.y + size), IM_COL32(255, 255, 255, 22), 6.0f);
    if (hovered) ImGui::SetTooltip("Start");

    // 2x2 tile logo in the accent colour.
    const float t = size * 0.18f, g = size * 0.05f, cx = p.x + size * 0.5f, cy = p.y + size * 0.5f;
    for (int i = 0; i < 4; ++i) {
        const float x = (i % 2) ? cx + g * 0.5f : cx - g * 0.5f - t;
        const float y = (i / 2) ? cy + g * 0.5f : cy - g * 0.5f - t;
        dl->AddRectFilled(ImVec2(x, y), ImVec2(x + t, y + t), os.Accent(i == 0 ? 1.0f : 0.75f), 2.0f);
    }
    ImGui::SameLine();
}

void StartMenu(OSState& os) {
    ImGui::SetNextWindowSize(ImVec2(300, 0));
    if (!BeginAnchoredPopup("##startmenu", false)) return;

    ImDrawList*  dl = ImGui::GetWindowDrawList();
    const ImVec2 p  = ImGui::GetCursorScreenPos();
    dl->AddCircleFilled(ImVec2(p.x + 22, p.y + 22), 22, os.Accent());
    const ImVec2 ts = ImGui::CalcTextSize("G5");
    dl->AddText(ImVec2(p.x + 22 - ts.x * 0.5f, p.y + 22 - ts.y * 0.5f), IM_COL32_WHITE, "G5");
    ImGui::SetCursorScreenPos(ImVec2(p.x + 54, p.y + 3));
    ImGui::TextUnformatted("Group 5");
    ImGui::SetCursorScreenPos(ImVec2(p.x + 54, p.y + 23));
    ImGui::TextDisabled("CSOPESY OS v1.0");
    ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + 50));
    ImGui::Separator();
    ImGui::TextDisabled("Applications");

    for (auto& app : os.apps) {
        ImGui::PushID(app->exe.c_str());
        InlineIcon(app->icon, 20.0f, os.Accent());
        if (ImGui::Selectable(app->title.c_str())) {
            app->Open(os);
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopID();
    }
    ImGui::Separator();
    InlineIcon(IconType::Power, 20.0f, os.Accent());
    if (ImGui::Selectable("Power options...")) {
        os.requestPowerMenu = true;
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void Calendar(OSState& os) {
    ImGui::SetNextWindowSize(ImVec2(290, 0));
    if (!BeginAnchoredPopup("##calendar", true)) return;
    ImGui::PushFont(os.fonts.uiLarge);
    ImGui::TextUnformatted(ClockString(os.settings).c_str());
    ImGui::PopFont();
    ImGui::TextDisabled("%s", FormatLocalTime("%A, %B %d, %Y").c_str());
    ImGui::Separator();

    std::time_t now = std::time(nullptr);
    std::tm     tm{};
#ifdef _WIN32
    localtime_s(&tm, &now);
#else
    localtime_r(&now, &tm);
#endif
    static const int kDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    const int  year   = tm.tm_year + 1900;
    const bool leap   = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
    const int  days   = kDays[tm.tm_mon] + (tm.tm_mon == 1 && leap ? 1 : 0);
    const int  first  = ((tm.tm_wday - (tm.tm_mday - 1) % 7) + 7) % 7;   // weekday of the 1st

    ImGui::TextUnformatted(FormatLocalTime("%B %Y").c_str());
    if (ImGui::BeginTable("##cal", 7, ImGuiTableFlags_SizingStretchSame)) {
        static const char* const kHead[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
        for (const char* h : kHead) {
            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
            TextRightAligned(h);
            ImGui::PopStyleColor();
        }
        for (int cell = 0; cell < first + days; ++cell) {
            ImGui::TableNextColumn();
            if (cell < first) continue;
            const int day = cell - first + 1;
            if (day == tm.tm_mday) ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, os.Accent(0.85f));
            char buf[16];
            std::snprintf(buf, sizeof(buf), "%d", day);
            TextRightAligned(buf);
        }
        ImGui::EndTable();
    }
    ImGui::EndPopup();
}

void VolumePopup(OSState& os) {
    if (!BeginAnchoredPopup("##volume", true)) return;
    ImGui::TextUnformatted("Volume");
    ImGui::SetNextItemWidth(200);
    float pct = os.settings.volume * 100.0f;
    if (ImGui::SliderFloat("##vol", &pct, 0.0f, 100.0f, "%.0f%%")) os.settings.volume = pct / 100.0f;
    ImGui::Checkbox("Mute", &os.settings.muted);
    ImGui::TextDisabled("Output: CSOPESY Virtual Speakers");
    ImGui::EndPopup();
}

void NetworkPopup(OSState& os) {
    if (!BeginAnchoredPopup("##network", true)) return;
    ImDrawList*  dl = ImGui::GetWindowDrawList();
    const ImVec2 p  = ImGui::GetCursorScreenPos();
    for (int i = 0; i < 4; ++i)   // signal bars
        dl->AddRectFilled(ImVec2(p.x + i * 7.0f, p.y + 16.0f - i * 4.0f), ImVec2(p.x + i * 7.0f + 5.0f, p.y + 18.0f), os.Accent());
    ImGui::Dummy(ImVec2(30, 18));
    ImGui::SameLine();
    ImGui::TextUnformatted("CSOPESY-LAN");
    ImGui::TextColored(ImVec4(0.45f, 0.9f, 0.5f, 1.0f), "Connected, secured");
    ImGui::Separator();
    ImGui::TextDisabled("Adapter:  CSOPESY Virtual Ethernet");
    ImGui::TextDisabled("IPv4:     192.168.5.10");
    ImGui::TextDisabled("Speed:    1.0 Gbps");
    ImGui::EndPopup();
}

}  // namespace

void DrawTaskbar(OSState& os) {
    const ImVec2 d  = ImGui::GetIO().DisplaySize;
    const float  th = os.taskbarHeight;

    ImGui::SetNextWindowPos(ImVec2(0, d.y - th));
    ImGui::SetNextWindowSize(ImVec2(d.x, th));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 6));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));   // theme min height would push it off-screen
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(16, 18, 26, 238));
    ImGui::Begin("##taskbar", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
                     ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollWithMouse);
    if (!ImGui::GetTopMostPopupModal())   // always above app windows (but under modal dialogs)
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
    ImGui::PopStyleVar(5);
    ImGui::PopStyleColor();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddLine(ImVec2(0, d.y - th), ImVec2(d.x, d.y - th), IM_COL32(255, 255, 255, 30));

    const float btn = th - 12.0f;
    StartButton(os, btn);
    const ImVec2 sep = ImGui::GetCursorScreenPos();
    dl->AddLine(ImVec2(sep.x + 1, sep.y + 8), ImVec2(sep.x + 1, sep.y + btn - 8), IM_COL32(255, 255, 255, 40));
    ImGui::Dummy(ImVec2(3, btn));
    ImGui::SameLine();
    for (auto& app : os.apps) AppButton(os, *app, btn);

    // ---- System tray (right-aligned) ----
    const std::string time = ClockString(os.settings);
    const std::string date = FormatLocalTime("%m/%d/%Y");
    const float clockW = ImMax(ImGui::CalcTextSize(time.c_str()).x, ImGui::CalcTextSize(date.c_str()).x) + 20.0f;
    const float fpsW   = os.settings.showFps ? 74.0f : 0.0f;
    const float trayW  = fpsW + 52.0f + 52.0f + clockW + 56.0f + 4 * 6.0f;
    ImGui::SetCursorScreenPos(ImVec2(d.x - 10.0f - trayW, d.y - th + 6.0f));

    if (os.settings.showFps) {
        char fps[32];
        std::snprintf(fps, sizeof(fps), "%.0f FPS", ImGui::GetIO().Framerate);
        TrayButton("##fps", fps, nullptr, fpsW, IM_COL32(120, 230, 140, 255));
    }
    const bool muted = os.settings.muted || os.settings.volume <= 0.0f;
    if (TrayButton("##vol", muted ? "MUTE" : "VOL", nullptr, 52.0f, muted ? IM_COL32(150, 150, 150, 255) : IM_COL32(225, 228, 236, 255)))
        OpenAnchoredPopup("##volume", true);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Volume: %.0f%%%s", os.settings.volume * 100.0f, os.settings.muted ? " (muted)" : "");
    if (TrayButton("##net", "NET", nullptr, 52.0f, IM_COL32(225, 228, 236, 255))) OpenAnchoredPopup("##network", true);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("CSOPESY-LAN: Connected");
    if (TrayButton("##clock", time.c_str(), date.c_str(), clockW, IM_COL32(235, 238, 245, 255)))
        OpenAnchoredPopup("##calendar", true);
    if (TrayButton("##pwr", "PWR", nullptr, 56.0f, IM_COL32(255, 92, 92, 255))) os.requestPowerMenu = true;
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Shut down or restart");

    StartMenu(os);
    VolumePopup(os);
    NetworkPopup(os);
    Calendar(os);
    ImGui::End();
}
