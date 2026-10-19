#include "Transaction.h"
#include <sstream>
#include <iomanip>
#include <vector>

Transaction::Transaction()
    : txnId(""), timestamp(Date()), type(TransactionType::DEPOSIT),
      amount(0.0), balanceAfter(0.0), description("") {}

Transaction::Transaction(const std::string& id, const Date& time, TransactionType t,
                         double amt, double balAfter, const std::string& desc)
    : txnId(id), timestamp(time), type(t), amount(amt),
      balanceAfter(balAfter), description(desc) {}

std::string Transaction::getTypeString() const {
    switch (type) {
        case TransactionType::ACCOUNT_CREATION: return "OPENING";
        case TransactionType::DEPOSIT:          return "DEPOSIT";
        case TransactionType::WITHDRAWAL:       return "WITHDRAWAL";
        case TransactionType::INTEREST:         return "INTEREST";
        default:                                return "UNKNOWN";
    }
}

std::string Transaction::serialize() const {
    std::ostringstream oss;
    oss << txnId << "|"
        << timestamp.toDateTimeString() << "|"
        << static_cast<int>(type) << "|"
        << std::fixed << std::setprecision(2) << amount << "|"
        << std::fixed << std::setprecision(2) << balanceAfter << "|"
        << description;
    return oss.str();
}

Transaction Transaction::deserialize(const std::string& line) {
    std::stringstream ss(line);
    std::string item;
    std::vector<std::string> tokens;

    while (std::getline(ss, item, '|')) {
        tokens.push_back(item);
    }

    if (tokens.size() >= 6) {
        std::string id = tokens[0];
        Date time = Date::parse(tokens[1]);
        TransactionType t = static_cast<TransactionType>(std::stoi(tokens[2]));
        double amt = std::stod(tokens[3]);
        double balAfter = std::stod(tokens[4]);
        std::string desc = tokens[5];
        return Transaction(id, time, t, amt, balAfter, desc);
    }
    return Transaction();
}

std::ostream& operator<<(std::ostream& os, const Transaction& txn) {
    os << "[" << txn.timestamp.toDateTimeString() << "] "
       << txn.txnId << " | "
       << std::setw(11) << txn.getTypeString() << " | "
       << "₹" << std::fixed << std::setprecision(2) << txn.amount << " | "
       << "Bal: ₹" << txn.balanceAfter << " | "
       << txn.description;
    return os;
}
