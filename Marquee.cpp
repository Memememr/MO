// CSOPESY - Marquee Operator
// Group 5
// Compile: g++ -std=c++17 -Wall -pthread -o csopesy Marquee.cpp
// Run Linux/Mac: ./csopesy
// Run Windows: .\csopesy.exe

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <string>
#include <thread>

#ifdef _WIN32
  #include <windows.h>
  #include <conio.h>
  #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
    #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
  #endif
#else
  #include <sys/select.h>
  #include <termios.h>
  #include <unistd.h>
#endif

// Terminal: non-blocking keyboard + ANSI escape support

namespace term {
#ifdef _WIN32
    void init() {
        HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        if (GetConsoleMode(out, &mode))
            SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
    void restore() {}
    bool keyAvailable() { return _kbhit() != 0; }
    int  readKey() {
        int c = _getch();
        if (c == 0 || c == 224) { _getch(); return -1; }  // arrow/function key: ignore
        return c;
    }
#else
    static termios original;
    static bool saved = false;

    void init() {
        if (tcgetattr(STDIN_FILENO, &original) == 0) {
            saved = true;
            termios raw = original;
            raw.c_lflag &= ~(ICANON | ECHO);   // read key by key, we echo ourselves
            raw.c_cc[VMIN] = 0;
            raw.c_cc[VTIME] = 0;
            tcsetattr(STDIN_FILENO, TCSANOW, &raw);
        }
    }
    void restore() {
        if (saved) tcsetattr(STDIN_FILENO, TCSANOW, &original);
    }
    bool keyAvailable() {
        fd_set set;
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);
        timeval tv{0, 0};
        return select(STDIN_FILENO + 1, &set, nullptr, nullptr, &tv) > 0;
    }
    int readKey() {   // returns -2 on end of input
        unsigned char c;
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n == 0) return -2;
        if (n < 0) return -1;
        if (c == 27) {                        // escape sequence (arrow keys)
            while (keyAvailable() && read(STDIN_FILENO, &c, 1) == 1) {
                if ((c >= 'A' && c <= 'Z') || c == '~') break;
            }
            return -1;
        }
        return c;
    }
#endif
}


// Shared state

static std::mutex        g_textMutex;                  // protects g_marqueeText
static std::string       g_marqueeText = "Hello CSOPESY";
static std::atomic<int>  g_refreshMs{200};             // marquee refresh (set_speed)
static std::atomic<int>  g_pollMs{10};                 // keyboard polling (set_poll)
static std::atomic<bool> g_marqueeRunning{false};
static std::atomic<bool> g_programDone{false};

// Wakes the marquee thread early (start, stop, speed change, exit) so a long
// refresh interval never makes the program feel frozen.
static std::mutex              g_wakeMutex;
static std::condition_variable g_wakeCv;
static unsigned                g_wakeGen = 0;

static void wakeMarquee() {
    { std::lock_guard<std::mutex> lk(g_wakeMutex); ++g_wakeGen; }
    g_wakeCv.notify_all();
}


// Screen (every write to the console goes through g_screen)

static std::mutex g_screen;

const int  ROW_MARQUEE = 15;
const int  ROW_STATUS  = 16;
const int  ROW_LOG     = 18;
const int  LOG_LINES   = 9;
const int  ROW_PROMPT  = ROW_LOG + LOG_LINES + 1;      // 28
const std::size_t MAX_WIDTH = 100;                     // keep lines from wrapping
const int  MARQUEE_WINDOW = 40;                        // visible marquee characters
const std::string PROMPT = "Command> ";

static std::deque<std::string> g_log;
static std::string g_input;
static std::string g_frame = "[Marquee] (idle - type start_marquee)";

static const char* BANNER = R"BANNER(  ____ ____   ___  ____  _____ ______   __
 / ___/ ___| / _ \|  _ \| ____/ ___\ \ / /
| |   \___ \| | | | |_) |  _| \___ \\ V /
| |___ ___) | |_| |  __/| |___ ___) || |
 \____|____/ \___/|_|   |_____|____/ |_|)BANNER";

// --- helpers below assume the caller holds g_screen ---
static void clearRow(int row) { std::cout << "\x1b[" << row << ";1H\x1b[2K"; }

static void placeCursor() {
    std::cout << "\x1b[" << ROW_PROMPT << ";" << (PROMPT.size() + g_input.size() + 1) << "H";
}

static void drawMarqueeLocked() {
    clearRow(ROW_MARQUEE);
    std::cout << g_frame;
    placeCursor();
    std::cout << std::flush;
}

static void drawStatusLocked() {
    clearRow(ROW_STATUS);
    std::cout << "Refresh: " << g_refreshMs.load() << " ms | Poll: " << g_pollMs.load()
              << " ms | Marquee: " << (g_marqueeRunning.load() ? "running" : "stopped");
}

static void drawLogLocked() {
    for (int i = 0; i < LOG_LINES; ++i) {
        clearRow(ROW_LOG + i);
        if (i < static_cast<int>(g_log.size())) std::cout << g_log[i];
    }
}

static void drawPromptLocked() {
    clearRow(ROW_PROMPT);
    std::cout << PROMPT << g_input;
    placeCursor();
    std::cout << std::flush;
}

static void drawAllLocked() {
    std::cout << "\x1b[2J\x1b[H" << BANNER << "\n\n"
              << "Group developer:\n"
              << "Alvarez, James Edsel\n"
              << "Quijano, Myrvin\n"
              << "Sanidad, Christian\n"
              << "Viray, Jarick\n\n"
              << "Version date: 2026-09-27\n";
    drawStatusLocked();
    drawLogLocked();
    drawMarqueeLocked();
    drawPromptLocked();
}

static void logLine(const std::string& s) {
    g_log.push_back(s.size() > MAX_WIDTH ? s.substr(0, MAX_WIDTH) : s);
    while (static_cast<int>(g_log.size()) > LOG_LINES) g_log.pop_front();
}


// Marquee thread: redraws only the marquee row

static void marqueeThread() {
    std::size_t offset = 0;
    while (!g_programDone.load()) {
        if (g_marqueeRunning.load()) {
            std::string text;
            {
                std::lock_guard<std::mutex> lk(g_textMutex);
                text = g_marqueeText;
            }
            std::string padded = text + "     ";        // gap before the text repeats
            std::string window;
            for (int i = 0; i < MARQUEE_WINDOW; ++i)
                window += padded[(offset + i) % padded.size()];
            offset = (offset + 1) % padded.size();

            std::lock_guard<std::mutex> lk(g_screen);
            g_frame = "[Marquee] |" + window + "|";
            drawMarqueeLocked();
        }

        // Sleep for the refresh interval, but wake immediately on start/stop/speed/exit.
        std::unique_lock<std::mutex> lk(g_wakeMutex);
        unsigned gen = g_wakeGen;
        auto wait = g_marqueeRunning.load() ? std::chrono::milliseconds(g_refreshMs.load())
                                            : std::chrono::milliseconds(1000);
        g_wakeCv.wait_for(lk, wait, [&] { return g_programDone.load() || g_wakeGen != gen; });
    }
}

// Command interpreter
static std::string trim(const std::string& s) {
    const std::string ws = " \t\r\n";
    std::size_t start = s.find_first_not_of(ws);
    if (start == std::string::npos) return "";
    return s.substr(start, s.find_last_not_of(ws) - start + 1);
}

static bool toInt(const std::string& s, int& out) {
    try {
        std::size_t pos = 0;
        long v = std::stol(s, &pos);
        if (pos != s.size() || v < 1 || v > 100000) return false;
        out = static_cast<int>(v);
        return true;
    } catch (...) {
        return false;
    }
}

// Runs one command. Caller holds g_screen. Returns false on exit.
static bool execute(const std::string& rawLine) {
    std::string line = trim(rawLine);
    if (line.empty()) return true;
    logLine(PROMPT + line);

    std::size_t sp = line.find_first_of(" \t");
    std::string command = line.substr(0, sp);
    std::string args = (sp == std::string::npos) ? "" : trim(line.substr(sp + 1));

    if (command == "help") {
        logLine("help          - displays the commands and its description");
        logLine("start_marquee - starts the marquee animation");
        logLine("stop_marquee  - stops the marquee animation");
        logLine("set_text      - accepts a text input and displays it as a marquee");
        logLine("set_speed     - sets the marquee animation refresh in milliseconds");
        logLine("set_poll      - sets the keyboard polling rate in milliseconds");
        logLine("exit          - terminates the console");
    }
    else if (command == "start_marquee") {
        g_marqueeRunning.store(true);
        wakeMarquee();
        logLine("Marquee started.");
    }
    else if (command == "stop_marquee") {
        g_marqueeRunning.store(false);
        wakeMarquee();
        logLine("Marquee stopped.");
    }
    else if (command == "set_text") {
        if (args.size() >= 2 && args.front() == '"' && args.back() == '"')
            args = args.substr(1, args.size() - 2);
        if (args.empty()) {
            logLine("Error: no text provided. Usage: set_text <your text>");
        } else {
            { std::lock_guard<std::mutex> lk(g_textMutex); g_marqueeText = args; }
            logLine("Marquee text set to: " + args);
        }
    }
    else if (command == "set_speed") {
        int ms;
        if (!toInt(args, ms)) {
            logLine("Error: invalid speed. Usage: set_speed <1-100000 milliseconds>");
        } else {
            g_refreshMs.store(ms);
            wakeMarquee();
            logLine("Marquee refresh set to " + std::to_string(ms) + " ms.");
        }
    }
    else if (command == "set_poll") {
        int ms;
        if (!toInt(args, ms) || ms > 1000) {
            logLine("Error: invalid polling rate. Usage: set_poll <1-1000 milliseconds>");
        } else {
            g_pollMs.store(ms);
            logLine("Keyboard polling set to " + std::to_string(ms) + " ms.");
        }
    }
    else if (command == "exit") {
        return false;
    }
    else {
        logLine("Error: '" + command + "' is not a recognized command. Type 'help' to see the list of commands.");
    }

    drawStatusLocked();
    drawLogLocked();
    drawPromptLocked();
    return true;
}

// Main: keyboard polling loop
static void shutdownTerminal() {
    std::cout << "\x1b[" << (ROW_PROMPT + 1) << ";1H\nTerminating console...\n" << std::flush;
    term::restore();
}

int main() {
    term::init();
    std::signal(SIGINT, [](int) { term::restore(); std::_Exit(0); });

    { std::lock_guard<std::mutex> lk(g_screen); drawAllLocked(); }
    std::thread worker(marqueeThread);

    bool running = true;
    while (running) {
        // Handle every key that arrived since the last poll.
        while (running && term::keyAvailable()) {
            int c = term::readKey();
            std::lock_guard<std::mutex> lk(g_screen);

            if (c == -2 || c == 3) {                    // end of input or Ctrl+C
                running = false;
            } else if (c == '\r' || c == '\n') {
                std::string line = g_input;
                g_input.clear();
                running = execute(line);
                if (running) drawPromptLocked();
            } else if (c == 8 || c == 127) {            // backspace
                if (!g_input.empty()) { g_input.pop_back(); drawPromptLocked(); }
            } else if (c >= 32 && c <= 126) {
                if (PROMPT.size() + g_input.size() < MAX_WIDTH) {
                    g_input += static_cast<char>(c);
                    drawPromptLocked();
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(g_pollMs.load()));
    }

    g_programDone.store(true);
    g_marqueeRunning.store(false);
    wakeMarquee();
    if (worker.joinable()) worker.join();

    std::lock_guard<std::mutex> lk(g_screen);
    shutdownTerminal();
    return 0;
}