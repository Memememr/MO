// CSOPESY Desktop OS Emulator - Group 5
// TaskManager.cpp - a Windows-style Task Manager with dummy usage values.
//
// "Processes" lists the apps that are really open in the emulator plus a set of
// fake background services. All CPU / memory / disk / network numbers are
// randomly generated around a base value, refreshed at the selected update
// speed, and recorded into 60-sample histories for the Performance graphs.
#include "Apps.h"
#include "../Icons.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <vector>

namespace {

constexpr int kHistory = 60;

const ImU32 kCpuCol  = IM_COL32(64, 160, 240, 255);
const ImU32 kMemCol  = IM_COL32(190, 96, 232, 255);
const ImU32 kDiskCol = IM_COL32(112, 200, 64, 255);
const ImU32 kNetCol  = IM_COL32(232, 144, 52, 255);

struct BgProcess {
    BgProcess(const char* n, const char* e, IconType i, float cpu, float mem, bool crit)
        : name(n), exe(e), icon(i), baseCpu(cpu), baseMem(mem), critical(crit) {}
    std::string name;
    std::string exe;
    IconType    icon;
    float       baseCpu;
    float       baseMem;
    bool        critical;
    int         pid = 0;
    bool        sleeping = false;
    ProcStats   st;
};

// One row of the Processes table (either a real app or a dummy service).
struct Row {
    const char*      name;
    const char*      exe;
    IconType         icon;
    const char*      status;
    ImU32            statusCol;
    int              pid;
    const ProcStats* st;
    App*             app;
    int              bgIndex;
};

void DrawGraph(ImDrawList* dl, ImVec2 p, ImVec2 s, const float* data, int offset, float maxV, ImU32 col, bool grid) {
    dl->AddRectFilled(p, ImVec2(p.x + s.x, p.y + s.y), IM_COL32(14, 18, 26, 255));
    if (grid) {
        for (int i = 1; i < 10; ++i) dl->AddLine(ImVec2(p.x + s.x * i / 10, p.y), ImVec2(p.x + s.x * i / 10, p.y + s.y), IM_COL32(40, 48, 64, 255));
        for (int i = 1; i < 5; ++i)  dl->AddLine(ImVec2(p.x, p.y + s.y * i / 5), ImVec2(p.x + s.x, p.y + s.y * i / 5), IM_COL32(40, 48, 64, 255));
    }
    ImVec2 pts[kHistory];
    for (int i = 0; i < kHistory; ++i) {
        const float v = ImClamp(data[(offset + i) % kHistory] / maxV, 0.0f, 1.0f);
        pts[i] = ImVec2(p.x + s.x * i / (kHistory - 1), p.y + s.y - 1.0f - (s.y - 2.0f) * v);
    }
    const ImU32 fill = (col & 0x00FFFFFF) | (60u << 24);
    for (int i = 0; i + 1 < kHistory; ++i)
        dl->AddQuadFilled(pts[i], pts[i + 1], ImVec2(pts[i + 1].x, p.y + s.y), ImVec2(pts[i].x, p.y + s.y), fill);
    dl->AddPolyline(pts, kHistory, col, 0, 1.5f);
    dl->AddRect(p, ImVec2(p.x + s.x, p.y + s.y), col);
}

std::string Uptime(double seconds) {
    const int t = (int)seconds;
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%d:%02d:%02d:%02d", t / 86400, (t / 3600) % 24, (t / 60) % 60, t % 60);
    return buf;
}

class TaskManagerApp : public App {
public:
    TaskManagerApp() : App("Task Manager", "taskmgr.exe", IconType::TaskManager, ImVec2(820, 540), 2.1f, 26.8f) {
        minSize = ImVec2(620, 380);
        bg = {
            {"System", "system", IconType::Settings, 0.3f, 0.2f, true},
            {"CSOPESY Kernel", "csopesy.sys", IconType::Settings, 1.1f, 12.4f, true},
            {"Desktop Compositor", "dcomp.exe", IconType::Computer, 3.2f, 64.5f, true},
            {"Session Manager", "smss.exe", IconType::Settings, 0.0f, 1.1f, true},
            {"Windowing Shell Host", "shellhost.exe", IconType::Computer, 0.8f, 38.2f, true},
            {"Service Host: Audio", "svchost.exe", IconType::Music, 0.5f, 7.8f, false},
            {"Service Host: Network", "svchost.exe", IconType::Drive, 0.3f, 5.3f, false},
            {"Marquee Daemon", "marqueed.exe", IconType::Terminal, 0.7f, 3.2f, false},
            {"Clock Service", "timesvc.exe", IconType::Settings, 0.1f, 2.0f, false},
            {"Input Polling Service", "inputsvc.exe", IconType::Settings, 0.4f, 2.6f, false},
            {"Font Cache Service", "fontcache.exe", IconType::TextFile, 0.0f, 4.1f, false},
            {"Print Spooler", "spoolsv.exe", IconType::File, 0.0f, 6.0f, false},
            {"Animo Guard Antivirus", "animoguard.exe", IconType::Settings, 1.4f, 85.0f, false},
            {"Search Indexer", "searchidx.exe", IconType::Folder, 0.3f, 21.7f, false},
            {"Update Orchestrator", "updater.exe", IconType::Archive, 0.0f, 9.4f, false},
        };
        int pid = 4;
        for (auto& p : bg) {
            p.pid = pid;
            pid += 88 + (pid % 5) * 36;
            p.st.cpu = p.baseCpu;
            p.st.mem = p.baseMem;
        }
        std::fill(std::begin(cpuHist), std::end(cpuHist), 0.0f);
        std::fill(std::begin(memHist), std::end(memHist), 0.0f);
        std::fill(std::begin(diskHist), std::end(diskHist), 0.0f);
        std::fill(std::begin(netHist), std::end(netHist), 0.0f);
    }

protected:
    void OnOpen(OSState& os) override {
        // Pre-fill the 60-second history so the graphs aren't empty on open.
        for (int i = 0; i < kHistory; ++i) Tick(os);
    }

    void DrawContent(OSState& os) override {
        static const float kIntervals[] = {0.5f, 1.0f, 4.0f, -1.0f};
        const float interval = kIntervals[speedIdx];
        if (interval > 0.0f && os.now - lastTick >= interval) Tick(os);

        if (ImGui::BeginTabBar("##tm_tabs")) {
            if (ImGui::BeginTabItem("Processes")) {
                DrawProcesses(os);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Performance")) {
                DrawPerformance(os);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        DrawMessagePopup();
    }

private:
    std::vector<BgProcess> bg;
    std::mt19937           rng{20260918u};
    double lastTick   = 0.0;
    int    speedIdx   = 1;   // High, Normal, Low, Paused
    int    selectedPid = -1;
    int    perfSel    = 0;
    char   runBuf[64] = "";
    std::string runError;
    std::string message;

    float cpuHist[kHistory], memHist[kHistory], diskHist[kHistory], netHist[kHistory];
    int   histPos = 0;
    float totalCpu = 0, totalMemMB = 0, totalDisk = 0, totalNet = 0;

    static constexpr float kRamMB      = 16384.0f;
    static constexpr float kReservedMB = 3150.0f;   // "kernel + drivers" baseline

    float Uniform(float a, float b) { return std::uniform_real_distribution<float>(a, b)(rng); }

    void Simulate(ProcStats& st, float baseCpu, float baseMem, bool suspended) {
        st.cpu  = suspended ? 0.0f : ImMax(0.0f, baseCpu * Uniform(0.3f, 1.7f) + (Uniform(0, 1) < 0.06f ? Uniform(2, 9) : 0.0f));
        st.mem += (baseMem - st.mem) * 0.15f + baseMem * Uniform(-0.03f, 0.03f);
        st.mem  = ImMax(0.1f, st.mem);
        st.disk = (!suspended && Uniform(0, 1) < 0.25f) ? Uniform(0.1f, 4.0f) : 0.0f;
        st.net  = (!suspended && Uniform(0, 1) < 0.20f) ? Uniform(0.1f, 2.5f) : 0.0f;
    }

    void Tick(OSState& os) {
        lastTick = os.now;
        totalCpu = totalMemMB = totalDisk = totalNet = 0.0f;
        for (auto& a : os.apps) {
            if (!a->isOpen) continue;
            Simulate(a->stats, a->baseCpu, a->baseMem, a->isMinimized);
            totalCpu += a->stats.cpu; totalMemMB += a->stats.mem; totalDisk += a->stats.disk; totalNet += a->stats.net;
        }
        for (auto& p : bg) {
            if (!p.critical && Uniform(0, 1) < 0.08f) p.sleeping = !p.sleeping;
            Simulate(p.st, p.baseCpu, p.baseMem, p.sleeping);
            totalCpu += p.st.cpu; totalMemMB += p.st.mem; totalDisk += p.st.disk; totalNet += p.st.net;
        }
        totalCpu = ImMin(totalCpu, 100.0f);
        totalMemMB += kReservedMB;

        cpuHist[histPos]  = totalCpu;
        memHist[histPos]  = totalMemMB / kRamMB * 100.0f;
        diskHist[histPos] = ImMin(100.0f, totalDisk * 2.5f);
        netHist[histPos]  = totalNet * 1000.0f / 8.0f;   // Mbps -> kbps-ish scale for display
        histPos = (histPos + 1) % kHistory;
    }

    std::vector<Row> BuildRows(OSState& os, bool apps) {
        std::vector<Row> rows;
        if (apps) {
            for (auto& a : os.apps) {
                if (!a->isOpen) continue;
                rows.push_back({a->title.c_str(), a->exe.c_str(), a->icon,
                                a->isMinimized ? "Suspended" : "Running",
                                a->isMinimized ? IM_COL32(240, 180, 70, 255) : IM_COL32(120, 220, 140, 255),
                                a->pid, &a->stats, a.get(), -1});
            }
        } else {
            for (int i = 0; i < (int)bg.size(); ++i) {
                auto& p = bg[i];
                rows.push_back({p.name.c_str(), p.exe.c_str(), p.icon, p.sleeping ? "Sleeping" : "Running",
                                p.sleeping ? IM_COL32(150, 156, 170, 255) : IM_COL32(120, 220, 140, 255),
                                p.pid, &p.st, nullptr, i});
            }
        }
        return rows;
    }

    static void SortRows(std::vector<Row>& rows, const ImGuiTableSortSpecs* specs) {
        if (!specs || specs->SpecsCount == 0) return;
        const ImGuiTableColumnSortSpecs& s = specs->Specs[0];
        std::stable_sort(rows.begin(), rows.end(), [&](const Row& a, const Row& b) {
            int d = 0;
            switch (s.ColumnIndex) {
            case 0: d = std::strcmp(a.name, b.name); break;
            case 1: d = a.pid - b.pid; break;
            case 2: d = std::strcmp(a.status, b.status); break;
            case 3: d = (a.st->cpu > b.st->cpu) - (a.st->cpu < b.st->cpu); break;
            case 4: d = (a.st->mem > b.st->mem) - (a.st->mem < b.st->mem); break;
            case 5: d = (a.st->disk > b.st->disk) - (a.st->disk < b.st->disk); break;
            case 6: d = (a.st->net > b.st->net) - (a.st->net < b.st->net); break;
            }
            return s.SortDirection == ImGuiSortDirection_Ascending ? d < 0 : d > 0;
        });
    }

    // Numeric cell shaded yellow like Windows' "heat map" columns.
    static void HeatCell(const char* text, float intensity) {
        const int a = (int)(18 + 150 * ImClamp(intensity, 0.0f, 1.0f));
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(230, 180, 40, a));
        TextRightAligned(text);
    }

    void DrawRow(OSState& os, const Row& r) {
        ImGui::PushID(r.pid);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        InlineIcon(r.icon, 16.0f, os.Accent());
        if (ImGui::Selectable(r.name, selectedPid == r.pid, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap))
            selectedPid = r.pid;
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0) && r.app) r.app->Restore();
        if (ImGui::BeginPopupContextItem("##rowctx")) {
            selectedPid = r.pid;
            if (r.app && ImGui::MenuItem("Switch to")) r.app->Restore();
            if (ImGui::MenuItem("End task")) EndTask(os);
            ImGui::Separator();
            ImGui::TextDisabled("Image name: %s", r.exe);
            ImGui::EndPopup();
        }

        char buf[32];
        ImGui::TableSetColumnIndex(1);
        ImGui::TextDisabled("%d", r.pid);
        ImGui::TableSetColumnIndex(2);
        ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(r.statusCol), "%s", r.status);
        ImGui::TableSetColumnIndex(3);
        std::snprintf(buf, sizeof(buf), "%.1f%%", r.st->cpu);
        HeatCell(buf, r.st->cpu / 10.0f);
        ImGui::TableSetColumnIndex(4);
        std::snprintf(buf, sizeof(buf), "%.1f MB", r.st->mem);
        HeatCell(buf, r.st->mem / 120.0f);
        ImGui::TableSetColumnIndex(5);
        std::snprintf(buf, sizeof(buf), "%.1f MB/s", r.st->disk);
        HeatCell(buf, r.st->disk / 4.0f);
        ImGui::TableSetColumnIndex(6);
        std::snprintf(buf, sizeof(buf), "%.1f Mbps", r.st->net);
        HeatCell(buf, r.st->net / 2.5f);
        ImGui::PopID();
    }

    void DrawProcesses(OSState& os) {
        // Toolbar
        if (ImGui::Button("Run new task")) {
            runBuf[0] = '\0';
            runError.clear();
            ImGui::OpenPopup("Create new task");
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(120);
        static const char* const kSpeeds[] = {"High (0.5s)", "Normal (1s)", "Low (4s)", "Paused"};
        ImGui::Combo("Update speed", &speedIdx, kSpeeds, IM_ARRAYSIZE(kSpeeds));
        ImGui::SameLine();
        if (ImGui::Button("Refresh now")) Tick(os);
        DrawRunDialog(os);

        auto apps = BuildRows(os, true);
        auto bgs  = BuildRows(os, false);
        const float footer = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y * 2.0f + 6.0f;

        const ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable | ImGuiTableFlags_RowBg |
                                      ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_ScrollY |
                                      ImGuiTableFlags_SizingFixedFit;
        if (ImGui::BeginTable("##procs", 7, flags, ImVec2(0, -footer))) {
            char cpuH[32], memH[32], diskH[32], netH[32];
            std::snprintf(cpuH, sizeof(cpuH), "CPU %.0f%%###cpu", totalCpu);
            std::snprintf(memH, sizeof(memH), "Memory %.0f%%###mem", totalMemMB / kRamMB * 100.0f);
            std::snprintf(diskH, sizeof(diskH), "Disk %.0f%%###disk", ImMin(100.0f, totalDisk * 2.5f));
            std::snprintf(netH, sizeof(netH), "Network %.0f%%###net", ImMin(100.0f, totalNet));
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide);
            ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 52);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 78);
            ImGui::TableSetupColumn(cpuH, ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending, 84);
            ImGui::TableSetupColumn(memH, ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending, 104);
            ImGui::TableSetupColumn(diskH, ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending, 84);
            ImGui::TableSetupColumn(netH, ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending, 112);
            ImGui::TableHeadersRow();

            const ImGuiTableSortSpecs* specs = ImGui::TableGetSortSpecs();
            SortRows(apps, specs);
            SortRows(bgs, specs);

            for (int g = 0; g < 2; ++g) {
                const auto& rows = g == 0 ? apps : bgs;
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                char label[64];
                std::snprintf(label, sizeof(label), "%s (%d)###group%d", g == 0 ? "Apps" : "Background processes", (int)rows.size(), g);
                if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAllColumns)) {
                    for (const Row& r : rows) DrawRow(os, r);
                    ImGui::TreePop();
                }
            }
            ImGui::EndTable();
        }

        // Footer: summary + End task
        ImGui::Dummy(ImVec2(0, 2));
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("Processes: %d    CPU usage: %.0f%%    Physical memory: %.0f%%", (int)(apps.size() + bgs.size()),
                            totalCpu, totalMemMB / kRamMB * 100.0f);
        ImGui::SameLine(ImGui::GetContentRegionMax().x - 90.0f);
        ImGui::BeginDisabled(selectedPid < 0);
        if (ImGui::Button("End task", ImVec2(90, 0))) EndTask(os);
        ImGui::EndDisabled();
    }

    void DrawRunDialog(OSState& os) {
        if (!ImGui::BeginPopupModal("Create new task", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;
        ImGui::TextUnformatted("Type the name of a program and CSOPESY OS will open it.");
        ImGui::TextDisabled("Try: explorer.exe, terminal.exe, taskmgr.exe, settings.exe");
        if (ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere();
        ImGui::SetNextItemWidth(320);
        const bool enter = ImGui::InputText("Open", runBuf, sizeof(runBuf), ImGuiInputTextFlags_EnterReturnsTrue);
        if (!runError.empty()) ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.45f, 1.0f), "%s", runError.c_str());
        if (ImGui::Button("OK", ImVec2(90, 0)) || enter) {
            std::string exe = runBuf;
            if (!exe.empty() && exe.find('.') == std::string::npos) exe += ".exe";
            if (App* a = os.FindApp(exe)) {
                a->Open(os);
                os.Notify("Task started", a->title + " (" + a->exe + ")");
                ImGui::CloseCurrentPopup();
            } else {
                runError = "CSOPESY OS cannot find '" + std::string(runBuf) + "'.";
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(90, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    void EndTask(OSState& os) {
        for (auto& a : os.apps) {
            if (a->isOpen && a->pid == selectedPid) {
                if (a.get() == this) { message = "Task Manager cannot end itself. Close the window instead."; return; }
                os.Notify("Task ended", a->title + " (PID " + std::to_string(a->pid) + ") was terminated.");
                a->Close();
                selectedPid = -1;
                return;
            }
        }
        for (size_t i = 0; i < bg.size(); ++i) {
            if (bg[i].pid != selectedPid) continue;
            if (bg[i].critical) {
                message = "Access is denied.\n\n'" + bg[i].name + "' is a critical system process.\nEnding it would crash CSOPESY OS.";
                return;
            }
            os.Notify("Task ended", bg[i].name + " (PID " + std::to_string(bg[i].pid) + ") was terminated.");
            bg.erase(bg.begin() + (long)i);
            selectedPid = -1;
            return;
        }
    }

    void DrawMessagePopup() {
        if (!message.empty()) ImGui::OpenPopup("Task Manager##msg");
        if (ImGui::BeginPopupModal("Task Manager##msg", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextUnformatted(message.c_str());
            ImGui::Spacing();
            if (ImGui::Button("OK", ImVec2(100, 0)) || ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                message.clear();
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    float Latest(const float* h) const { return h[(histPos + kHistory - 1) % kHistory]; }

    void DrawPerformance(OSState& os) {
        struct Res { const char* name; const float* hist; ImU32 col; float maxV; };
        float netMax = 100.0f;
        for (float v : netHist) netMax = ImMax(netMax, v * 1.25f);
        const Res res[] = {{"CPU", cpuHist, kCpuCol, 100.0f},
                           {"Memory", memHist, kMemCol, 100.0f},
                           {"Disk 0 (C:)", diskHist, kDiskCol, 100.0f},
                           {"Ethernet", netHist, kNetCol, netMax}};

        ImGui::BeginChild("##perf_list", ImVec2(230, 0), ImGuiChildFlags_Borders);
        for (int i = 0; i < 4; ++i) {
            ImGui::PushID(i);
            const ImVec2 p = ImGui::GetCursorScreenPos();
            const float  w = ImGui::GetContentRegionAvail().x;
            if (ImGui::InvisibleButton("##card", ImVec2(w, 62))) perfSel = i;
            ImDrawList* dl = ImGui::GetWindowDrawList();
            if (perfSel == i)              dl->AddRectFilled(p, ImVec2(p.x + w, p.y + 62), os.Accent(0.25f), 4.0f);
            else if (ImGui::IsItemHovered()) dl->AddRectFilled(p, ImVec2(p.x + w, p.y + 62), IM_COL32(255, 255, 255, 14), 4.0f);
            DrawGraph(dl, ImVec2(p.x + 6, p.y + 8), ImVec2(70, 46), res[i].hist, histPos, res[i].maxV, res[i].col, false);
            char value[48];
            const float v = Latest(res[i].hist);
            if (i == 1)      std::snprintf(value, sizeof(value), "%.1f/16.0 GB (%.0f%%)", totalMemMB / 1024.0f, v);
            else if (i == 3) std::snprintf(value, sizeof(value), "S: %.0f R: %.0f Kbps", v * 0.3f, v * 0.7f);
            else             std::snprintf(value, sizeof(value), "%.0f%%", v);
            dl->AddText(ImVec2(p.x + 86, p.y + 12), IM_COL32(235, 238, 245, 255), res[i].name);
            dl->AddText(ImVec2(p.x + 86, p.y + 32), IM_COL32(160, 168, 186, 255), value);
            ImGui::PopID();
        }
        ImGui::EndChild();
        ImGui::SameLine();

        ImGui::BeginChild("##perf_detail", ImVec2(0, 0), ImGuiChildFlags_Borders);
        static const char* const kModel[] = {"CSOPESY Virtual CPU @ 3.20GHz", "16.0 GB DDR5",
                                             "CSOPESY-NVMe 512 GB (SSD)", "CSOPESY Virtual Ethernet"};
        const Res& r = res[perfSel];
        ImGui::PushFont(os.fonts.uiLarge);
        ImGui::TextUnformatted(r.name);
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);
        TextRightAligned(kModel[perfSel]);

        ImGui::TextDisabled(perfSel == 3 ? "Throughput" : perfSel == 1 ? "Memory usage" : "%% Utilization");
        ImGui::SameLine();
        char top[32];
        std::snprintf(top, sizeof(top), perfSel == 3 ? "%.0f Kbps" : "100%%", r.maxV);
        TextRightAligned(top);

        const ImVec2 gp = ImGui::GetCursorScreenPos();
        const ImVec2 gs(ImGui::GetContentRegionAvail().x, ImMax(120.0f, ImGui::GetContentRegionAvail().y - 150.0f));
        DrawGraph(ImGui::GetWindowDrawList(), gp, gs, r.hist, histPos, r.maxV, r.col, true);
        ImGui::Dummy(gs);
        ImGui::TextDisabled("60 seconds");
        ImGui::SameLine();
        TextRightAligned("0");
        ImGui::Spacing();

        // Stats: big values on the left, static specs on the right.
        const int    procs = (int)bg.size() + [&] { int n = 0; for (auto& a : os.apps) n += a->isOpen; return n; }();
        const double up    = os.now - os.desktopStart;
        const float  v     = Latest(r.hist);
        char a[48], b[48], c[48], d[48];
        const char*  la = "", *lb = "", *lc = "", *ld = "";
        switch (perfSel) {
        case 0:
            la = "Utilization"; std::snprintf(a, sizeof(a), "%.0f%%", v);
            lb = "Speed";       std::snprintf(b, sizeof(b), "%.2f GHz", 2.40f + v / 100.0f * 1.4f);
            lc = "Processes";   std::snprintf(c, sizeof(c), "%d", procs);
            ld = "Up time";     std::snprintf(d, sizeof(d), "%s", Uptime(up).c_str());
            break;
        case 1:
            la = "In use";      std::snprintf(a, sizeof(a), "%.1f GB", totalMemMB / 1024.0f);
            lb = "Available";   std::snprintf(b, sizeof(b), "%.1f GB", (kRamMB - totalMemMB) / 1024.0f);
            lc = "Committed";   std::snprintf(c, sizeof(c), "%.1f/24.0 GB", totalMemMB / 1024.0f * 1.35f);
            ld = "Cached";      std::snprintf(d, sizeof(d), "%.1f GB", 2.4f + totalMemMB / 4096.0f);
            break;
        case 2:
            la = "Active time"; std::snprintf(a, sizeof(a), "%.0f%%", v);
            lb = "Avg. response"; std::snprintf(b, sizeof(b), "%.1f ms", 0.2f + v * 0.03f);
            lc = "Read speed";  std::snprintf(c, sizeof(c), "%.1f MB/s", totalDisk * 0.6f);
            ld = "Write speed"; std::snprintf(d, sizeof(d), "%.1f MB/s", totalDisk * 0.4f);
            break;
        default:
            la = "Send";        std::snprintf(a, sizeof(a), "%.0f Kbps", v * 0.3f);
            lb = "Receive";     std::snprintf(b, sizeof(b), "%.0f Kbps", v * 0.7f);
            lc = "Packets";     std::snprintf(c, sizeof(c), "%d/s", (int)(v * 1.8f));
            ld = "Up time";     std::snprintf(d, sizeof(d), "%s", Uptime(up).c_str());
            break;
        }
        static const char* const kSpecs[4][4] = {
            {"Base speed:  3.20 GHz", "Cores:  8", "Logical processors:  16", "Virtualization:  Enabled"},
            {"Speed:  4800 MT/s", "Slots used:  2 of 2", "Form factor:  SODIMM", "Hardware reserved:  210 MB"},
            {"Capacity:  512 GB", "Formatted:  476 GB", "System disk:  Yes", "Type:  SSD (NVMe)"},
            {"Adapter:  CSOPESY-LAN", "Connection:  Ethernet", "IPv4:  192.168.5.10", "Link speed:  1.0 Gbps"},
        };
        if (ImGui::BeginTable("##stats", 3, ImGuiTableFlags_SizingStretchSame)) {
            const char* labels[] = {la, lb, lc, ld};
            const char* values[] = {a, b, c, d};
            for (int row = 0; row < 2; ++row) {
                for (int col = 0; col < 2; ++col) {
                    ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s", labels[row * 2 + col]);
                    ImGui::PushFont(os.fonts.uiLarge);
                    ImGui::TextUnformatted(values[row * 2 + col]);
                    ImGui::PopFont();
                }
                ImGui::TableNextColumn();
                ImGui::TextDisabled("%s", kSpecs[perfSel][row * 2]);
                ImGui::TextDisabled("%s", kSpecs[perfSel][row * 2 + 1]);
            }
            ImGui::EndTable();
        }
        ImGui::EndChild();
    }
};

}  // namespace

std::unique_ptr<App> MakeTaskManager() { return std::make_unique<TaskManagerApp>(); }
