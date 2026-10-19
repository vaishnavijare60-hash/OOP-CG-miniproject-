#include "CurrentAccount.h"

constexpr double CurrentAccount::MINIMUM_BALANCE;
constexpr double CurrentAccount::INTEREST_RATE;
constexpr double CurrentAccount::DEFAULT_DAILY_LIMIT;

CurrentAccount::CurrentAccount(const std::string& accNo, const std::string& name,
                               const std::string& phone, const std::string& mail,
                               const std::string& pinCode, double initialBal,
                               double dailyLimit)
    : Account(accNo, name, phone, mail, pinCode, initialBal,
              (dailyLimit > 0.0) ? dailyLimit : DEFAULT_DAILY_LIMIT) {}

void CurrentAccount::applyInterest() {
    // Current accounts do not earn interest
}
