// CSOPESY Desktop OS Emulator - Group 5
// Wallpaper.cpp - every wallpaper is generated each frame from draw commands,
// so it scales to any window size and can be animated.
#include "Wallpaper.h"

#include <cmath>

const char* const kWallpaperNames[kWallpaperCount] = {"Rolling Hills", "Sunset", "Retro Grid", "Midnight"};

namespace {

ImU32 Lerp(ImU32 a, ImU32 b, float t) {
    const ImVec4 A = ImGui::ColorConvertU32ToFloat4(a), B = ImGui::ColorConvertU32ToFloat4(b);
    return ImGui::ColorConvertFloat4ToU32(ImVec4(A.x + (B.x - A.x) * t, A.y + (B.y - A.y) * t,
                                                 A.z + (B.z - A.z) * t, A.w + (B.w - A.w) * t));
}

// Fills the area below the curve y = f(x) (both normalised 0..1) with
// thin vertical gradient strips. Handles non-convex shapes such as hills.
template <typename F>
void FillUnderCurve(ImDrawList* dl, ImVec2 o, ImVec2 s, F f, ImU32 top, ImU32 bottom) {
    const float step = ImMax(2.0f, s.x / 360.0f);
    for (float x = 0.0f; x < s.x; x += step) {
        const float y = o.y + s.y * f((x + step * 0.5f) / s.x);
        dl->AddRectFilledMultiColor(ImVec2(o.x + x, y), ImVec2(o.x + x + step + 0.5f, o.y + s.y),
                                    top, top, bottom, bottom);
    }
}

void Cloud(ImDrawList* dl, ImVec2 c, float r) {
    const ImU32 shade = IM_COL32(214, 226, 242, 235), white = IM_COL32(255, 255, 255, 240);
    dl->AddCircleFilled(ImVec2(c.x - r * 1.1f, c.y + r * 0.25f), r * 0.75f, shade);
    dl->AddCircleFilled(ImVec2(c.x + r * 1.2f, c.y + r * 0.25f), r * 0.80f, shade);
    dl->AddCircleFilled(ImVec2(c.x - r * 1.1f, c.y + r * 0.10f), r * 0.75f, white);
    dl->AddCircleFilled(ImVec2(c.x + r * 1.2f, c.y + r * 0.10f), r * 0.80f, white);
    dl->AddCircleFilled(ImVec2(c.x, c.y - r * 0.15f), r, white);
    dl->AddCircleFilled(ImVec2(c.x + r * 0.55f, c.y - r * 0.45f), r * 0.75f, white);
    dl->AddRectFilled(ImVec2(c.x - r * 1.1f, c.y + r * 0.1f), ImVec2(c.x + r * 1.2f, c.y + r * 0.85f), white);
}

void RollingHills(ImDrawList* dl, ImVec2 o, ImVec2 s, float t) {
    dl->AddRectFilledMultiColor(o, ImVec2(o.x + s.x, o.y + s.y * 0.8f), IM_COL32(24, 86, 196, 255),
                                IM_COL32(24, 86, 196, 255), IM_COL32(156, 204, 246, 255), IM_COL32(156, 204, 246, 255));

    const float k = s.y / 720.0f;   // scale clouds with the drawing size
    for (int i = 0; i < 9; ++i) {
        const float speed = 6.0f + 4.0f * (i % 3);
        const float span  = s.x + 400.0f * k;
        const float x     = std::fmod(i * 0.37f * span + t * speed * k, span) - 200.0f * k;
        const float y     = s.y * (0.07f + 0.05f * ((i * 7) % 6));
        Cloud(dl, ImVec2(o.x + x, o.y + y), (18.0f + 7.0f * ((i * 5) % 4)) * k);
    }

    FillUnderCurve(dl, o, s, [](float x) { return 0.60f + 0.035f * std::sin(x * 6.0f + 0.8f) + 0.02f * std::sin(x * 13.0f); },
                   IM_COL32(86, 150, 66, 255), IM_COL32(40, 96, 40, 255));
    FillUnderCurve(dl, o, s, [](float x) {
        const float d = x - 0.42f;
        return 0.80f - 0.21f * std::exp(-(d * d) / 0.07f) - 0.025f * std::sin(x * 4.0f);
    }, IM_COL32(126, 204, 70, 255), IM_COL32(38, 118, 30, 255));
}

void Sunset(ImDrawList* dl, ImVec2 o, ImVec2 s, float t) {
    const ImU32 c0 = IM_COL32(32, 18, 74, 255), c1 = IM_COL32(196, 66, 112, 255), c2 = IM_COL32(255, 170, 92, 255);
    dl->AddRectFilledMultiColor(o, ImVec2(o.x + s.x, o.y + s.y * 0.45f), c0, c0, c1, c1);
    dl->AddRectFilledMultiColor(ImVec2(o.x, o.y + s.y * 0.45f), ImVec2(o.x + s.x, o.y + s.y * 0.72f), c1, c1, c2, c2);

    for (int i = 0; i < 60; ++i) {   // twinkling stars
        const float x = std::fmod(i * 97.13f, 1.0f) * s.x, y = std::fmod(i * 0.6180339f, 1.0f) * s.y * 0.35f;
        const int   a = (int)(120 + 100 * std::sin(t * 1.5f + i));
        dl->AddCircleFilled(ImVec2(o.x + x, o.y + y), ImMax(0.8f, s.y / 600.0f), IM_COL32(255, 255, 255, a));
    }

    const ImVec2 sun(o.x + s.x * 0.5f, o.y + s.y * 0.60f);
    const float  r = s.y * 0.11f;
    for (int i = 6; i >= 1; --i)
        dl->AddCircleFilled(sun, r * (1.0f + i * 0.18f), IM_COL32(255, 200, 120, 14), 48);
    dl->AddCircleFilled(sun, r, IM_COL32(255, 226, 140, 255), 48);

    FillUnderCurve(dl, o, s, [](float x) { return 0.58f + 0.06f * std::sin(x * 7.0f + 1.3f) + 0.03f * std::sin(x * 17.0f); },
                   IM_COL32(110, 46, 96, 255), IM_COL32(60, 24, 70, 255));
    FillUnderCurve(dl, o, s, [](float x) { return 0.70f + 0.05f * std::sin(x * 5.0f + 3.0f) + 0.015f * std::sin(x * 31.0f); },
                   IM_COL32(52, 20, 56, 255), IM_COL32(20, 8, 26, 255));
}

void RetroGrid(ImDrawList* dl, ImVec2 o, ImVec2 s, float t) {
    const float hz = o.y + s.y * 0.58f;
    const ImU32 top = IM_COL32(10, 4, 30, 255), mid = IM_COL32(92, 22, 104, 255);
    dl->AddRectFilledMultiColor(o, ImVec2(o.x + s.x, hz), top, top, mid, mid);

    // Striped sun: horizontal chords, with gaps growing towards the bottom.
    const ImVec2 c(o.x + s.x * 0.5f, o.y + s.y * 0.42f);
    const float  r = s.y * 0.20f;
    for (float y = -r; y < ImMin(r, hz - c.y); y += 1.0f) {
        const float f = (y + r) / (2.0f * r);
        if (f > 0.55f && std::fmod(y + r, r * 0.16f) < r * 0.16f * (f - 0.45f)) continue;
        const float w = std::sqrt(r * r - y * y);
        dl->AddLine(ImVec2(c.x - w, c.y + y), ImVec2(c.x + w, c.y + y),
                    Lerp(IM_COL32(255, 222, 90, 255), IM_COL32(255, 52, 150, 255), f));
    }

    dl->AddRectFilled(ImVec2(o.x, hz), ImVec2(o.x + s.x, o.y + s.y), IM_COL32(14, 4, 34, 255));
    const ImU32 line = IM_COL32(255, 60, 200, 200);
    const float phase = std::fmod(t * 0.35f, 1.0f);
    for (int i = 0; i < 14; ++i) {
        const float f = std::pow((i + phase) / 14.0f, 2.2f);
        const float y = hz + (o.y + s.y - hz) * f;
        dl->AddLine(ImVec2(o.x, y), ImVec2(o.x + s.x, y), line, 1.0f + f * 1.5f);
    }
    for (int i = -16; i <= 16; ++i)
        dl->AddLine(ImVec2(c.x + i * s.x * 0.03f, hz), ImVec2(c.x + i * s.x * 0.16f, o.y + s.y), line, 1.2f);
    dl->AddLine(ImVec2(o.x, hz), ImVec2(o.x + s.x, hz), IM_COL32(120, 240, 255, 255), 2.0f);
}

void Midnight(ImDrawList* dl, ImVec2 o, ImVec2 s, float t) {
    dl->AddRectFilledMultiColor(o, ImVec2(o.x + s.x, o.y + s.y), IM_COL32(8, 14, 40, 255), IM_COL32(40, 16, 70, 255),
                                IM_COL32(6, 40, 60, 255), IM_COL32(4, 8, 24, 255));
    for (int i = 0; i < 5; ++i) {   // slow-moving glowing orbs
        const float x = o.x + s.x * (0.5f + 0.38f * std::sin(t * 0.07f * (i + 1) + i * 1.7f));
        const float y = o.y + s.y * (0.45f + 0.30f * std::cos(t * 0.05f * (i + 2) + i));
        const ImU32 col = (i % 2) ? IM_COL32(60, 220, 200, 6) : IM_COL32(150, 90, 255, 6);
        for (int k = 8; k >= 1; --k) dl->AddCircleFilled(ImVec2(x, y), s.y * 0.05f * k, col, 48);
    }
}

}  // namespace

void DrawWallpaper(ImDrawList* dl, ImVec2 o, ImVec2 s, int mode, float t) {
    dl->PushClipRect(o, ImVec2(o.x + s.x, o.y + s.y), true);
    switch (mode) {
    case 1:  Sunset(dl, o, s, t); break;
    case 2:  RetroGrid(dl, o, s, t); break;
    case 3:  Midnight(dl, o, s, t); break;
    default: RollingHills(dl, o, s, t); break;
    }
    dl->PopClipRect();
}
