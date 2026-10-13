// CSOPESY Desktop OS Emulator - Group 5
// Terminal.cpp - the MO3 marquee console, re-implemented as a desktop window.
//
// MO3 needed a separate marquee thread because the console blocked on input.
// Here the compositor already redraws every frame, so the marquee simply
// advances by one character every `speedMs` milliseconds of accumulated time.
#include "Apps.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace {

const ImU32 kOut   = IM_COL32(210, 214, 222, 255);
const ImU32 kEcho  = IM_COL32(120, 200, 255, 255);
const ImU32 kError = IM_COL32(255, 110, 110, 255);
const ImU32 kOk    = IM_COL32(120, 230, 140, 255);
const ImU32 kDim   = IM_COL32(140, 148, 166, 255);

constexpr int kMinSpeed = 10;
constexpr int kMaxSpeed = 2000;

std::string Trim(const std::string& s) {
    const char* ws = " \t\r\n";
    const auto  a  = s.find_first_not_of(ws);
    if (a == std::string::npos) return "";
    return s.substr(a, s.find_last_not_of(ws) - a + 1);
}

class TerminalApp : public App {
public:
    TerminalApp() : App("Terminal", "terminal.exe", IconType::Terminal, ImVec2(720, 470), 0.6f, 12.4f) {}

protected:
    void OnOpen(OSState&) override {
        lines.clear();
        Print("CSOPESY Terminal [Version 1.0]", kOut);
        Print("Marquee Operator (MO3) running inside the desktop. Type 'help' for commands.", kDim);
        Print("", kOut);
        refocus = true;
    }

    void DrawContent(OSState& os) override {
        AdvanceMarquee(os.dt);
        DrawMarquee(os);

        const float inputH = ImGui::GetFrameHeightWithSpacing();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(12, 14, 20, 255));
        ImGui::PushFont(os.fonts.mono);
        ImGui::BeginChild("##log", ImVec2(0, -inputH), ImGuiChildFlags_Borders);
        for (const Line& l : lines) {
            ImGui::PushStyleColor(ImGuiCol_Text, l.col);
            ImGui::TextWrapped("%s", l.text.c_str());
            ImGui::PopStyleColor();
        }
        if (scrollToBottom) ImGui::SetScrollHereY(1.0f);
        scrollToBottom = false;
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::AlignTextToFramePadding();
        ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(kEcho), "Command>");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1);
        if (refocus) ImGui::SetKeyboardFocusHere();
        refocus = false;
        // Enter keeps the box active (no lost keystrokes between commands);
        // the buffer is cleared and ImGui is told to reload it.
        ImGuiIO&   io         = ImGui::GetIO();
        const bool keepActive = io.ConfigInputTextEnterKeepActive;
        io.ConfigInputTextEnterKeepActive = true;
        const ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory;
        const bool entered = ImGui::InputText("##cmd", input, sizeof(input), flags, &TerminalApp::HistoryCallback, this);
        io.ConfigInputTextEnterKeepActive = keepActive;
        if (entered) {
            const std::string line = Trim(input);
            input[0] = '\0';
            if (ImGuiInputTextState* st = ImGui::GetInputTextState(ImGui::GetItemID())) st->ReloadUserBufAndMoveToEnd();
            if (!line.empty()) Execute(os, line);
        }
        ImGui::PopFont();
    }

private:
    struct Line { std::string text; ImU32 col; };

    std::vector<Line>        lines;
    std::vector<std::string> history;
    int  historyPos = -1;
    char input[256] = "";
    bool scrollToBottom = false;
    bool refocus = true;

    // Marquee state
    std::string text    = "Welcome to CSOPESY OS!  Group 5: Alvarez | Sanidad | Viray | Quijano";
    bool        running = true;
    int         speedMs = 120;
    double      accMs   = 0.0;
    size_t      offset  = 0;
    size_t      visible = 40;

    void Print(const std::string& s, ImU32 col) {
        lines.push_back({s, col});
        if (lines.size() > 500) lines.erase(lines.begin());
        scrollToBottom = true;
    }

    void AdvanceMarquee(float dt) {
        if (!running) return;
        accMs += dt * 1000.0;
        int steps = 0;
        while (accMs >= speedMs && steps++ < 64) {
            accMs -= speedMs;
            ++offset;
        }
        if (steps >= 64) accMs = 0.0;   // window was minimized / frame hitch: don't fast-forward
    }

    void DrawMarquee(OSState& os) {
        ImFont*     f  = os.fonts.monoLarge;
        const float cw = f->CalcTextSizeA(f->FontSize, FLT_MAX, 0.0f, "M").x;
        const float h  = f->FontSize + 18.0f;

        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 255));
        ImGui::PushStyleColor(ImGuiCol_Border, os.Accent(0.8f));
        ImGui::BeginChild("##marquee", ImVec2(0, h), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar);
        const ImVec2 p = ImGui::GetCursorScreenPos();
        visible = (size_t)ImMax(1.0f, (ImGui::GetContentRegionAvail().x) / cw);

        // The text enters from the right edge, crosses the band, leaves on the left, repeats.
        const std::string track = std::string(visible, ' ') + text;
        std::string frame(visible, ' ');
        for (size_t i = 0; i < visible; ++i) frame[i] = track[(offset + i) % track.size()];
        ImGui::GetWindowDrawList()->AddText(f, f->FontSize, ImVec2(p.x, p.y + 1.0f), IM_COL32(90, 240, 120, 255), frame.c_str());
        ImGui::EndChild();
        ImGui::PopStyleColor(2);

        ImGui::TextDisabled("Marquee: %s  |  Speed: %d ms  |  Text: %zu chars", running ? "RUNNING" : "STOPPED", speedMs, text.size());
    }

    void Execute(OSState& os, const std::string& line) {
        history.push_back(line);
        historyPos = -1;
        Print("Command> " + line, kEcho);

        const size_t      sp   = line.find_first_of(" \t");
        std::string       cmd  = line.substr(0, sp);
        const std::string args = sp == std::string::npos ? "" : Trim(line.substr(sp + 1));
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), [](unsigned char c) { return (char)std::tolower(c); });

        if (cmd == "help") {
            Print("help              - displays the commands and its description", kOut);
            Print("start_marquee     - starts the marquee \"animation\"", kOut);
            Print("stop_marquee      - stops the marquee \"animation\"", kOut);
            Print("set_text <text>   - accepts a text input and displays it as a marquee", kOut);
            Print("set_speed <ms>    - sets the marquee animation refresh in milliseconds", kOut);
            Print("clear             - clears the terminal output", kOut);
            Print("date              - shows the current date and time", kOut);
            Print("ver               - shows the OS version", kOut);
            Print("exit              - closes the terminal window", kOut);
        } else if (cmd == "start_marquee") {
            if (running) Print("Marquee is already running.", kDim);
            else { running = true; accMs = 0.0; Print("Marquee started.", kOk); }
        } else if (cmd == "stop_marquee") {
            if (!running) Print("Marquee is already stopped.", kDim);
            else { running = false; Print("Marquee stopped.", kOk); }
        } else if (cmd == "set_text") {
            std::string t = args;
            if (t.size() >= 2 && t.front() == '"' && t.back() == '"') t = t.substr(1, t.size() - 2);
            if (t.empty()) Print("Error: no text provided. Usage: set_text <your_string>", kError);
            else { text = t; offset = 0; Print("Marquee text set to: " + text, kOk); }
        } else if (cmd == "set_speed") {
            bool numeric = !args.empty() && args.find_first_not_of("0123456789") == std::string::npos && args.size() <= 6;
            const int ms = numeric ? std::stoi(args) : -1;
            if (!numeric) Print("Error: invalid speed. Usage: set_speed <milliseconds>", kError);
            else if (ms < kMinSpeed || ms > kMaxSpeed)
                Print("Error: speed must be between " + std::to_string(kMinSpeed) + " and " + std::to_string(kMaxSpeed) + " ms.", kError);
            else { speedMs = ms; Print("Marquee refresh set to " + std::to_string(ms) + " ms.", kOk); }
        } else if (cmd == "clear" || cmd == "cls") {
            lines.clear();
        } else if (cmd == "date") {
            Print(FormatLocalTime("%A, %B %d, %Y  ") + ClockString(os.settings), kOut);
        } else if (cmd == "ver") {
            Print("CSOPESY OS v1.0 (Semi-Major Output 2) - Group 5", kOut);
        } else if (cmd == "exit") {
            Close();
        } else {
            Print("Error: '" + cmd + "' is not a recognized command. Type 'help' to see the list of commands.", kError);
        }
    }

    static int HistoryCallback(ImGuiInputTextCallbackData* data) {
        auto* self = static_cast<TerminalApp*>(data->UserData);
        if (self->history.empty()) return 0;
        const int last = (int)self->history.size() - 1;
        if (data->EventKey == ImGuiKey_UpArrow)
            self->historyPos = self->historyPos < 0 ? last : ImMax(0, self->historyPos - 1);
        else if (data->EventKey == ImGuiKey_DownArrow && self->historyPos >= 0)
            self->historyPos = self->historyPos >= last ? -1 : self->historyPos + 1;
        data->DeleteChars(0, data->BufTextLen);
        if (self->historyPos >= 0) data->InsertChars(0, self->history[self->historyPos].c_str());
        return 0;
    }
};

}  // namespace

std::unique_ptr<App> MakeTerminal() { return std::make_unique<TerminalApp>(); }
