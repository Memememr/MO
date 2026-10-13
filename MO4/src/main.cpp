// CSOPESY Desktop OS Emulator - Group 5 (Alvarez, Sanidad, Viray, Quijano)
// Semi-Major Output 2: Desktop-Style OS Mock-up
//
// main.cpp - ENTRY POINT. Creates the GLFW window + OpenGL context, sets up
// Dear ImGui, installs the applications and runs the render loop.
//
// The loop ends only when the compositor finishes the shutdown sequence
// started from the PWR button. Closing the window (X / Alt+F4) is intercepted
// and opens the same Power Options dialog instead of force-quitting.
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

#include "App.h"
#include "Compositor.h"
#include "OS.h"
#include "apps/Apps.h"
#include "fonts/CousineRegular.h"
#include "fonts/RobotoMedium.h"

#include <cstdio>
#include <cstring>

namespace {

OSState* gOS = nullptr;

void OnGlfwError(int code, const char* desc) { std::fprintf(stderr, "GLFW error %d: %s\n", code, desc); }

// Window close button / Alt+F4: cancel the close and ask through the PWR dialog.
void OnCloseRequested(GLFWwindow* window) {
    glfwSetWindowShouldClose(window, GLFW_FALSE);
    if (gOS) gOS->requestPowerMenu = true;
}

void ToggleFullscreen(GLFWwindow* window, OSState& os) {
    static int x = 100, y = 100, w = 1280, h = 720;
    if (!os.isFullscreen) {
        glfwGetWindowPos(window, &x, &y);
        glfwGetWindowSize(window, &w, &h);
        GLFWmonitor*       mon  = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(mon);
        glfwSetWindowMonitor(window, mon, 0, 0, mode->width, mode->height, mode->refreshRate);
    } else {
        glfwSetWindowMonitor(window, nullptr, x, y, w, h, 0);
    }
    os.isFullscreen = !os.isFullscreen;
}

void LoadFonts(OSState& os) {
    ImGuiIO&     io = ImGui::GetIO();
    ImFontConfig cfg;
    cfg.OversampleH = 2;
    os.fonts.ui        = io.Fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, 17.0f, &cfg);
    os.fonts.uiLarge   = io.Fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, 24.0f, &cfg);
    os.fonts.uiHuge    = io.Fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, 44.0f, &cfg);
    os.fonts.mono      = io.Fonts->AddFontFromMemoryCompressedTTF(CousineRegular_compressed_data, CousineRegular_compressed_size, 16.0f, &cfg);
    os.fonts.monoLarge = io.Fonts->AddFontFromMemoryCompressedTTF(CousineRegular_compressed_data, CousineRegular_compressed_size, 22.0f, &cfg);
}

void InstallApps(OSState& os) {
    os.apps.push_back(MakeFileExplorer());
    os.apps.push_back(MakeTerminal());
    os.apps.push_back(MakeTaskManager());
    os.apps.push_back(MakeSettings());
    for (int i = 0; i < (int)os.apps.size(); ++i) os.apps[i]->slot = i;
}

}  // namespace

int main(int argc, char** argv) {
    bool skipBoot = false;
    for (int i = 1; i < argc; ++i)
        if (std::strcmp(argv[i], "--skip-boot") == 0) skipBoot = true;

    // ---- Window + OpenGL context ----
    glfwSetErrorCallback(OnGlfwError);
    if (!glfwInit()) return 1;
#if defined(__APPLE__)
    const char* glsl = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    const char* glsl = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif
    GLFWwindow* window = glfwCreateWindow(1280, 720, "CSOPESY Desktop OS Emulator", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);   // vsync
    glfwSetWindowSizeLimits(window, 960, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetWindowCloseCallback(window, OnCloseRequested);

    // ---- Dear ImGui ----
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;   // the OS boots fresh every run
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl);

    OSState os;
    gOS = &os;
    os.glRenderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    os.glVersion  = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    LoadFonts(os);
    ApplyTheme(os.settings.accent);
    InstallApps(os);
    os.now = glfwGetTime();
    Compositor compositor(os, skipBoot);

    // ---- Main loop: one iteration = one composited frame ----
    while (!os.poweredOff) {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }
        const double now = glfwGetTime();
        os.dt  = (float)(now - os.now);
        os.now = now;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        compositor.Frame();
        ImGui::Render();

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        if (os.toggleFullscreen) {
            os.toggleFullscreen = false;
            ToggleFullscreen(window, os);
        }
    }

    // ---- Clean shutdown ----
    os.apps.clear();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    std::printf("CSOPESY OS shut down normally.\n");
    return 0;
}
