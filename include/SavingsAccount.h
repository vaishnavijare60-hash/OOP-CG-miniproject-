#ifndef SAVINGS_ACCOUNT_H
#define SAVINGS_ACCOUNT_H

#include "Account.h"

class SavingsAccount : public Account {
private:
    static constexpr double MINIMUM_BALANCE = 1000.0;
    static constexpr double INTEREST_RATE = 4.0; // 4% per annum
    static constexpr double DEFAULT_DAILY_LIMIT = 25000.0;

public:
    SavingsAccount(const std::string& accNo, const std::string& name,
                   const std::string& phone, const std::string& mail,
                   const std::string& pinCode, double initialBal,
                   double dailyLimit = DEFAULT_DAILY_LIMIT);

    std::string getAccountType() const override { return "SAVINGS"; }
    double getMinimumBalance() const override { return MINIMUM_BALANCE; }
    double getInterestRate() const override { return INTEREST_RATE; }
    void applyInterest() override;
};

#endif // SAVINGS_ACCOUNT_H
