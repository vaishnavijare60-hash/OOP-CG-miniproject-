#ifndef VISUALIZER_H
#define VISUALIZER_H

#include "Account.h"
#include <memory>

class Visualizer {
public:
    // Launches native Win32 Computer Graphics dashboard for the specified account
    static void renderAccountDashboard(const Account& account);

    // Check if graphical subsystem is supported
    static bool isSupported();
};

#endif // VISUALIZER_H
