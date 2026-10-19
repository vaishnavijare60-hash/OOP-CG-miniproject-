#ifndef BANK_H
#define BANK_H

#include <string>
#include <map>
#include <memory>
#include <vector>
#include "Account.h"
#include "SavingsAccount.h"
#include "CurrentAccount.h"
#include "Exceptions.h"

class Bank {
private:
    std::string bankName;
    std::map<std::string, std::shared_ptr<Account>> accounts;
    int nextSavingsSeq;
    int nextCurrentSeq;
    std::string dataFilePath;

    std::string generateAccountNumber(const std::string& type);

public:
    Bank(const std::string& name = "APEX NATIONAL BANK",
         const std::string& dataPath = "data/bank_data.txt");

    // Core Operations (a through g)
    std::string createAccount(const std::string& name, const std::string& phone,
                              const std::string& email, const std::string& type,
                              const std::string& pin, double initialDeposit,
                              double customDailyLimit = 0.0);

    void depositMoney(const std::string& accNo, double amount,
                      const std::string& desc = "Cash Deposit");

    void withdrawMoney(const std::string& accNo, const std::string& pin,
                       double amount, const std::string& desc = "Cash Withdrawal");

    double honorDailyWithdrawalLimit(const std::string& accNo, const std::string& pin,
                                     double attemptedAmount = 0.0);

    double checkBalance(const std::string& accNo, const std::string& pin);

    void displayAccountInformation(const std::string& accNo) const;

    void passbookPrint(const std::string& accNo, const std::string& pin,
                       const Date& fromDate, const Date& toDate,
                       std::ostream& out = std::cout);

    void passbookPrintRange(const std::string& accNo, const std::string& pin,
                            size_t fromIdx, size_t toIdx,
                            std::ostream& out = std::cout);

    void passbookPrintFull(const std::string& accNo, const std::string& pin,
                           std::ostream& out = std::cout);

    bool exportPassbookToFile(const std::string& accNo, const std::string& pin,
                              const std::string& filePath,
                              const Date& fromDate, const Date& toDate);

    void updateDailyLimit(const std::string& accNo, const std::string& pin, double newLimit);

    // Utilities & File Persistence
    bool accountExists(const std::string& accNo) const;
    std::shared_ptr<Account> getAccount(const std::string& accNo) const;
    std::vector<std::shared_ptr<Account>> getAllAccounts() const;

    void saveToFile(const std::string& filePath = "");
    bool loadFromFile(const std::string& filePath = "");
    void loadSampleData();

    std::string getBankName() const { return bankName; }
    size_t getTotalAccounts() const { return accounts.size(); }
};

#endif // BANK_H
