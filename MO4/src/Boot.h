// CSOPESY Desktop OS Emulator - Group 5
// Boot.h - full-screen sequences shown before and after the desktop.
#pragma once

#include "OS.h"

constexpr float kBiosDuration     = 4.6f;
constexpr float kSplashDuration   = 3.6f;
constexpr float kShutdownDuration = 2.6f;

void DrawBios(OSState& os, float t);                   // POST / memory test screen
void DrawSplash(OSState& os, float t);                 // logo + loading bar
void DrawPowerOff(OSState& os, float t, bool restart); // fade to black with message
