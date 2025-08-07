#define NOMINMAX
#include "Common.h"
#include "PerformanceMonitor.h"
#include "System.h"
#include "Interactive.h"
#include "ArgParser.h"
#include "IO.h"

// Basic console setup
void setup_console() {
    SetConsoleTitle(L"Fluminum Matrix Operations");
    SetConsoleOutputCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
}

// Non-interactive mode handler (placeholder for cmd line logic)
void run_command_line_mode(const ArgParser& parser) {
    cout << "Command-line mode is not yet implemented." << endl;
    // Here you would parse options from the `parser` object
    // and call the appropriate functions from Algorithms.h
    // Example:
    // if (parser.optionExists("multiply")) { ... }
    // if (parser.optionExists("compare")) { ... }
}


int main(int argc, char* argv[]) {
    // 1. Check for monitor-only mode
    if (argc > 1 && strcmp(argv[1], "--monitor") == 0) {
        return RunPerformanceMonitorEntry();
    }

    // 2. Standard Program Initialization
    setup_console();
    LaunchMonitorProcess();
    initializePerformanceCounter();

    ArgParser parser(argc, argv);

    // 3. Decide execution mode
    // If more than one argument is passed (program name + something else),
    // assume command-line mode. Otherwise, interactive.
    if (argc > 1) {
        // NOTE: Full command-line operation logic would be extensive.
        // This structure enables it. For now, it will launch interactive mode.
        cout << YELLOW << "Command-line arguments detected. "
            << "For this version, launching interactive mode." << RESET << endl << endl;
        run_interactive_mode();
    }
    else {
        // Launch the standard interactive menu
        run_interactive_mode();
    }

    return 0;
}