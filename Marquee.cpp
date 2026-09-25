// CSOPESY - Main Menu Console
// Group - 5
// Compile: g++ -std=c++17 -Wall -o csopesy main.cpp

#include <iostream>
#include <string>

// Removes leading and trailing whitespace (spaces, tabs, CR, LF).
static std::string trim(const std::string& s) {
    const std::string ws = " \t\r\n";
    const std::size_t start = s.find_first_not_of(ws);
    if (start == std::string::npos) return "";
    const std::size_t end = s.find_last_not_of(ws);
    return s.substr(start, end - start + 1);
}

// ---------- Console output ----------

static void printHeader() {
    std::cout << "Welcome to CSOPESY!\n\n"
              << "Group developer:\n"
              << "Alvarez, James Edsel\n"
              << "Quijano, Myrvin\n"
              << "Sanidad, Christian\n"
              << "Viray, Jarick\n\n"
              << "Version date: 2026-09-18\n";
}

static void printHelp() {
    std::cout << "help - displays the commands and its description\n"
              << "start_marquee - starts the marquee \"animation\"\n"
              << "stop_marquee - stops the marquee \"animation\"\n"
              << "set_text - accepts a text input and displays it as a marquee\n"
              << "set_speed - sets the marquee animation refresh in milliseconds\n"
              << "exit - terminates the console\n";
}



int main() {
    std::string marqueeText;
    bool running = true;

    printHeader();

    while (running) {
        std::cout << "\nCommand> " << std::flush;

        std::string line;
        if (!std::getline(std::cin, line)) {      
            std::cout << "\nTerminating console...\n";
            break;
        }

        line = trim(line);
        if (line.empty()) continue;              

        // Split into command (first word) and arguments (rest of the line).
        const std::size_t spacePos = line.find_first_of(" \t");
        const std::string command = line.substr(0, spacePos);
        const std::string args =
            (spacePos == std::string::npos) ? "" : trim(line.substr(spacePos + 1));

        if (command == "help") {
            printHelp();
        }
        else if (command == "set_text") {
            if (args.empty()) {
                std::cout << "Error: no text provided. Usage: set_text <your_string>\n";
            } else {
                marqueeText = args;
                std::cout << "Text saved for marquee: " << marqueeText << "\n";
            }
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

    return 0;
}
