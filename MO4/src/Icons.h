// CSOPESY Desktop OS Emulator - Group 5
// Icons.h - every icon is drawn with ImDrawList primitives (no image files).
#pragma once

#include "OS.h"

// Draws an icon centred on `center`, fitting inside a size x size square.
void DrawIcon(ImDrawList* dl, IconType type, ImVec2 center, float size, ImU32 accent);

// Draws an icon at the current cursor position and advances the layout.
void InlineIcon(IconType type, float size, ImU32 accent);
