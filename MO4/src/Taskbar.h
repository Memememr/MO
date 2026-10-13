// CSOPESY Desktop OS Emulator - Group 5
// Taskbar.h - fixed panel at the bottom of the screen.
#pragma once

#include "OS.h"

// Start button + start menu, one button per application (with running
// indicators), and the system tray (VOL, NET, clock/calendar, PWR).
void DrawTaskbar(OSState& os);
