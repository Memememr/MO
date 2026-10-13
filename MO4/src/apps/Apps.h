// CSOPESY Desktop OS Emulator - Group 5
// Apps.h - factory functions for the installed applications.
#pragma once

#include "../App.h"

#include <memory>

std::unique_ptr<App> MakeFileExplorer();   // unique screen #1: placeholder file system browser
std::unique_ptr<App> MakeTerminal();       // unique screen #2: the MO3 marquee console, now as a window
std::unique_ptr<App> MakeTaskManager();    // required Task Manager
std::unique_ptr<App> MakeSettings();       // wallpaper, accent colour, clock, about
