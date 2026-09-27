CSOPESY - Marquee Operator (Semi-Major Output 1)
Group 5
Version date: 2026-09-27

MEMBERS
  Alvarez, James Edsel
  Quijano, Myrvin
  Sanidad, Christian
  Viray, Jarick

ENTRY FILE
  Marquee.cpp  (contains int main())

REQUIREMENTS
  g++ with C++17 support (MinGW-w64 on Windows, or GCC on Linux/Mac)

HOW TO COMPILE
  g++ -std=c++17 -Wall -pthread -o csopesy Marquee.cpp

  In VS Code: Terminal > Run Build Task (task "build csopesy")

HOW TO RUN
  Windows:    .\csopesy.exe
  Linux/Mac:  ./csopesy

  In VS Code: press Run > Start Debugging (F5), config "Run Marquee".
  It builds and opens the program in an external console.

  Make the console window at least 30 rows tall and 100 columns wide
  so the whole screen fits.

COMMANDS
  help           displays the commands and their descriptions
  start_marquee  starts the marquee animation
  stop_marquee   stops the marquee animation
  set_text <t>   sets the marquee text (quotes are optional)
  set_speed <ms> sets the marquee refresh interval in milliseconds (1-100000)
  set_poll <ms>  sets the keyboard polling interval in milliseconds (1-1000)
  exit           terminates the console

  Defaults: refresh 200 ms, polling 10 ms. Both can be changed while the
  program runs, so no recompiling is needed during the quiz.
