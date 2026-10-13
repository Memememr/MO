// CSOPESY Desktop OS Emulator - Group 5
// App.h - base class for every windowed application.
//
// An App is the emulator's notion of a "process": it has a PID, a state
// (closed / running / minimized), dummy resource usage shown in the Task
// Manager, and a window the compositor draws every frame while it runs.
#pragma once

#include "OS.h"

struct ProcStats {
    float cpu  = 0.0f;   // %
    float mem  = 0.0f;   // MB
    float disk = 0.0f;   // MB/s
    float net  = 0.0f;   // Mbps
};

class App {
public:
    App(std::string title, std::string exe, IconType icon, ImVec2 defaultSize,
        float baseCpu, float baseMem);
    virtual ~App() = default;

    void Open(OSState& os);   // launch (or bring to front if already running)
    void Close();             // terminate
    void Minimize();
    void Restore();           // un-minimize and focus
    void Draw(OSState& os);   // called by the compositor once per frame

    std::string title;        // window title / display name
    std::string exe;          // "image name" shown in the Task Manager
    IconType    icon;
    ImVec2      defaultSize;
    ImVec2      minSize = ImVec2(360, 240);
    int         slot    = 0;  // used to cascade first-time window positions

    bool   isOpen      = false;
    bool   isMinimized = false;
    bool   isFocused   = false;
    int    pid         = 0;
    double startedAt   = 0.0;

    ProcStats stats;          // dummy values, refreshed by the Task Manager
    float     baseCpu;
    float     baseMem;

protected:
    virtual void DrawContent(OSState& os) = 0;
    virtual void OnOpen(OSState&) {}
    virtual void OnClose() {}

private:
    bool focusRequested = false;
    bool DrawMinimizeButton();
};
