#include "ConsoleUI.h"
#include "Visualizer.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <cstdlib>

ConsoleUI::ConsoleUI() : bank("APEX NATIONAL BANK", "data/bank_data.txt") {}

void ConsoleUI::clearScreen() {
#ifdef _WIN32
    std::system("cls");
#else
    std::cout << "\033[2J\033[1;1H";
#endif
}

void ConsoleUI::pressEnterToContinue() {
    if (std::cin.eof()) return;
    std::cout << "\nPress [ENTER] to return to main menu...";
    std::string dummy;
    std::getline(std::cin, dummy);
}

std::string ConsoleUI::promptString(const std::string& prompt) {
    if (std::cin.eof()) return "";
    std::cout << prompt;
    std::string val;
    if (!std::getline(std::cin, val)) return "";
    size_t first = val.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = val.find_last_not_of(" \t\r\n");
    return val.substr(first, (last - first + 1));
}

double ConsoleUI::promptDouble(const std::string& prompt, double minVal) {
    while (!std::cin.eof()) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) break;
        try {
            double val = std::stod(line);
            if (val >= minVal) {
                return val;
            }
            std::cout << "  [!] Value must be at least " << minVal << ". Please try again.\n";
        } catch (...) {
            std::cout << "  [!] Invalid numeric input. Please try again.\n";
        }
    }
    return minVal;
}

int ConsoleUI::promptInt(const std::string& prompt, int minVal, int maxVal) {
    while (!std::cin.eof()) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) break;
        try {
            int val = std::stoi(line);
            if (val >= minVal && val <= maxVal) {
                return val;
            }
            std::cout << "  [!] Please enter a value between " << minVal << " and " << maxVal << ".\n";
        } catch (...) {
            std::cout << "  [!] Invalid integer input. Please try again.\n";
        }
    }
    return 0;
}

std::string ConsoleUI::promptPin(const std::string& prompt) {
    while (!std::cin.eof()) {
        std::cout << prompt;
        std::string pin;
        if (!std::getline(std::cin, pin)) break;
        size_t first = pin.find_first_not_of(" \t\r\n");
        if (first != std::string::npos) {
            size_t last = pin.find_last_not_of(" \t\r\n");
            pin = pin.substr(first, (last - first + 1));
        }
        if (pin.length() == 4 && pin.find_first_not_of("0123456789") == std::string::npos) {
            return pin;
        }
        std::cout << "  [!] Security PIN must be exactly 4 digits. Please try again.\n";
    }
    return "0000";
}

void ConsoleUI::handleCreateAccount() {
    std::cout << "\n==============================================================\n";
    std::cout << "                 [a] CREATE A NEW BANK ACCOUNT                \n";
    std::cout << "==============================================================\n";

    std::string name = promptString("Enter Account Holder Full Name : ");
    if (name.empty()) {
        std::cout << "[!] Account holder name cannot be empty.\n";
        pressEnterToContinue();
        return;
    }

    std::string phone = promptString("Enter Contact Mobile Number    : ");
    std::string email = promptString("Enter Email Address            : ");

    std::cout << "\nSelect Account Type:\n";
    std::cout << "  1. Savings Account (Min Bal: INR 1,000 | Interest: 4.0% | Daily Limit: INR 25,000)\n";
    std::cout << "  2. Current Account (Min Bal: INR 5,000 | Interest: 0.0% | Daily Limit: INR 1,00,000)\n";
    int typeChoice = promptInt("Enter choice (1 or 2): ", 1, 2);
    std::string type = (typeChoice == 1) ? "SAVINGS" : "CURRENT";
    double minInitial = (typeChoice == 1) ? 1000.0 : 5000.0;

    std::string pin = promptPin("Set 4-Digit Secret PIN         : ");

    std::cout << "\nInitial Deposit must be at least INR " << std::fixed << std::setprecision(2) << minInitial << "\n";
    double initialBal = promptDouble("Enter Initial Deposit Amount   : INR ", minInitial);

    std::cout << "Set Custom Daily Withdrawal Limit (Enter 0 for default) : INR ";
    double customLimit = 0.0;
    std::string limitInput;
    std::getline(std::cin, limitInput);
    if (!limitInput.empty()) {
        try { customLimit = std::stod(limitInput); } catch (...) { customLimit = 0.0; }
    }

    try {
        std::string newAccNo = bank.createAccount(name, phone, email, type, pin, initialBal, customLimit);
        std::cout << "\n--------------------------------------------------------------\n";
        std::cout << " SUCCESS! Account created successfully.\n";
        std::cout << " Generated Account Number : " << newAccNo << "\n";
        std::cout << " Account Type             : " << type << "\n";
        std::cout << " Initial Balance Credited : INR " << std::fixed << std::setprecision(2) << initialBal << "\n";
        std::cout << "--------------------------------------------------------------\n";
    } catch (const std::exception& e) {
        std::cout << "\n[ERROR] Failed to create account: " << e.what() << "\n";
    }

    pressEnterToContinue();
}

void ConsoleUI::handleDepositMoney() {
    std::cout << "\n==============================================================\n";
    std::cout << "                     [b] DEPOSIT MONEY                        \n";
    std::cout << "==============================================================\n";

    std::string accNo = promptString("Enter Account Number : ");
    if (!bank.accountExists(accNo)) {
        std::cout << "[!] Account " << accNo << " does not exist.\n";
        pressEnterToContinue();
        return;
    }

    double amount = promptDouble("Enter Deposit Amount : INR ", 0.01);
    std::string desc = promptString("Enter Description/Remarks (Optional) : ");
    if (desc.empty()) desc = "Cash Deposit";

    try {
        bank.depositMoney(accNo, amount, desc);
        auto acc = bank.getAccount(accNo);
        std::cout << "\n--------------------------------------------------------------\n";
        std::cout << " SUCCESS! INR " << std::fixed << std::setprecision(2) << amount
                  << " successfully deposited into account " << accNo << ".\n";
        std::cout << " New Available Balance : INR " << acc->getRawBalance() << "\n";
        std::cout << "--------------------------------------------------------------\n";
    } catch (const std::exception& e) {
        std::cout << "\n[ERROR] Deposit failed: " << e.what() << "\n";
    }

    pressEnterToContinue();
}

void ConsoleUI::handleWithdrawMoney() {
    std::cout << "\n==============================================================\n";
    std::cout << "                    [c] WITHDRAW MONEY                        \n";
    std::cout << "==============================================================\n";

    std::string accNo = promptString("Enter Account Number : ");
    if (!bank.accountExists(accNo)) {
        std::cout << "[!] Account " << accNo << " does not exist.\n";
        pressEnterToContinue();
        return;
    }

    std::string pin = promptPin("Enter 4-Digit PIN    : ");
    double amount = promptDouble("Enter Withdrawal Amount : INR ", 0.01);
    std::string desc = promptString("Enter Description/Remarks (Optional) : ");
    if (desc.empty()) desc = "ATM / Counter Cash Withdrawal";

    try {
        bank.withdrawMoney(accNo, pin, amount, desc);
        auto acc = bank.getAccount(accNo);
        std::cout << "\n--------------------------------------------------------------\n";
        std::cout << " TRANSACTION APPROVED!\n";
        std::cout << " Dispensed Cash          : INR " << std::fixed << std::setprecision(2) << amount << "\n";
        std::cout << " Updated Account Balance : INR " << acc->getRawBalance() << "\n";
        std::cout << " Remaining Daily Limit   : INR " << acc->getRemainingDailyLimit() << "\n";
        std::cout << "--------------------------------------------------------------\n";
    } catch (const DailyLimitExceededException& e) {
        std::cout << "\n[TRANSACTION DECLINED] " << e.what() << "\n";
    } catch (const InsufficientFundsException& e) {
        std::cout << "\n[TRANSACTION DECLINED] " << e.what() << "\n";
    } catch (const std::exception& e) {
        std::cout << "\n[ERROR] Withdrawal failed: " << e.what() << "\n";
    }

    pressEnterToContinue();
}

void ConsoleUI::handleHonorDailyLimit() {
    std::cout << "\n==============================================================\n";
    std::cout << "             [d] HONOR DAILY WITHDRAWAL LIMIT                 \n";
    std::cout << "==============================================================\n";

    std::string accNo = promptString("Enter Account Number : ");
    if (!bank.accountExists(accNo)) {
        std::cout << "[!] Account " << accNo << " does not exist.\n";
        pressEnterToContinue();
        return;
    }

    std::string pin = promptPin("Enter 4-Digit PIN    : ");

    try {
        auto acc = bank.getAccount(accNo);
        if (!acc->authenticate(pin)) {
            throw AuthenticationException();
        }

        acc->checkDailyLimitReset();
        double dailyLimit = acc->getDailyWithdrawalLimit();
        double withdrawnToday = acc->getWithdrawnToday();
        double remaining = acc->getRemainingDailyLimit();

        std::cout << "\n--------------------------------------------------------------\n";
        std::cout << " DAILY WITHDRAWAL LIMIT STATUS FOR: " << accNo << " (" << acc->getHolderName() << ")\n";
        std::cout << "--------------------------------------------------------------\n";
        std::cout << std::left
                  << std::setw(30) << "Configured Daily Limit"    << ": INR " << std::fixed << std::setprecision(2) << dailyLimit << "\n"
                  << std::setw(30) << "Total Withdrawn Today"     << ": INR " << withdrawnToday << "\n"
                  << std::setw(30) << "Remaining Daily Allowance" << ": INR " << remaining << "\n"
                  << std::setw(30) << "Last Withdrawal Date"      << ": " << acc->getLastWithdrawalDate().toDateString() << "\n";
        std::cout << "--------------------------------------------------------------\n";

        std::cout << "\nSub-operations:\n";
        std::cout << "  1. Test if a specific withdrawal amount honors the daily limit\n";
        std::cout << "  2. Update / Reconfigure Daily Withdrawal Limit\n";
        std::cout << "  3. Return to Main Menu\n";
        int subChoice = promptInt("Choose option (1-3): ", 1, 3);

        if (subChoice == 1) {
            double testAmt = promptDouble("Enter amount to test : INR ", 0.01);
            if (testAmt <= remaining) {
                std::cout << "\n[STATUS: APPROVED] Yes, INR " << testAmt << " is within your remaining daily limit of INR " << remaining << ".\n";
            } else {
                std::cout << "\n[STATUS: REJECTED] No, INR " << testAmt << " exceeds your remaining daily limit of INR " << remaining << "!\n";
            }
        } else if (subChoice == 2) {
            double newLimit = promptDouble("Enter new Daily Withdrawal Limit : INR ", 1000.0);
            bank.updateDailyLimit(accNo, pin, newLimit);
            std::cout << "\n[SUCCESS] Daily withdrawal limit updated to INR " << newLimit << ".\n";
        }
    } catch (const std::exception& e) {
        std::cout << "\n[ERROR] " << e.what() << "\n";
    }

    pressEnterToContinue();
}

void ConsoleUI::handleCheckBalance() {
    std::cout << "\n==============================================================\n";
    std::cout << "                     [e] CHECK THE BALANCE                    \n";
    std::cout << "==============================================================\n";

    std::string accNo = promptString("Enter Account Number : ");
    if (!bank.accountExists(accNo)) {
        std::cout << "[!] Account " << accNo << " does not exist.\n";
        pressEnterToContinue();
        return;
    }

    std::string pin = promptPin("Enter 4-Digit PIN    : ");

    try {
        double bal = bank.checkBalance(accNo, pin);
        auto acc = bank.getAccount(accNo);
        double minBal = acc->getMinimumBalance();
        double withdrawable = (bal > minBal) ? (bal - minBal) : 0.0;
        double remLimit = acc->getRemainingDailyLimit();

        std::cout << "\n--------------------------------------------------------------\n";
        std::cout << " BALANCE ENQUIRY REPORT\n";
        std::cout << " Account Holder    : " << acc->getHolderName() << "\n";
        std::cout << " Account Type      : " << acc->getAccountType() << "\n";
        std::cout << " Current Balance   : INR " << std::fixed << std::setprecision(2) << bal << "\n";
        std::cout << " Minimum Balance   : INR " << minBal << "\n";
        std::cout << " Withdrawable Bal  : INR " << withdrawable << " (Subject to daily limit)\n";
        std::cout << " Today's Rem. Limit: INR " << remLimit << "\n";
        std::cout << "--------------------------------------------------------------\n";
    } catch (const std::exception& e) {
        std::cout << "\n[ERROR] " << e.what() << "\n";
    }

    pressEnterToContinue();
}

void ConsoleUI::handleDisplayAccountInfo() {
    std::cout << "\n==============================================================\n";
    std::cout << "              [f] DISPLAY ACCOUNT INFORMATION                 \n";
    std::cout << "==============================================================\n";

    std::string accNo = promptString("Enter Account Number : ");
    try {
        bank.displayAccountInformation(accNo);
    } catch (const std::exception& e) {
        std::cout << "\n[ERROR] " << e.what() << "\n";
    }

    pressEnterToContinue();
}

void ConsoleUI::handlePassbookPrint() {
    std::cout << "\n==============================================================\n";
    std::cout << "                 [g] PASSBOOK PRINT (FROM TO)                 \n";
    std::cout << "==============================================================\n";

    std::string accNo = promptString("Enter Account Number : ");
    if (!bank.accountExists(accNo)) {
        std::cout << "[!] Account " << accNo << " does not exist.\n";
        pressEnterToContinue();
        return;
    }

    std::string pin = promptPin("Enter 4-Digit PIN    : ");

    try {
        auto acc = bank.getAccount(accNo);
        if (!acc->authenticate(pin)) {
            throw AuthenticationException();
        }

        std::cout << "\nPassbook Print Options:\n";
        std::cout << "  1. Filter by Date Range (From Date to To Date)\n";
        std::cout << "  2. Filter by Transaction Sequence Numbers (From # to To #)\n";
        std::cout << "  3. Print Entire Passbook (All transactions)\n";
        int filterChoice = promptInt("Select option (1-3): ", 1, 3);

        Date fromDate(1970, 1, 1);
        Date toDate = Date::now();

        if (filterChoice == 1) {
            std::cout << "\nEnter Start Date (Format: YYYY-MM-DD, e.g. 2026-01-01) : ";
            std::string fromStr;
            std::getline(std::cin, fromStr);
            fromDate = Date::parse(fromStr);

            std::cout << "Enter End Date   (Format: YYYY-MM-DD, e.g. 2026-12-31) : ";
            std::string toStr;
            std::getline(std::cin, toStr);
            toDate = Date::parse(toStr);

            bank.passbookPrint(accNo, pin, fromDate, toDate, std::cout);
        } else if (filterChoice == 2) {
            std::cout << "Total recorded transactions: " << acc->getTransactions().size() << "\n";
            int fromIdx = promptInt("Enter From Transaction Index: ", 1, (int)acc->getTransactions().size());
            int toIdx = promptInt("Enter To Transaction Index  : ", fromIdx, (int)acc->getTransactions().size());

            bank.passbookPrintRange(accNo, pin, (size_t)fromIdx, (size_t)toIdx, std::cout);
        } else {
            bank.passbookPrintFull(accNo, pin, std::cout);
        }

        // Export to file option
        std::cout << "Would you like to export this passbook statement to a text file? (y/n): ";
        std::string expChoice;
        std::getline(std::cin, expChoice);
        if (!expChoice.empty() && (expChoice[0] == 'y' || expChoice[0] == 'Y')) {
            std::string outPath = "data/passbook_" + accNo + ".txt";
            if (bank.exportPassbookToFile(accNo, pin, outPath, fromDate, toDate)) {
                std::cout << "[SUCCESS] Passbook statement saved to: " << outPath << "\n";
            } else {
                std::cout << "[ERROR] Could not save passbook file.\n";
            }
        }

        // Computer Graphics visualizer prompt
        std::cout << "\nLaunch 2D Computer Graphics Analytics Dashboard for this account? (y/n): ";
        std::string cgChoice;
        std::getline(std::cin, cgChoice);
        if (!cgChoice.empty() && (cgChoice[0] == 'y' || cgChoice[0] == 'Y')) {
            std::cout << "Launching graphical window...\n";
            Visualizer::renderAccountDashboard(*acc);
        }

    } catch (const std::exception& e) {
        std::cout << "\n[ERROR] " << e.what() << "\n";
    }

    pressEnterToContinue();
}

void ConsoleUI::handleGraphicalAnalytics() {
    std::cout << "\n==============================================================\n";
    std::cout << "       COMPUTER GRAPHICS (CG) VISUAL ANALYTICS DASHBOARD      \n";
    std::cout << "==============================================================\n";

    std::string accNo = promptString("Enter Account Number : ");
    if (!bank.accountExists(accNo)) {
        std::cout << "[!] Account " << accNo << " does not exist.\n";
        pressEnterToContinue();
        return;
    }

    try {
        auto acc = bank.getAccount(accNo);
        std::cout << "Rendering 2D Computer Graphics Dashboard for Account " << accNo << "...\n";
        std::cout << "(A graphical window will open. Press ESC or close the window to return.)\n";
        Visualizer::renderAccountDashboard(*acc);
    } catch (const std::exception& e) {
        std::cout << "\n[ERROR] " << e.what() << "\n";
    }

    pressEnterToContinue();
}

void ConsoleUI::handleViewAllAccounts() {
    std::cout << "\n==============================================================\n";
    std::cout << "                   ALL REGISTERED ACCOUNTS                    \n";
    std::cout << "==============================================================\n";

    auto all = bank.getAllAccounts();
    if (all.empty()) {
        std::cout << "No accounts registered in system.\n";
    } else {
        std::cout << std::left
                  << std::setw(12) << "Acc No."
                  << std::setw(10) << "Type"
                  << std::setw(26) << "Holder Name"
                  << std::setw(16) << "Phone"
                  << std::setw(15) << "Balance (INR)"
                  << "Daily Limit (INR)\n";
        std::cout << "----------------------------------------------------------------------------------------\n";
        for (const auto& acc : all) {
            std::cout << std::left
                      << std::setw(12) << acc->getAccountNumber()
                      << std::setw(10) << acc->getAccountType()
                      << std::setw(26) << acc->getHolderName()
                      << std::setw(16) << acc->getPhoneNumber()
                      << std::setw(15) << std::fixed << std::setprecision(2) << acc->getRawBalance()
                      << acc->getDailyWithdrawalLimit() << "\n";
        }
        std::cout << "----------------------------------------------------------------------------------------\n";
        std::cout << "Total Active Accounts: " << all.size() << "\n";
    }

    pressEnterToContinue();
}

void ConsoleUI::run() {
    while (true) {
        if (!std::cin.eof()) clearScreen();
        std::cout << "========================================================================================\n";
        std::cout << "Banking system having the following operations:\n";
        std::cout << "  a. Create an account\n";
        std::cout << "  b. Deposit money\n";
        std::cout << "  c. Withdraw money\n";
        std::cout << "  d. Honor daily withdrawal limit\n";
        std::cout << "  e. Check the balance\n";
        std::cout << "  f. Display Account information.\n";
        std::cout << "  g. Passbook Print (from to)\n";
        std::cout << "----------------------------------------------------------------------------------------\n";
        std::cout << "  h. Computer Graphics: Visual Analytics Dashboard (Bar Chart & Balance Graph)\n";
        std::cout << "  i. View All Accounts Directory\n";
        std::cout << "  q. Save & Exit\n";
        std::cout << "========================================================================================\n";

        std::string choice = promptString("Select an option (a-g, or q to exit): ");
        if (std::cin.eof()) break;
        if (choice.empty()) continue;

        char c = std::tolower(choice[0]);
        if (c == 'a' || choice == "1") {
            handleCreateAccount();
        } else if (c == 'b' || choice == "2") {
            handleDepositMoney();
        } else if (c == 'c' || choice == "3") {
            handleWithdrawMoney();
        } else if (c == 'd' || choice == "4") {
            handleHonorDailyLimit();
        } else if (c == 'e' || choice == "5") {
            handleCheckBalance();
        } else if (c == 'f' || choice == "6") {
            handleDisplayAccountInfo();
        } else if (c == 'g' || choice == "7") {
            handlePassbookPrint();
        } else if (c == 'h' || choice == "8") {
            handleGraphicalAnalytics();
        } else if (c == 'i' || choice == "9") {
            handleViewAllAccounts();
        } else if (c == 'q' || choice == "0") {
            std::cout << "\nSaving bank records to persistent storage...\n";
            bank.saveToFile();
            std::cout << "All changes saved successfully. Thank you for using Apex Banking System!\n";
            return;
        } else {
            std::cout << "\n[!] Invalid choice '" << choice << "'. Please select a letter from a to g, or q to exit.\n";
            pressEnterToContinue();
        }
    }
}
