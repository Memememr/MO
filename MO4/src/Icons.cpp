// CSOPESY Desktop OS Emulator - Group 5
// Icons.cpp - vector icons built from ImGui draw commands.
#include "Icons.h"

#include <cmath>

namespace {

const float kPi = 3.14159265f;

ImVec2 P(ImVec2 c, float s, float x, float y) { return ImVec2(c.x + s * x, c.y + s * y); }

// White document with a folded corner; returns nothing, used by the file icons.
void Page(ImDrawList* dl, ImVec2 c, float s, ImU32 fold) {
    const ImVec2 a = P(c, s, -0.32f, -0.42f), b = P(c, s, 0.32f, 0.42f);
    const float  f = s * 0.18f;
    const ImVec2 outline[] = {a, ImVec2(b.x - f, a.y), ImVec2(b.x, a.y + f), b, ImVec2(a.x, b.y)};
    dl->AddConvexPolyFilled(outline, 5, IM_COL32(236, 240, 246, 255));
    dl->AddTriangleFilled(ImVec2(b.x - f, a.y), ImVec2(b.x - f, a.y + f), ImVec2(b.x, a.y + f), fold);
    dl->AddPolyline(outline, 5, IM_COL32(120, 130, 150, 255), ImDrawFlags_Closed, 1.0f);
}

void Folder(ImDrawList* dl, ImVec2 c, float s) {
    const float r = s * 0.05f;
    dl->AddRectFilled(P(c, s, -0.45f, -0.33f), P(c, s, -0.06f, -0.15f), IM_COL32(214, 160, 40, 255), r);
    dl->AddRectFilled(P(c, s, -0.45f, -0.24f), P(c, s, 0.45f, 0.34f), IM_COL32(214, 160, 40, 255), r);
    dl->AddRectFilled(P(c, s, -0.45f, -0.12f), P(c, s, 0.45f, 0.34f), IM_COL32(255, 202, 70, 255), r);
    dl->AddLine(P(c, s, -0.40f, -0.06f), P(c, s, 0.40f, -0.06f), IM_COL32(255, 228, 140, 255), 1.0f);
}

void Terminal(ImDrawList* dl, ImVec2 c, float s) {
    const ImVec2 a = P(c, s, -0.45f, -0.36f), b = P(c, s, 0.45f, 0.36f);
    dl->AddRectFilled(a, b, IM_COL32(22, 24, 30, 255), s * 0.08f);
    dl->AddRectFilled(a, ImVec2(b.x, a.y + s * 0.14f), IM_COL32(70, 76, 92, 255), s * 0.08f, ImDrawFlags_RoundCornersTop);
    dl->AddRect(a, b, IM_COL32(110, 118, 136, 255), s * 0.08f, 0, 1.0f);
    const ImU32 g = IM_COL32(80, 230, 120, 255);
    const float t = ImMax(1.5f, s * 0.06f);
    dl->AddLine(P(c, s, -0.30f, -0.08f), P(c, s, -0.14f, 0.05f), g, t);
    dl->AddLine(P(c, s, -0.14f, 0.05f), P(c, s, -0.30f, 0.18f), g, t);
    dl->AddLine(P(c, s, -0.04f, 0.20f), P(c, s, 0.22f, 0.20f), g, t);
}

void TaskManager(ImDrawList* dl, ImVec2 c, float s, ImU32 accent) {
    const ImVec2 a = P(c, s, -0.45f, -0.36f), b = P(c, s, 0.45f, 0.36f);
    dl->AddRectFilled(a, b, IM_COL32(28, 34, 46, 255), s * 0.08f);
    dl->AddRect(a, b, IM_COL32(110, 118, 136, 255), s * 0.08f, 0, 1.0f);
    for (int i = 1; i < 3; ++i) {
        const float y = a.y + (b.y - a.y) * i / 3.0f;
        dl->AddLine(ImVec2(a.x + 2, y), ImVec2(b.x - 2, y), IM_COL32(60, 70, 90, 255));
    }
    const ImVec2 pts[] = {P(c, s, -0.38f, 0.20f), P(c, s, -0.20f, 0.05f), P(c, s, -0.06f, 0.14f),
                          P(c, s, 0.08f, -0.22f), P(c, s, 0.20f, 0.02f), P(c, s, 0.38f, -0.12f)};
    dl->AddPolyline(pts, 6, accent, 0, ImMax(1.5f, s * 0.06f));
}

void Gear(ImDrawList* dl, ImVec2 c, float s) {
    const ImU32 col = IM_COL32(170, 180, 198, 255);
    const float rOuter = s * 0.40f, rBody = s * 0.30f;
    for (int i = 0; i < 8; ++i) {
        const float ang = i * kPi / 4.0f, w = 0.24f;
        const ImVec2 p1(c.x + std::cos(ang - w) * rBody, c.y + std::sin(ang - w) * rBody);
        const ImVec2 p2(c.x + std::cos(ang - w * 0.6f) * rOuter, c.y + std::sin(ang - w * 0.6f) * rOuter);
        const ImVec2 p3(c.x + std::cos(ang + w * 0.6f) * rOuter, c.y + std::sin(ang + w * 0.6f) * rOuter);
        const ImVec2 p4(c.x + std::cos(ang + w) * rBody, c.y + std::sin(ang + w) * rBody);
        dl->AddQuadFilled(p1, p2, p3, p4, col);
    }
    dl->AddCircleFilled(c, rBody, col, 24);
    dl->AddCircleFilled(c, s * 0.13f, IM_COL32(40, 46, 60, 255), 20);
}

void Power(ImDrawList* dl, ImVec2 c, float s, ImU32 col) {
    const float t = ImMax(1.5f, s * 0.08f);
    dl->PathArcTo(c, s * 0.32f, -kPi / 2 + 0.75f, 3 * kPi / 2 - 0.75f, 24);
    dl->PathStroke(col, 0, t);
    dl->AddLine(P(c, s, 0.0f, -0.42f), P(c, s, 0.0f, -0.05f), col, t);
}

void Computer(ImDrawList* dl, ImVec2 c, float s, ImU32 accent) {
    dl->AddRectFilled(P(c, s, -0.45f, -0.36f), P(c, s, 0.45f, 0.22f), IM_COL32(60, 66, 80, 255), s * 0.05f);
    dl->AddRectFilledMultiColor(P(c, s, -0.40f, -0.31f), P(c, s, 0.40f, 0.17f), accent, accent,
                                IM_COL32(20, 40, 90, 255), IM_COL32(20, 40, 90, 255));
    dl->AddRectFilled(P(c, s, -0.07f, 0.22f), P(c, s, 0.07f, 0.34f), IM_COL32(90, 96, 110, 255));
    dl->AddRectFilled(P(c, s, -0.25f, 0.33f), P(c, s, 0.25f, 0.39f), IM_COL32(110, 116, 130, 255), s * 0.03f);
}

void Drive(ImDrawList* dl, ImVec2 c, float s, ImU32 accent) {
    dl->AddRectFilled(P(c, s, -0.45f, -0.16f), P(c, s, 0.45f, 0.24f), IM_COL32(150, 158, 172, 255), s * 0.08f);
    dl->AddRectFilled(P(c, s, -0.45f, 0.10f), P(c, s, 0.45f, 0.24f), IM_COL32(110, 116, 130, 255), s * 0.08f,
                      ImDrawFlags_RoundCornersBottom);
    dl->AddCircleFilled(P(c, s, 0.32f, 0.04f), s * 0.05f, accent);
}

}  // namespace

void DrawIcon(ImDrawList* dl, IconType type, ImVec2 c, float s, ImU32 accent) {
    switch (type) {
    case IconType::Folder:      Folder(dl, c, s); break;
    case IconType::Terminal:    Terminal(dl, c, s); break;
    case IconType::TaskManager: TaskManager(dl, c, s, accent); break;
    case IconType::Settings:    Gear(dl, c, s); break;
    case IconType::Power:       Power(dl, c, s, IM_COL32(255, 90, 90, 255)); break;
    case IconType::Computer:    Computer(dl, c, s, accent); break;
    case IconType::Drive:       Drive(dl, c, s, accent); break;
    case IconType::File:        Page(dl, c, s, IM_COL32(190, 198, 212, 255)); break;
    case IconType::TextFile:
        Page(dl, c, s, IM_COL32(190, 198, 212, 255));
        for (int i = 0; i < 4; ++i)
            dl->AddLine(P(c, s, -0.20f, -0.14f + i * 0.13f), P(c, s, i == 3 ? 0.05f : 0.20f, -0.14f + i * 0.13f),
                        IM_COL32(110, 120, 140, 255), ImMax(1.0f, s * 0.04f));
        break;
    case IconType::Image:
        Page(dl, c, s, IM_COL32(120, 190, 120, 255));
        dl->AddRectFilled(P(c, s, -0.24f, -0.18f), P(c, s, 0.24f, 0.26f), IM_COL32(120, 180, 240, 255));
        dl->AddTriangleFilled(P(c, s, -0.24f, 0.26f), P(c, s, -0.04f, -0.02f), P(c, s, 0.14f, 0.26f), IM_COL32(60, 150, 70, 255));
        dl->AddTriangleFilled(P(c, s, 0.02f, 0.26f), P(c, s, 0.14f, 0.08f), P(c, s, 0.24f, 0.26f), IM_COL32(40, 120, 55, 255));
        dl->AddCircleFilled(P(c, s, 0.12f, -0.08f), s * 0.05f, IM_COL32(255, 220, 90, 255));
        break;
    case IconType::Music:
        Page(dl, c, s, IM_COL32(220, 130, 200, 255));
        dl->AddLine(P(c, s, 0.06f, -0.20f), P(c, s, 0.06f, 0.14f), IM_COL32(170, 60, 160, 255), ImMax(1.5f, s * 0.05f));
        dl->AddLine(P(c, s, 0.06f, -0.20f), P(c, s, 0.18f, -0.12f), IM_COL32(170, 60, 160, 255), ImMax(1.5f, s * 0.05f));
        dl->AddCircleFilled(P(c, s, -0.01f, 0.16f), s * 0.08f, IM_COL32(170, 60, 160, 255));
        break;
    case IconType::Video:
        Page(dl, c, s, IM_COL32(240, 120, 100, 255));
        dl->AddTriangleFilled(P(c, s, -0.10f, -0.14f), P(c, s, -0.10f, 0.18f), P(c, s, 0.16f, 0.02f), IM_COL32(220, 70, 60, 255));
        break;
    case IconType::Archive:
        Page(dl, c, s, IM_COL32(230, 190, 90, 255));
        for (int i = 0; i < 5; ++i)
            dl->AddRectFilled(P(c, s, i % 2 ? 0.0f : -0.06f, -0.38f + i * 0.09f),
                              P(c, s, i % 2 ? 0.06f : 0.0f, -0.30f + i * 0.09f), IM_COL32(120, 100, 60, 255));
        dl->AddRectFilled(P(c, s, -0.08f, 0.10f), P(c, s, 0.08f, 0.26f), IM_COL32(200, 160, 60, 255), s * 0.02f);
        break;
    case IconType::Code:
        Page(dl, c, s, IM_COL32(120, 160, 240, 255));
        dl->AddLine(P(c, s, -0.08f, -0.10f), P(c, s, -0.20f, 0.02f), IM_COL32(60, 100, 200, 255), ImMax(1.5f, s * 0.05f));
        dl->AddLine(P(c, s, -0.20f, 0.02f), P(c, s, -0.08f, 0.14f), IM_COL32(60, 100, 200, 255), ImMax(1.5f, s * 0.05f));
        dl->AddLine(P(c, s, 0.08f, -0.10f), P(c, s, 0.20f, 0.02f), IM_COL32(60, 100, 200, 255), ImMax(1.5f, s * 0.05f));
        dl->AddLine(P(c, s, 0.20f, 0.02f), P(c, s, 0.08f, 0.14f), IM_COL32(60, 100, 200, 255), ImMax(1.5f, s * 0.05f));
        break;
    }
}

void InlineIcon(IconType type, float size, ImU32 accent) {
    const ImVec2 p = ImGui::GetCursorScreenPos();
    const float  h = ImGui::GetTextLineHeight();
    DrawIcon(ImGui::GetWindowDrawList(), type, ImVec2(p.x + size * 0.5f, p.y + h * 0.5f), size, accent);
    ImGui::Dummy(ImVec2(size, h));
    ImGui::SameLine();
}
