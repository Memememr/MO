// CSOPESY - Marquee Operator
// Group 5
// Compile: g++ -std=c++17 -Wall -pthread -o csopesy Marquee.cpp
// Run Linux/Mac: ./csopesy
// Run Windows: .\csopesy.exe

#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

// ---------- Shared state ----------
static std::mutex g_textMutex;                 // protects marqueeText
static std::string marqueeText = "Hello CSOPESY";
static std::atomic<int>  speedMs{200};         // default 200 ms
static std::atomic<bool> marqueeRunning{false};
static std::atomic<bool> programDone{false};

// ---------- Helpers ----------
static std::string trim(const std::string& s) {
    const std::string ws = " \t\r\n";
    std::size_t start = s.find_first_not_of(ws);
    if (start == std::string::npos) return "";
    std::size_t end = s.find_last_not_of(ws);
    return s.substr(start, end - start + 1);
}

static bool toInt(const std::string& s, int& out) {
    try {
        std::size_t pos = 0;
        int v = std::stoi(s, &pos);
        if (pos != s.size()) return false;
        out = v;
        return true;
    } catch (...) {
        return false;
    }
}

// ---------- Marquee thread ----------
// Scrolls the text on one line using \r (carriage return).
static void marqueeThread() {
    const int WINDOW = 30;     // how many characters are visible at once
    std::size_t offset = 0;

    while (!programDone.load()) {
        if (!marqueeRunning.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        std::string text;
        {
            std::lock_guard<std::mutex> lock(g_textMutex);
            text = marqueeText;
        }

        if (!text.empty()) {
            // Add a gap so the text wraps nicely
            std::string padded = text + "   ";

            // Build the visible window
            std::string frame;
            for (int i = 0; i < WINDOW; ++i) {
                frame += padded[(offset + i) % padded.size()];
            }

            // \r moves cursor to start of line, overwriting previous frame
            std::cout << "\r[Marquee] " << frame << std::flush;

            offset = (offset + 1) % padded.size();
        }

        int s = speedMs.load();
        if (s < 10) s = 10;   // avoid burning CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(s));
    }
}

// ---------- Console output ----------
static void printHeader() {
    std::cout << "Welcome to CSOPESY!\n\n"
              << "Group developer:\n"
              << "Alvarez, James Edsel\n"
              << "Quijano, Myrvin\n"
              << "Sanidad, Christian\n"
              << "Viray, Jarick\n\n"
              << "Version date: 2026-09-26\n";
}

static void printHelp() {
    std::cout << "help          - displays the commands and its description\n"
              << "start_marquee - starts the marquee animation\n"
              << "stop_marquee  - stops the marquee animation\n"
              << "set_text      - accepts a text input and displays it as a marquee\n"
              << "set_speed     - sets the marquee animation refresh in milliseconds\n"
              << "exit          - terminates the console\n";
}

// ---------- Main ----------
int main() {
    printHeader();

    // Start the background marquee thread right away.
    // It will idle until start_marquee is issued.
    std::thread worker(marqueeThread);

    bool running = true;
    while (running) {
        std::cout << "\nCommand> " << std::flush;

        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout << "\nTerminating console...\n";
            break;
        }

        line = trim(line);
        if (line.empty()) continue;

        // Split "command arg1 arg2..." into command + rest
        std::size_t spacePos = line.find_first_of(" \t");
        std::string command = line.substr(0, spacePos);
        std::string args = (spacePos == std::string::npos)
                            ? ""
                            : trim(line.substr(spacePos + 1));

        if (command == "help") {
            printHelp();
        }
        else if (command == "set_text") {
            if (args.empty()) {
                std::cout << "Error: no text provided. Usage: set_text <your_string>\n";
            } else {
                {
                    std::lock_guard<std::mutex> lock(g_textMutex);
                    marqueeText = args;
                }
                std::cout << "Text saved for marquee: " << args << "\n";
            }
        }
        else if (command == "set_speed") {
            int ms;
            if (args.empty() || !toInt(args, ms) || ms <= 0) {
                std::cout << "Error: invalid speed. Usage: set_speed <milliseconds>\n";
            } else {
                speedMs.store(ms);
                std::cout << "Marquee refresh set to " << ms << " ms.\n";
            }
        }
        else if (command == "start_marquee") {
            marqueeRunning.store(true);
            std::cout << "Marquee started.\n";
        }
        else if (command == "stop_marquee") {
            marqueeRunning.store(false);
            std::cout << "Marquee stopped.\n";
        }
        else if (command == "exit") {
            std::cout << "Terminating console...\n";
            running = false;
        }
        else {
            std::cout << "Error: '" << command << "' is not a recognized command. "
                      << "Type 'help' to see the list of commands.\n";
        }
    }

    // Tell the worker to stop and wait for it to finish.
    programDone.store(true);
    marqueeRunning.store(false);
    if (worker.joinable()) worker.join();

    return 0;
}