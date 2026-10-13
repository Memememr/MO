// CSOPESY Desktop OS Emulator - Group 5
// FileExplorer.cpp - placeholder file browser (unique UI screen #1).
//
// The "file system" is an in-memory tree built at startup. Folders can be
// opened, navigated with Back / Forward / Up or the breadcrumb bar, searched,
// and shown as icons or as a details table. Text files open in a viewer.
#include "Apps.h"
#include "../Icons.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace {

struct Node {
    std::string       name;
    IconType          icon;
    std::string       type;
    std::string       size;       // display string
    std::string       modified;
    std::vector<Node> children;
    std::string       content;    // for text files
    float             usage = -1; // for drives: fraction used

    bool IsDir() const { return icon == IconType::Folder || icon == IconType::Drive || icon == IconType::Computer; }
};

Node Dir(const std::string& name, std::vector<Node> kids, const std::string& date = "09/18/2026 9:00 AM") {
    return {name, IconType::Folder, "File folder", "", date, std::move(kids), "", -1};
}

Node FileN(const std::string& name, const std::string& size, const std::string& date, const std::string& content = "") {
    const std::string ext = name.substr(name.find_last_of('.') + 1);
    IconType icon = IconType::File;
    std::string type = ext + " File";
    if (ext == "txt" || ext == "md")                    { icon = IconType::TextFile; type = "Text Document"; }
    else if (ext == "png" || ext == "jpg")              { icon = IconType::Image;    type = "Image"; }
    else if (ext == "mp3")                              { icon = IconType::Music;    type = "Audio"; }
    else if (ext == "mp4")                              { icon = IconType::Video;    type = "Video"; }
    else if (ext == "zip")                              { icon = IconType::Archive;  type = "Compressed Folder"; }
    else if (ext == "cpp" || ext == "h")                { icon = IconType::Code;     type = "C++ Source"; }
    else if (ext == "pptx")                             { type = "PowerPoint Presentation"; }
    else if (ext == "pdf")                              { type = "PDF Document"; }
    else if (ext == "exe")                              { icon = IconType::Computer; type = "Application"; }
    return {name, icon, type, size, date, {}, content, -1};
}

Node BuildFileSystem() {
    Node docs = Dir("Documents", {
        Dir("CSOPESY", {
            Dir("Lecture Notes", {
                FileN("01 - Processes and Threads.pdf", "2.1 MB", "08/14/2026 1:10 PM"),
                FileN("02 - CPU Scheduling.pdf", "3.4 MB", "08/21/2026 1:05 PM"),
                FileN("03 - Synchronization.pdf", "2.8 MB", "08/28/2026 1:12 PM"),
                FileN("04 - Memory Management.pdf", "4.0 MB", "09/04/2026 1:02 PM"),
            }),
            FileN("MO3 Technical Report.pptx", "18.4 MB", "09/11/2026 11:48 PM"),
            FileN("MO4 Technical Report.pptx", "22.9 MB", "10/02/2026 10:15 PM"),
            FileN("Marquee M03 Phase 1.cpp", "3 KB", "09/04/2026 8:30 PM"),
        }),
        FileN("Group 5 Members.txt", "1 KB", "08/10/2026 4:20 PM",
              "CSOPESY - Group 5\n\nAlvarez\nSanidad\nViray\nQuijano\n\nMax group size: 4 (same section)."),
        FileN("README.txt", "2 KB", "10/02/2026 9:41 PM",
              "CSOPESY Desktop OS Emulator\n\nBuild with CMake (see the real README.txt in the project root).\n"
              "Entry point: src/main.cpp\n\nShut the OS down with the PWR button on the taskbar."),
        FileN("todo.txt", "1 KB", "10/08/2026 7:02 PM",
              "[x] Desktop + wallpaper\n[x] Real-time clock\n[x] Taskbar with 3+ buttons\n[x] Task Manager\n"
              "[x] PWR button shutdown\n[ ] Record the test case videos (no cuts!)\n[ ] Embed MP4 in the PPTX"),
    });
    Node user = Dir("Group5", {
        Dir("Desktop", {
            FileN("notes.txt", "1 KB", "10/09/2026 8:00 AM", "Remember: 480p to 720p video, max 1 GB, no cuts."),
            FileN("test_case_01.mp4", "84.2 MB", "10/05/2026 3:33 PM"),
        }),
        docs,
        Dir("Downloads", {
            FileN("glfw-3.4.zip", "1.5 MB", "09/15/2026 6:10 PM"),
            FileN("imgui-1.91.9b.zip", "1.9 MB", "09/15/2026 6:12 PM"),
            FileN("cmake-3.30-windows-x86_64.msi", "31.2 MB", "09/15/2026 6:20 PM"),
        }),
        Dir("Pictures", {
            FileN("rolling_hills.png", "3.8 MB", "09/16/2026 2:00 PM"),
            FileN("group_photo.jpg", "2.4 MB", "08/10/2026 5:45 PM"),
            FileN("screenshot_001.png", "412 KB", "10/03/2026 9:12 PM"),
            FileN("screenshot_002.png", "398 KB", "10/03/2026 9:14 PM"),
        }),
        Dir("Music", {
            FileN("study_playlist_01.mp3", "6.1 MB", "07/30/2026 10:00 PM"),
            FileN("chill_beats.mp3", "4.7 MB", "07/30/2026 10:02 PM"),
        }),
        Dir("Videos", {
            FileN("MO3_test_cases.mp4", "212.0 MB", "09/12/2026 12:30 AM"),
            FileN("MO4_demo.mp4", "180.5 MB", "10/04/2026 11:58 PM"),
        }),
    });
    Node c = {"Local Disk (C:)", IconType::Drive, "Local Disk", "128 GB free of 476 GB", "", {
        Dir("CSOPESY", {Dir("System32", {
            FileN("kernel.sys", "1.2 MB", "09/18/2026 9:00 AM"),
            FileN("scheduler.dll", "640 KB", "09/18/2026 9:00 AM"),
            FileN("marquee.dll", "88 KB", "09/18/2026 9:00 AM"),
            FileN("compositor.exe", "2.3 MB", "09/18/2026 9:00 AM"),
        })}),
        Dir("Program Files", {
            Dir("CSOPESY Desktop", {FileN("csopesy_desktop.exe", "3.1 MB", "10/02/2026 9:40 PM")}),
            Dir("Dear ImGui", {FileN("imgui.h", "320 KB", "09/15/2026 6:12 PM")}),
        }),
        Dir("Users", {user}),
    }, "", 0.73f};
    Node dDrive = {"Data (D:)", IconType::Drive, "Local Disk", "702 GB free of 931 GB", "", {
        Dir("Backups", {FileN("MO1_backup.zip", "12.3 MB", "08/20/2026 11:00 PM"),
                        FileN("MO3_backup.zip", "15.8 MB", "09/12/2026 1:00 AM")}),
        Dir("Projects", {Dir("csopesy-desktop", {
            FileN("main.cpp", "5 KB", "10/02/2026 9:40 PM"),
            FileN("Compositor.cpp", "9 KB", "10/02/2026 9:40 PM"),
            FileN("Taskbar.cpp", "11 KB", "10/02/2026 9:40 PM"),
            FileN("README.txt", "2 KB", "10/02/2026 9:41 PM", "See the README.txt in the project root."),
        })}),
    }, "", 0.25f};
    return {"This PC", IconType::Computer, "System", "", "", {c, dDrive}, "", -1};
}

std::string Lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char ch) { return (char)std::tolower(ch); });
    return s;
}

class FileExplorerApp : public App {
public:
    FileExplorerApp() : App("File Explorer", "explorer.exe", IconType::Folder, ImVec2(860, 520), 1.1f, 38.6f) {
        minSize = ImVec2(560, 320);
        root = BuildFileSystem();
        // Quick-access shortcuts: paths of child indices from the root.
        const std::vector<int> user = {0, 2, 0};   // C: > Users > Group5
        const char* names[] = {"Desktop", "Documents", "Downloads", "Pictures", "Music", "Videos"};
        const IconType icons[] = {IconType::Computer, IconType::TextFile, IconType::Archive,
                                  IconType::Image, IconType::Music, IconType::Video};
        for (int i = 0; i < 6; ++i) {
            auto p = user;
            p.push_back(i);
            quick.push_back({names[i], icons[i], p});
        }
    }

protected:
    void OnOpen(OSState&) override {
        path = {0, 2, 0};   // open in the user's home folder
        back.clear();
        forward.clear();
        selected = -1;
        search[0] = '\0';
    }

    void DrawContent(OSState& os) override {
        DrawToolbar(os);
        const float statusH = ImGui::GetTextLineHeightWithSpacing() + 4.0f;

        ImGui::BeginChild("##sidebar", ImVec2(190, -statusH), ImGuiChildFlags_Borders);
        DrawSidebar(os);
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("##content", ImVec2(0, -statusH), ImGuiChildFlags_Borders);
        visibleItems.clear();
        const Node& cur = Current();
        const std::string q = Lower(search);
        for (int i = 0; i < (int)cur.children.size(); ++i)
            if (q.empty() || Lower(cur.children[i].name).find(q) != std::string::npos) visibleItems.push_back(i);
        if (visibleItems.empty()) ImGui::TextDisabled(q.empty() ? "This folder is empty." : "No items match your search.");
        else if (detailsView) DrawDetails(os, cur);
        else DrawGrid(os, cur);
        ImGui::EndChild();

        ImGui::TextDisabled("%d items", (int)visibleItems.size());
        if (selected >= 0 && selected < (int)cur.children.size()) {
            const Node& n = cur.children[selected];
            ImGui::SameLine();
            ImGui::TextDisabled("  |  1 item selected  %s", n.size.c_str());
        }
        DrawViewer();
    }

private:
    struct Shortcut { const char* name; IconType icon; std::vector<int> path; };

    Node                  root;
    std::vector<int>      path;
    std::vector<std::vector<int>> back, forward;
    std::vector<Shortcut> quick;
    std::vector<int>      visibleItems;
    int   selected    = -1;
    bool  detailsView = false;
    char  search[64]  = "";
    const Node* viewing = nullptr;

    const Node& NodeAt(const std::vector<int>& p) const {
        const Node* n = &root;
        for (int i : p) n = &n->children[i];
        return *n;
    }
    const Node& Current() const { return NodeAt(path); }

    void Navigate(const std::vector<int>& to) {
        if (to == path) return;
        back.push_back(path);
        forward.clear();
        path = to;
        selected = -1;
        search[0] = '\0';
    }

    void Activate(int index) {
        const Node& n = Current().children[index];
        if (n.IsDir()) {
            auto p = path;
            p.push_back(index);
            Navigate(p);
        } else {
            viewing = &n;
        }
    }

    void DrawToolbar(OSState& os) {
        ImGui::BeginDisabled(back.empty());
        if (ImGui::ArrowButton("##back", ImGuiDir_Left)) { forward.push_back(path); path = back.back(); back.pop_back(); selected = -1; }
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(forward.empty());
        if (ImGui::ArrowButton("##fwd", ImGuiDir_Right)) { back.push_back(path); path = forward.back(); forward.pop_back(); selected = -1; }
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(path.empty());
        if (ImGui::ArrowButton("##up", ImGuiDir_Up)) { auto p = path; p.pop_back(); Navigate(p); }
        ImGui::EndDisabled();
        ImGui::SameLine();

        // Breadcrumb address bar
        const ImGuiStyle& st = ImGui::GetStyle();
        const float searchW = 170.0f;
        const float viewW   = 2.0f * (ImGui::GetFrameHeight() + st.ItemInnerSpacing.x) + ImGui::CalcTextSize("Icons").x +
                              ImGui::CalcTextSize("Details").x + st.ItemSpacing.x;
        const float barW    = ImGui::GetContentRegionAvail().x - searchW - viewW - 2.0f * st.ItemSpacing.x;
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
        ImGui::BeginChild("##address", ImVec2(barW, ImGui::GetFrameHeight()), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
        std::vector<int> partial;
        const Node* n = &root;
        for (size_t i = 0; i <= path.size(); ++i) {
            if (i > 0) {
                partial.push_back(path[i - 1]);
                n = &n->children[path[i - 1]];
                ImGui::SameLine();
                ImGui::TextDisabled(">");
                ImGui::SameLine();
            }
            ImGui::PushID((int)i);
            if (ImGui::Button(n->name.c_str())) Navigate(partial);
            ImGui::PopID();
        }
        ImGui::SetScrollHereX(1.0f);   // deep paths: keep the current folder visible
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::SetNextItemWidth(searchW);
        ImGui::InputTextWithHint("##search", "Search this folder", search, sizeof(search));
        ImGui::SameLine();
        if (ImGui::RadioButton("Icons", !detailsView)) detailsView = false;
        ImGui::SameLine();
        if (ImGui::RadioButton("Details", detailsView)) detailsView = true;
        (void)os;
    }

    void SidebarItem(OSState& os, const char* name, IconType icon, const std::vector<int>& p) {
        ImGui::PushID(name);
        InlineIcon(icon, 18.0f, os.Accent());
        if (ImGui::Selectable(name, path == p)) Navigate(p);
        ImGui::PopID();
    }

    void DrawSidebar(OSState& os) {
        ImGui::TextDisabled("Quick access");
        for (const auto& q : quick) SidebarItem(os, q.name, q.icon, q.path);
        ImGui::Spacing();
        ImGui::TextDisabled("Devices");
        SidebarItem(os, "This PC", IconType::Computer, {});
        SidebarItem(os, "Local Disk (C:)", IconType::Drive, {0});
        SidebarItem(os, "Data (D:)", IconType::Drive, {1});
    }

    void DrawGrid(OSState& os, const Node& cur) {
        const float cellW = 104.0f, cellH = 120.0f;
        const int   cols  = ImMax(1, (int)(ImGui::GetContentRegionAvail().x / (cellW + 6.0f)));
        ImDrawList* dl = ImGui::GetWindowDrawList();
        for (int k = 0; k < (int)visibleItems.size(); ++k) {
            const int   i = visibleItems[k];
            const Node& n = cur.children[i];
            if (k % cols != 0) ImGui::SameLine(0, 6.0f);
            ImGui::PushID(i);
            const ImVec2 p = ImGui::GetCursorScreenPos();
            if (ImGui::InvisibleButton("##item", ImVec2(cellW, cellH))) selected = i;
            const bool hovered = ImGui::IsItemHovered();
            if (hovered && ImGui::IsMouseDoubleClicked(0)) Activate(i);
            if (selected == i) {
                dl->AddRectFilled(p, ImVec2(p.x + cellW, p.y + cellH), os.Accent(0.30f), 6.0f);
                dl->AddRect(p, ImVec2(p.x + cellW, p.y + cellH), os.Accent(0.8f), 6.0f);
            } else if (hovered) {
                dl->AddRectFilled(p, ImVec2(p.x + cellW, p.y + cellH), IM_COL32(255, 255, 255, 16), 6.0f);
            }
            DrawIcon(dl, n.icon, ImVec2(p.x + cellW * 0.5f, p.y + 34.0f), 50.0f, os.Accent());
            if (n.usage >= 0.0f) {   // drive capacity bar
                const ImVec2 b0(p.x + 14, p.y + 64), b1(p.x + cellW - 14, p.y + 70);
                dl->AddRectFilled(b0, b1, IM_COL32(60, 66, 80, 255), 2.0f);
                dl->AddRectFilled(b0, ImVec2(b0.x + (b1.x - b0.x) * n.usage, b1.y), os.Accent(), 2.0f);
            }
            // Label: wrapped to two lines, centred.
            const float  wrap = cellW - 8.0f;
            const ImVec2 ts   = ImGui::CalcTextSize(n.name.c_str(), nullptr, false, wrap);
            const float  ty   = p.y + (n.usage >= 0.0f ? 74.0f : 66.0f);
            dl->PushClipRect(ImVec2(p.x, ty), ImVec2(p.x + cellW, p.y + cellH), true);
            dl->AddText(nullptr, 0.0f, ImVec2(p.x + (cellW - ImMin(ts.x, wrap)) * 0.5f, ty), IM_COL32(230, 234, 242, 255),
                        n.name.c_str(), nullptr, wrap);
            dl->PopClipRect();
            if (hovered) ImGui::SetTooltip("%s\nType: %s%s%s", n.name.c_str(), n.type.c_str(),
                                           n.size.empty() ? "" : "\nSize: ", n.size.c_str());
            ImGui::PopID();
        }
        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) selected = -1;
    }

    void DrawDetails(OSState& os, const Node& cur) {
        const ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV |
                                      ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit;
        if (!ImGui::BeginTable("##details", 4, flags)) return;
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Date modified", ImGuiTableColumnFlags_WidthFixed, 140);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 130);
        ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 70);
        ImGui::TableHeadersRow();
        for (int i : visibleItems) {
            const Node& n = cur.children[i];
            ImGui::PushID(i);
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            InlineIcon(n.icon, 18.0f, os.Accent());
            if (ImGui::Selectable(n.name.c_str(), selected == i, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {
                selected = i;
                if (ImGui::IsMouseDoubleClicked(0)) Activate(i);
            }
            ImGui::TableSetColumnIndex(1); ImGui::TextDisabled("%s", n.modified.c_str());
            ImGui::TableSetColumnIndex(2); ImGui::TextDisabled("%s", n.type.c_str());
            ImGui::TableSetColumnIndex(3); ImGui::TextDisabled("%s", n.size.c_str());
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    // Text files open in a small viewer; other files show their properties.
    void DrawViewer() {
        if (viewing) ImGui::OpenPopup("File Viewer");
        ImGui::SetNextWindowSize(ImVec2(440, 0), ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal("File Viewer", nullptr, ImGuiWindowFlags_NoSavedSettings)) {
            if (viewing) {
                ImGui::TextUnformatted(viewing->name.c_str());
                ImGui::TextDisabled("%s  |  %s  |  Modified %s", viewing->type.c_str(), viewing->size.c_str(), viewing->modified.c_str());
                ImGui::Separator();
                if (!viewing->content.empty()) {
                    ImGui::BeginChild("##text", ImVec2(0, 180), ImGuiChildFlags_Borders);
                    ImGui::TextWrapped("%s", viewing->content.c_str());
                    ImGui::EndChild();
                } else {
                    ImGui::TextWrapped("This is a placeholder file. CSOPESY OS has no application installed to open '%s' files.",
                                       viewing->type.c_str());
                }
            }
            if (ImGui::Button("Close", ImVec2(100, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                viewing = nullptr;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
};

}  // namespace

std::unique_ptr<App> MakeFileExplorer() { return std::make_unique<FileExplorerApp>(); }
