#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <string>
#include <sstream>
#include <iomanip>

class BankException : public std::runtime_error {
public:
    explicit BankException(const std::string& msg) : std::runtime_error(msg) {}
};

class AccountNotFoundException : public BankException {
public:
    explicit AccountNotFoundException(const std::string& accNo)
        : BankException("Account Not Found: Account Number '" + accNo + "' does not exist.") {}
};

class InsufficientFundsException : public BankException {
private:
    double requested;
    double currentBalance;
    double minBalance;
public:
    InsufficientFundsException(double req, double bal, double minBal)
        : BankException("Insufficient Funds: Requested withdrawal of ₹" + std::to_string(req) +
                        " exceeds available balance. (Current: ₹" + std::to_string(bal) +
                        ", Min Balance: ₹" + std::to_string(minBal) + ")"),
          requested(req), currentBalance(bal), minBalance(minBal) {}

    double getRequested() const { return requested; }
    double getCurrentBalance() const { return currentBalance; }
    double getMinBalance() const { return minBalance; }
};

class DailyLimitExceededException : public BankException {
private:
    double requested;
    double withdrawnToday;
    double dailyLimit;
    double remainingLimit;

    static std::string buildMessage(double req, double today, double limit) {
        double remaining = limit - today;
        if (remaining < 0) remaining = 0;
        std::ostringstream oss;
        oss << "Daily Withdrawal Limit Exceeded!\n"
            << "  * Daily Limit Allowed : INR " << std::fixed << std::setprecision(2) << limit << "\n"
            << "  * Already Withdrawn   : INR " << std::fixed << std::setprecision(2) << today << "\n"
            << "  * Remaining for Today : INR " << std::fixed << std::setprecision(2) << remaining << "\n"
            << "  * Attempted Amount    : INR " << std::fixed << std::setprecision(2) << req;
        return oss.str();
    }

public:
    DailyLimitExceededException(double req, double today, double limit)
        : BankException(buildMessage(req, today, limit)),
          requested(req), withdrawnToday(today), dailyLimit(limit),
          remainingLimit(limit > today ? limit - today : 0.0) {}

    double getRequested() const { return requested; }
    double getWithdrawnToday() const { return withdrawnToday; }
    double getDailyLimit() const { return dailyLimit; }
    double getRemainingLimit() const { return remainingLimit; }
};

class InvalidAmountException : public BankException {
public:
    explicit InvalidAmountException(const std::string& reason)
        : BankException("Invalid Amount: " + reason) {}
};

class AuthenticationException : public BankException {
public:
    AuthenticationException()
        : BankException("Authentication Failed: Incorrect 4-digit PIN provided.") {}
};

class AccountAlreadyExistsException : public BankException {
public:
    explicit AccountAlreadyExistsException(const std::string& accNo)
        : BankException("Account already exists with number: " + accNo) {}
};

#endif // EXCEPTIONS_H
