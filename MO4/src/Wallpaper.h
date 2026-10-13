// CSOPESY Desktop OS Emulator - Group 5
// Wallpaper.h - procedural wallpapers drawn with ImGui draw commands.
#pragma once

#include "imgui.h"
#include "imgui_internal.h"   // ImMin / ImMax helpers

constexpr int kWallpaperCount = 4;
extern const char* const kWallpaperNames[kWallpaperCount];

// Fills the rectangle [origin, origin + size] with wallpaper `mode`.
// `t` is the animation time in seconds (pass a constant to freeze it).
// The same function draws the full-screen desktop and the small previews in Settings.
void DrawWallpaper(ImDrawList* dl, ImVec2 origin, ImVec2 size, int mode, float t);
