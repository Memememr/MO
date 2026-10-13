// CSOPESY Desktop OS Emulator - Group 5
// App.cpp - window lifecycle shared by every application.
#include "App.h"

#include "imgui_internal.h"   // title-bar rect for the custom minimize button

App::App(std::string title_, std::string exe_, IconType icon_, ImVec2 defaultSize_,
         float baseCpu_, float baseMem_)
    : title(std::move(title_)), exe(std::move(exe_)), icon(icon_),
      defaultSize(defaultSize_), baseCpu(baseCpu_), baseMem(baseMem_) {}

void App::Open(OSState& os) {
    if (!isOpen) {
        isOpen    = true;
        pid       = os.nextPid;
        os.nextPid += 4 + (os.nextPid % 7) * 4;   // PIDs that look "real"
        startedAt = os.now;
        stats.cpu = baseCpu;
        stats.mem = baseMem;
        OnOpen(os);
    }
    Restore();
}

void App::Close() {
    if (!isOpen) return;
    OnClose();
    isOpen = isMinimized = isFocused = false;
    pid = 0;
    stats = ProcStats{};
}

void App::Minimize() {
    isMinimized = true;
    isFocused   = false;
}

void App::Restore() {
    isMinimized    = false;
    focusRequested = true;
}

// Draws a "_" button just left of ImGui's close button in the title bar.
bool App::DrawMinimizeButton() {
    ImGuiWindow*      win = ImGui::GetCurrentWindow();
    const ImGuiStyle& st  = ImGui::GetStyle();
    const ImRect      bar = win->TitleBarRect();
    const float       sz  = ImGui::GetFontSize();

    const ImVec2 pos(bar.Max.x - st.FramePadding.x - sz * 2.0f - st.ItemInnerSpacing.x,
                     bar.Min.y + st.FramePadding.y);
    const ImVec2 backup = ImGui::GetCursorScreenPos();

    ImGui::PushClipRect(bar.Min, bar.Max, false);
    ImGui::SetCursorScreenPos(pos);
    const bool pressed = ImGui::InvisibleButton("##minimize", ImVec2(sz, sz));
    const bool hovered = ImGui::IsItemHovered();
    const bool held    = ImGui::IsItemActive();

    ImDrawList*  dl = ImGui::GetWindowDrawList();
    const ImVec2 c(pos.x + sz * 0.5f, pos.y + sz * 0.5f);
    if (hovered)
        dl->AddCircleFilled(c, sz * 0.5f + 1.0f,
                            ImGui::GetColorU32(held ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered));
    const float h = sz * 0.25f;
    dl->AddLine(ImVec2(c.x - h, c.y + h * 0.6f), ImVec2(c.x + h, c.y + h * 0.6f),
                ImGui::GetColorU32(ImGuiCol_Text), 1.5f);
    if (hovered) ImGui::SetTooltip("Minimize");
    ImGui::PopClipRect();

    ImGui::SetCursorScreenPos(backup);
    return pressed;
}

void App::Draw(OSState& os) {
    if (!isOpen || isMinimized) {
        isFocused = false;
        return;
    }

    const ImVec2 work = os.WorkArea();
    ImGui::SetNextWindowPos(ImVec2(150.0f + 42.0f * slot, 50.0f + 34.0f * slot), ImGuiCond_Once);
    ImGui::SetNextWindowSize(defaultSize, ImGuiCond_Once);
    ImGui::SetNextWindowSizeConstraints(minSize, ImVec2(work.x, work.y));
    if (focusRequested) {
        ImGui::SetNextWindowFocus();
        focusRequested = false;
    }

    bool keepOpen = true;
    const std::string id = title + "###" + exe;   // stable ID even if the title changes
    ImGui::Begin(id.c_str(), &keepOpen, ImGuiWindowFlags_NoCollapse);
    isFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    if (isFocused) os.activeApp = this;

    const bool minimize = DrawMinimizeButton();

    // Keep the title bar reachable: never let a window slide under the taskbar
    // or completely off-screen.
    const ImVec2 p = ImGui::GetWindowPos();
    const ImVec2 s = ImGui::GetWindowSize();
    ImVec2 c = p;
    c.x = ImClamp(c.x, 80.0f - s.x, work.x - 80.0f);
    c.y = ImClamp(c.y, 0.0f, work.y - ImGui::GetFrameHeight());
    if (c.x != p.x || c.y != p.y) ImGui::SetWindowPos(c);

    DrawContent(os);
    ImGui::End();

    if (minimize) Minimize();
    if (!keepOpen) Close();
}
