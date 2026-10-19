#ifndef CURRENT_ACCOUNT_H
#define CURRENT_ACCOUNT_H

#include "Account.h"

class CurrentAccount : public Account {
private:
    static constexpr double MINIMUM_BALANCE = 5000.0;
    static constexpr double INTEREST_RATE = 0.0;
    static constexpr double DEFAULT_DAILY_LIMIT = 100000.0;

public:
    CurrentAccount(const std::string& accNo, const std::string& name,
                   const std::string& phone, const std::string& mail,
                   const std::string& pinCode, double initialBal,
                   double dailyLimit = DEFAULT_DAILY_LIMIT);

    std::string getAccountType() const override { return "CURRENT"; }
    double getMinimumBalance() const override { return MINIMUM_BALANCE; }
    double getInterestRate() const override { return INTEREST_RATE; }
    void applyInterest() override;
};

#endif // CURRENT_ACCOUNT_H
