#include "SavingsAccount.h"

constexpr double SavingsAccount::MINIMUM_BALANCE;
constexpr double SavingsAccount::INTEREST_RATE;
constexpr double SavingsAccount::DEFAULT_DAILY_LIMIT;

SavingsAccount::SavingsAccount(const std::string& accNo, const std::string& name,
                               const std::string& phone, const std::string& mail,
                               const std::string& pinCode, double initialBal,
                               double dailyLimit)
    : Account(accNo, name, phone, mail, pinCode, initialBal,
              (dailyLimit > 0.0) ? dailyLimit : DEFAULT_DAILY_LIMIT) {}

void SavingsAccount::applyInterest() {
    double interestAmt = (balance * INTEREST_RATE) / 100.0;
    if (interestAmt > 0) {
        balance += interestAmt;
        transactions.push_back(
            Transaction(generateTxnId(), Date::now(),
                        TransactionType::INTEREST,
                        interestAmt, balance, "Annual Interest Credit @ 4.0%")
        );
    }
}
