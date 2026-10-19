#ifndef CONSOLE_UI_H
#define CONSOLE_UI_H

#include "Bank.h"
#include <string>

class ConsoleUI {
private:
    Bank bank;

    // Helper input methods
    std::string promptString(const std::string& prompt);
    double promptDouble(const std::string& prompt, double minVal = 0.0);
    int promptInt(const std::string& prompt, int minVal = 0, int maxVal = 100);
    std::string promptPin(const std::string& prompt = "Enter 4-digit PIN: ");
    void pressEnterToContinue();
    void clearScreen();

    // Menu Action Handlers (Matching a through g)
    void handleCreateAccount();
    void handleDepositMoney();
    void handleWithdrawMoney();
    void handleHonorDailyLimit();
    void handleCheckBalance();
    void handleDisplayAccountInfo();
    void handlePassbookPrint();
    void handleGraphicalAnalytics();
    void handleViewAllAccounts();

public:
    ConsoleUI();
    void run();
};

#endif // CONSOLE_UI_H
