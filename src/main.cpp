#include "ConsoleUI.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
#ifdef _WIN32
    // Set console title
    SetConsoleTitleA("OOP & CG Mini-Project - Banking System");
#endif

    try {
        ConsoleUI app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "\n[FATAL ERROR] An unexpected exception occurred: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "\n[FATAL ERROR] An unknown critical error occurred.\n";
        return 1;
    }

    return 0;
}
