#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include <iostream>
#include "Date.h"

enum class TransactionType {
    ACCOUNT_CREATION,
    DEPOSIT,
    WITHDRAWAL,
    INTEREST
};

class Transaction {
private:
    std::string txnId;
    Date timestamp;
    TransactionType type;
    double amount;
    double balanceAfter;
    std::string description;

public:
    Transaction();
    Transaction(const std::string& id, const Date& time, TransactionType t,
                double amt, double balAfter, const std::string& desc = "");

    // Getters
    std::string getTxnId() const { return txnId; }
    Date getTimestamp() const { return timestamp; }
    TransactionType getType() const { return type; }
    std::string getTypeString() const;
    double getAmount() const { return amount; }
    double getBalanceAfter() const { return balanceAfter; }
    std::string getDescription() const { return description; }

    // Serialization for File I/O
    std::string serialize() const;
    static Transaction deserialize(const std::string& line);

    friend std::ostream& operator<<(std::ostream& os, const Transaction& txn);
};

#endif // TRANSACTION_H
