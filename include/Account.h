#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include "Date.h"
#include "Transaction.h"
#include "Exceptions.h"

class Account {
protected:
    std::string accountNumber;
    std::string holderName;
    std::string phoneNumber;
    std::string email;
    std::string pin;
    double balance;
    Date creationDate;
    double dailyWithdrawalLimit;
    double withdrawnToday;
    Date lastWithdrawalDate;
    std::vector<Transaction> transactions;
    int nextTxnId;

    std::string generateTxnId();

public:
    Account(const std::string& accNo, const std::string& name,
            const std::string& phone, const std::string& mail,
            const std::string& pinCode, double initialBal,
            double dailyLimit);
    virtual ~Account() {}

    // Pure virtual functions (polymorphic interface)
    virtual std::string getAccountType() const = 0;
    virtual double getMinimumBalance() const = 0;
    virtual double getInterestRate() const = 0;
    virtual void applyInterest() = 0;

    // Authentication
    bool authenticate(const std::string& inputPin) const;

    // Operations (a through g)
    virtual void deposit(double amount, const std::string& desc = "Deposit");
    virtual void withdraw(double amount, const std::string& inputPin, const std::string& desc = "Withdrawal");
    virtual void checkDailyLimitReset();
    virtual double getRemainingDailyLimit();
    virtual void setDailyWithdrawalLimit(double newLimit);

    double getBalance(const std::string& inputPin) const;
    double getRawBalance() const { return balance; }

    virtual void displayAccountInfo() const;

    // Passbook Operations
    virtual void printPassbook(const Date& fromDate, const Date& toDate, std::ostream& out = std::cout) const;
    virtual void printPassbookRange(size_t fromIdx, size_t toIdx, std::ostream& out = std::cout) const;
    virtual void printFullPassbook(std::ostream& out = std::cout) const;
    virtual bool exportPassbookToFile(const std::string& filePath, const Date& fromDate, const Date& toDate) const;

    // Getters
    std::string getAccountNumber() const { return accountNumber; }
    std::string getHolderName() const { return holderName; }
    std::string getPhoneNumber() const { return phoneNumber; }
    std::string getEmail() const { return email; }
    std::string getPin() const { return pin; }
    Date getCreationDate() const { return creationDate; }
    double getDailyWithdrawalLimit() const { return dailyWithdrawalLimit; }
    double getWithdrawnToday() const { return withdrawnToday; }
    Date getLastWithdrawalDate() const { return lastWithdrawalDate; }
    const std::vector<Transaction>& getTransactions() const { return transactions; }

    // Serialization & Restoration
    virtual std::string serializeHeader() const;
    virtual void addRawTransaction(const Transaction& txn);
    void restoreState(double bal, const Date& created, double withdrawn, const Date& lastWithdrawn, int nextId);

    // Operator overload
    friend std::ostream& operator<<(std::ostream& os, const Account& acc);
};

#endif // ACCOUNT_H
