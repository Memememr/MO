// CSOPESY Desktop OS Emulator - Group 5
// Boot.cpp - BIOS POST screen, splash/loading screen and power-off screen.
#include "Boot.h"

#include <cmath>
#include <cstdio>
#include <string>

namespace {

const char* const kLogo[] = {
    R"(  _____  _____  ____  _____  ______  _______     __)",
    R"( / ____|/ ____|/ __ \|  __ \|  ____|/ ____\ \   / /)",
    R"(| |    | (___ | |  | | |__) | |__  | (___  \ \_/ / )",
    R"(| |     \___ \| |  | |  ___/|  __|  \___ \  \   /  )",
    R"(| |____ ____) | |__| | |    | |____ ____) |  | |   )",
    R"( \_____|_____/ \____/|_|    |______|_____/   |_|   )",
};

void CenteredText(ImDrawList* dl, ImFont* font, float y, ImU32 col, const char* text) {
    const float  w = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, text).x;
    const ImVec2 d = ImGui::GetIO().DisplaySize;
    dl->AddText(font, font->FontSize, ImVec2((d.x - w) * 0.5f, y), col, text);
}

}  // namespace

void DrawBios(OSState& os, float t) {
    ImDrawList*  dl = ImGui::GetForegroundDrawList();
    const ImVec2 d  = ImGui::GetIO().DisplaySize;
    ImFont*      f  = os.fonts.mono;
    const float  lh = f->FontSize + 4.0f;
    dl->AddRectFilled(ImVec2(0, 0), d, IM_COL32(0, 0, 0, 255));

    char ram[64];
    const int kb = (int)ImClamp((t - 0.6f) / 1.4f * 65536.0f, 0.0f, 65536.0f) / 64 * 64;
    std::snprintf(ram, sizeof(ram), "Checking RAM : %dK%s", kb, t > 2.0f ? " OK" : "");
    const std::string gpu = "Display Adapter: " + os.glRenderer;

    struct Line { float at; const char* text; ImU32 col; };
    const ImU32 W = IM_COL32(200, 200, 200, 255), Y = IM_COL32(240, 220, 90, 255), G = IM_COL32(110, 230, 120, 255);
    const Line lines[] = {
        {0.0f, "CSOPESY Megatrends BIOS v5.0", W},
        {0.0f, "Released: 09/18/26", W},
        {0.0f, "DLSU CSOPESY Lab (C)2026 Group 5 Systems Inc.", W},
        {0.6f, "", W},
        {0.6f, "Memory Test:", W},
        {0.6f, ram, t > 2.0f ? G : W},
        {2.2f, "", W},
        {2.2f, "CPU Type: CSOPESY Virtual CPU @ 3.20GHz", W},
        {2.35f, "BIOS Version: G5 SIT 2026 Special CSOPESY", W},
        {2.5f, "Main Processor: x86-64 (emulated)", W},
        {2.6f, "Numeric Processor: Built-in", W},
        {2.9f, "", W},
        {2.9f, "Primary Master:   512 GB CSOPESY-NVMe", W},
        {3.0f, "Primary Slave:    None", W},
        {3.1f, "Secondary Master: CD-ROM CSO-52X", W},
        {3.2f, "Secondary Slave:  None", W},
        {3.4f, gpu.c_str(), W},
        {3.8f, "", W},
        {3.8f, "Booting CSOPESY OS from Primary Master...", Y},
    };

    float y = 24.0f;
    for (const Line& l : lines) {
        if (t < l.at) break;
        dl->AddText(f, f->FontSize, ImVec2(28.0f, y), l.col, l.text);
        y += lh;
    }
    if (std::fmod(t, 0.8f) < 0.4f)   // blinking block cursor
        dl->AddRectFilled(ImVec2(28.0f, y + 2), ImVec2(38.0f, y + lh - 4), W);

    dl->AddText(f, f->FontSize, ImVec2(28.0f, d.y - 36.0f), IM_COL32(130, 130, 130, 255),
                "Press any key or click to skip");
}

void DrawSplash(OSState& os, float t) {
    ImDrawList*  dl = ImGui::GetForegroundDrawList();
    const ImVec2 d  = ImGui::GetIO().DisplaySize;
    dl->AddRectFilled(ImVec2(0, 0), d, IM_COL32(0, 0, 0, 255));

    const int   alpha = (int)(255 * ImClamp(t / 0.6f, 0.0f, 1.0f));
    ImFont*     mono  = os.fonts.mono;
    const int   n     = IM_ARRAYSIZE(kLogo);
    float       y     = d.y * 0.5f - 170.0f;
    for (int i = 0; i < n; ++i) {
        const float k = (float)i / (n - 1);   // cyan at the top, blue at the bottom
        CenteredText(dl, os.fonts.monoLarge, y, IM_COL32((int)(40 + 30 * k), (int)(210 - 90 * k), 255, alpha), kLogo[i]);
        y += os.fonts.monoLarge->FontSize;
    }

    y += 22.0f;
    const ImU32 white = IM_COL32(225, 225, 225, alpha), grey = IM_COL32(150, 150, 150, alpha);
    CenteredText(dl, mono, y, white, "CSOPESY Operating System Emulator v1.0");                   y += 22.0f;
    CenteredText(dl, mono, y, grey, "Developed by Group 5: Alvarez | Sanidad | Viray | Quijano"); y += 36.0f;
    CenteredText(dl, mono, y, white, "Semi-Major Output 2: Desktop-Style OS Mock-up");            y += 22.0f;
    CenteredText(dl, mono, y, grey, "Built on GLFW + OpenGL + Dear ImGui");                      y += 44.0f;

    static const char* const kDots[] = {"Loading   ", "Loading.  ", "Loading.. ", "Loading..."};
    CenteredText(dl, mono, y, IM_COL32(90, 230, 110, alpha), kDots[(int)(t * 3.0f) % 4]);
    y += 30.0f;

    const float w = 320.0f, x0 = (d.x - w) * 0.5f;
    const float p = 1.0f - std::pow(1.0f - ImClamp(t / (kSplashDuration - 0.3f), 0.0f, 1.0f), 2.0f);
    dl->AddRect(ImVec2(x0, y), ImVec2(x0 + w, y + 12), IM_COL32(90, 230, 110, alpha), 3.0f);
    dl->AddRectFilled(ImVec2(x0 + 2, y + 2), ImVec2(x0 + 2 + (w - 4) * p, y + 10), IM_COL32(90, 230, 110, alpha), 2.0f);
}

void DrawPowerOff(OSState& os, float t, bool restart) {
    ImDrawList*  dl = ImGui::GetForegroundDrawList();
    const ImVec2 d  = ImGui::GetIO().DisplaySize;
    const float  k  = ImClamp(t / 0.6f, 0.0f, 1.0f);
    dl->AddRectFilled(ImVec2(0, 0), d, IM_COL32(6, 10, 22, (int)(255 * k)));
    if (k < 1.0f) return;

    // Spinner: ring of dots, the brightest one rotating.
    const ImVec2 c(d.x * 0.5f, d.y * 0.5f - 30.0f);
    for (int i = 0; i < 10; ++i) {
        const float a     = i / 10.0f * 6.2831853f;
        const float head  = std::fmod(t * 8.0f, 10.0f);
        const float dist  = std::fmod(head - i + 10.0f, 10.0f);
        const int   alpha = (int)(255 * ImMax(0.15f, 1.0f - dist / 6.0f));
        dl->AddCircleFilled(ImVec2(c.x + std::cos(a) * 20, c.y + std::sin(a) * 20), 3.5f, os.Accent(alpha / 255.0f));
    }
    const char* title = restart ? "Restarting..." : "Shutting down...";
    CenteredText(dl, os.fonts.uiLarge, c.y + 44.0f, IM_COL32(235, 235, 235, 255), title);
    CenteredText(dl, os.fonts.ui, c.y + 80.0f, IM_COL32(150, 160, 180, 255),
                 t < 1.6f ? "Closing applications..." : "Goodbye from Group 5!");
}
