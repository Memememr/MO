// CSOPESY Desktop OS Emulator - Group 5
// OS.cpp - small helpers shared by every subsystem.
#include "OS.h"
#include "App.h"

#include <ctime>

void OSState::Notify(const std::string& title, const std::string& text) {
    toasts.push_back({title, text, now});
    if (toasts.size() > 4) toasts.erase(toasts.begin());
}

App* OSState::FindApp(const std::string& exeName) const {
    for (const auto& a : apps)
        if (a->exe == exeName) return a.get();
    return nullptr;
}

ImU32 OSState::Accent(float alpha) const {
    ImVec4 c = settings.accent;
    c.w *= alpha;
    return ImGui::ColorConvertFloat4ToU32(c);
}

ImVec2 OSState::WorkArea() const {
    const ImVec2 d = ImGui::GetIO().DisplaySize;
    return ImVec2(d.x, d.y - taskbarHeight);
}

std::string FormatLocalTime(const char* fmt) {
    std::time_t t = std::time(nullptr);
    std::tm     tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[128];
    std::strftime(buf, sizeof(buf), fmt, &tm);
    return buf;
}

std::string ClockString(const Settings& s) {
    if (s.clock24h) return FormatLocalTime(s.showSeconds ? "%H:%M:%S" : "%H:%M");
    return FormatLocalTime(s.showSeconds ? "%I:%M:%S %p" : "%I:%M %p");
}

void TextRightAligned(const char* text) {
    const float w     = ImGui::CalcTextSize(text).x;
    const float avail = ImGui::GetContentRegionAvail().x;
    if (avail > w) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + avail - w);
    ImGui::TextUnformatted(text);
}

void ApplyTheme(const ImVec4& a) {
    ImGuiStyle& st = ImGui::GetStyle();
    ImGui::StyleColorsDark(&st);
    st.WindowRounding    = 8.0f;
    st.ChildRounding     = 6.0f;
    st.FrameRounding     = 5.0f;
    st.PopupRounding     = 8.0f;
    st.GrabRounding      = 4.0f;
    st.TabRounding       = 5.0f;
    st.ScrollbarRounding = 6.0f;
    st.WindowPadding     = ImVec2(12, 10);
    st.FramePadding      = ImVec2(8, 5);
    st.ItemSpacing       = ImVec2(8, 6);
    st.WindowTitleAlign  = ImVec2(0.0f, 0.5f);
    st.WindowMinSize     = ImVec2(200, 120);

    auto rgba = [](int r, int g, int b, int al = 255) { return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, al / 255.0f); };
    auto acc  = [&](float alpha) { return ImVec4(a.x, a.y, a.z, alpha); };
    ImVec4* c = st.Colors;
    c[ImGuiCol_WindowBg]             = rgba(24, 27, 36, 250);
    c[ImGuiCol_PopupBg]              = rgba(28, 31, 42, 250);
    c[ImGuiCol_Border]               = rgba(255, 255, 255, 34);
    c[ImGuiCol_FrameBg]              = rgba(40, 44, 58);
    c[ImGuiCol_FrameBgHovered]       = rgba(52, 57, 74);
    c[ImGuiCol_FrameBgActive]        = rgba(62, 68, 88);
    c[ImGuiCol_TitleBg]              = rgba(20, 22, 30);
    c[ImGuiCol_TitleBgActive]        = rgba(32, 36, 50);
    c[ImGuiCol_TitleBgCollapsed]     = rgba(20, 22, 30);
    c[ImGuiCol_Button]               = rgba(48, 53, 70);
    c[ImGuiCol_ButtonHovered]        = acc(0.65f);
    c[ImGuiCol_ButtonActive]         = acc(0.90f);
    c[ImGuiCol_Header]               = acc(0.35f);
    c[ImGuiCol_HeaderHovered]        = acc(0.50f);
    c[ImGuiCol_HeaderActive]         = acc(0.70f);
    c[ImGuiCol_CheckMark]            = acc(1.00f);
    c[ImGuiCol_SliderGrab]           = acc(0.85f);
    c[ImGuiCol_SliderGrabActive]     = acc(1.00f);
    c[ImGuiCol_Tab]                  = rgba(34, 38, 50);
    c[ImGuiCol_TabHovered]           = acc(0.60f);
    c[ImGuiCol_TabSelected]          = acc(0.45f);
    c[ImGuiCol_TabSelectedOverline]  = acc(1.00f);
    c[ImGuiCol_TabDimmedSelected]    = rgba(44, 48, 62);
    c[ImGuiCol_SeparatorHovered]     = acc(0.70f);
    c[ImGuiCol_SeparatorActive]      = acc(1.00f);
    c[ImGuiCol_ResizeGrip]           = acc(0.20f);
    c[ImGuiCol_ResizeGripHovered]    = acc(0.60f);
    c[ImGuiCol_ResizeGripActive]     = acc(0.90f);
    c[ImGuiCol_TextSelectedBg]       = acc(0.35f);
    c[ImGuiCol_TableHeaderBg]        = rgba(34, 38, 50);
    c[ImGuiCol_TableRowBgAlt]        = rgba(255, 255, 255, 8);
    c[ImGuiCol_ModalWindowDimBg]     = rgba(0, 0, 0, 120);
    c[ImGuiCol_PlotLines]            = acc(1.00f);
}
