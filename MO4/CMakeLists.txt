cmake_minimum_required(VERSION 3.16)
project(CSOPESY_Desktop LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ---- GLFW (bundled source in external/glfw) ----
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND  OFF CACHE BOOL "" FORCE)   # Linux: X11 only (works under XWayland too)
add_subdirectory(external/glfw)

find_package(OpenGL REQUIRED)

# ---- Dear ImGui (bundled source in external/imgui) ----
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

# ---- The emulator (entry point: src/main.cpp) ----
add_executable(csopesy_desktop
    src/main.cpp
    src/OS.cpp
    src/App.cpp
    src/Compositor.cpp
    src/Boot.cpp
    src/Wallpaper.cpp
    src/Taskbar.cpp
    src/Icons.cpp
    src/apps/FileExplorer.cpp
    src/apps/Terminal.cpp
    src/apps/TaskManager.cpp
    src/apps/Settings.cpp)
target_include_directories(csopesy_desktop PRIVATE src)
target_link_libraries(csopesy_desktop PRIVATE imgui)

if(MSVC)
    target_compile_options(csopesy_desktop PRIVATE /W4 /utf-8)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT csopesy_desktop)
else()
    target_compile_options(csopesy_desktop PRIVATE -Wall -Wextra)
endif()
if(MINGW)
    target_link_options(csopesy_desktop PRIVATE -static)   # no MinGW DLLs needed next to the .exe
endif()
