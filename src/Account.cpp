#include "Account.h"
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>

Account::Account(const std::string& accNo, const std::string& name,
                 const std::string& phone, const std::string& mail,
                 const std::string& pinCode, double initialBal,
                 double dailyLimit)
    : accountNumber(accNo), holderName(name), phoneNumber(phone),
      email(mail), pin(pinCode), balance(initialBal),
      creationDate(Date::now()), dailyWithdrawalLimit(dailyLimit),
      withdrawnToday(0.0), lastWithdrawalDate(Date::now()),
      nextTxnId(1001) {

    // Record initial opening transaction
    if (initialBal > 0) {
        transactions.push_back(
            Transaction(generateTxnId(), creationDate,
                        TransactionType::ACCOUNT_CREATION,
                        initialBal, initialBal, "Initial Account Opening Deposit")
        );
    }
}

std::string Account::generateTxnId() {
    std::ostringstream oss;
    oss << "TXN" << nextTxnId++;
    return oss.str();
}

bool Account::authenticate(const std::string& inputPin) const {
    return (pin == inputPin);
}

void Account::checkDailyLimitReset() {
    Date today = Date::now();
    if (!today.isSameDay(lastWithdrawalDate)) {
        withdrawnToday = 0.0;
        lastWithdrawalDate = today;
    }
}

double Account::getRemainingDailyLimit() {
    checkDailyLimitReset();
    return (dailyWithdrawalLimit > withdrawnToday) ? (dailyWithdrawalLimit - withdrawnToday) : 0.0;
}

void Account::setDailyWithdrawalLimit(double newLimit) {
    if (newLimit < 0) {
        throw InvalidAmountException("Daily withdrawal limit cannot be negative.");
    }
    dailyWithdrawalLimit = newLimit;
}

void Account::deposit(double amount, const std::string& desc) {
    if (amount <= 0) {
        throw InvalidAmountException("Deposit amount must be strictly greater than zero.");
    }

    balance += amount;
    transactions.push_back(
        Transaction(generateTxnId(), Date::now(),
                    TransactionType::DEPOSIT,
                    amount, balance, desc.empty() ? "Cash Deposit" : desc)
    );
}

void Account::withdraw(double amount, const std::string& inputPin, const std::string& desc) {
    if (!authenticate(inputPin)) {
        throw AuthenticationException();
    }
    if (amount <= 0) {
        throw InvalidAmountException("Withdrawal amount must be strictly greater than zero.");
    }

    checkDailyLimitReset();

    // Check Daily Withdrawal Limit (Requirement d)
    if (withdrawnToday + amount > dailyWithdrawalLimit) {
        throw DailyLimitExceededException(amount, withdrawnToday, dailyWithdrawalLimit);
    }

    // Check Balance including Minimum Balance requirement
    if (balance - amount < getMinimumBalance()) {
        throw InsufficientFundsException(amount, balance, getMinimumBalance());
    }

    balance -= amount;
    withdrawnToday += amount;
    lastWithdrawalDate = Date::now();

    transactions.push_back(
        Transaction(generateTxnId(), Date::now(),
                    TransactionType::WITHDRAWAL,
                    amount, balance, desc.empty() ? "Cash Withdrawal" : desc)
    );
}

double Account::getBalance(const std::string& inputPin) const {
    if (!authenticate(inputPin)) {
        throw AuthenticationException();
    }
    return balance;
}

void Account::displayAccountInfo() const {
    const_cast<Account*>(this)->checkDailyLimitReset();
    double remainingLimit = (dailyWithdrawalLimit > withdrawnToday) ? (dailyWithdrawalLimit - withdrawnToday) : 0.0;

    std::cout << "\n==============================================================\n";
    std::cout << "                 ACCOUNT INFORMATION DETAILS                   \n";
    std::cout << "==============================================================\n";
    std::cout << std::left
              << std::setw(26) << "Account Number"        << ": " << accountNumber << "\n"
              << std::setw(26) << "Account Type"          << ": " << getAccountType() << "\n"
              << std::setw(26) << "Account Holder"        << ": " << holderName << "\n"
              << std::setw(26) << "Contact Phone"         << ": " << phoneNumber << "\n"
              << std::setw(26) << "Email Address"         << ": " << email << "\n"
              << std::setw(26) << "Account Created On"    << ": " << creationDate.toDateTimeString() << "\n"
              << "--------------------------------------------------------------\n"
              << std::setw(26) << "Current Balance"       << ": INR " << std::fixed << std::setprecision(2) << balance << "\n"
              << std::setw(26) << "Minimum Balance"       << ": INR " << getMinimumBalance() << "\n"
              << std::setw(26) << "Annual Interest Rate"  << ": " << getInterestRate() << "%\n"
              << "--------------------------------------------------------------\n"
              << std::setw(26) << "Daily Withdrawal Limit" << ": INR " << dailyWithdrawalLimit << "\n"
              << std::setw(26) << "Withdrawn Today"       << ": INR " << withdrawnToday << "\n"
              << std::setw(26) << "Remaining Daily Limit" << ": INR " << remainingLimit << "\n"
              << std::setw(26) << "Total Transactions"    << ": " << transactions.size() << "\n";
    std::cout << "==============================================================\n";
}

void Account::printPassbook(const Date& fromDate, const Date& toDate, std::ostream& out) const {
    out << "\n";
    out << "========================================================================================\n";
    out << "                               OFFICIAL BANK PASSBOOK                                   \n";
    out << "========================================================================================\n";
    out << "Account Number: " << std::setw(15) << std::left << accountNumber
        << "Holder Name : " << holderName << "\n";
    out << "Account Type  : " << std::setw(15) << std::left << getAccountType()
        << "Filter Range: " << fromDate.toDateString() << " to " << toDate.toDateString() << "\n";
    out << "Current Bal   : INR " << std::fixed << std::setprecision(2) << balance << "\n";
    out << "----------------------------------------------------------------------------------------\n";
    out << std::left
        << std::setw(20) << "Date & Time"
        << std::setw(11) << "Txn ID"
        << std::setw(12) << "Type"
        << std::setw(14) << "Debit (INR)"
        << std::setw(14) << "Credit (INR)"
        << std::setw(15) << "Balance (INR)"
        << "Description\n";
    out << "----------------------------------------------------------------------------------------\n";

    double totalDebit = 0.0;
    double totalCredit = 0.0;
    int matchedCount = 0;

    // We compare based on calendar day or full timestamp
    for (const auto& txn : transactions) {
        Date txnDate = txn.getTimestamp();
        // Check date inclusive range
        Date fromStart(fromDate.getYear(), fromDate.getMonth(), fromDate.getDay(), 0, 0, 0);
        Date toEnd(toDate.getYear(), toDate.getMonth(), toDate.getDay(), 23, 59, 59);

        if (txnDate >= fromStart && txnDate <= toEnd) {
            matchedCount++;
            std::string debitStr = "-";
            std::string creditStr = "-";

            if (txn.getType() == TransactionType::WITHDRAWAL) {
                std::ostringstream oss;
                oss << std::fixed << std::setprecision(2) << txn.getAmount();
                debitStr = oss.str();
                totalDebit += txn.getAmount();
            } else {
                std::ostringstream oss;
                oss << std::fixed << std::setprecision(2) << txn.getAmount();
                creditStr = oss.str();
                totalCredit += txn.getAmount();
            }

            std::ostringstream balOss;
            balOss << std::fixed << std::setprecision(2) << txn.getBalanceAfter();

            out << std::left
                << std::setw(20) << txn.getTimestamp().toDateTimeString()
                << std::setw(11) << txn.getTxnId()
                << std::setw(12) << txn.getTypeString()
                << std::setw(14) << debitStr
                << std::setw(14) << creditStr
                << std::setw(15) << balOss.str()
                << txn.getDescription() << "\n";
        }
    }

    if (matchedCount == 0) {
        out << "              No transactions found within the specified date range.                     \n";
    }

    out << "----------------------------------------------------------------------------------------\n";
    out << "SUMMARY FOR PERIOD:\n";
    out << "  * Total Transactions Recorded : " << matchedCount << "\n";
    out << "  * Total Amount Debited        : INR " << std::fixed << std::setprecision(2) << totalDebit << "\n";
    out << "  * Total Amount Credited       : INR " << std::fixed << std::setprecision(2) << totalCredit << "\n";
    out << "  * Net Transaction Difference  : INR " << std::fixed << std::setprecision(2) << (totalCredit - totalDebit) << "\n";
    out << "========================================================================================\n\n";
}

void Account::printPassbookRange(size_t fromIdx, size_t toIdx, std::ostream& out) const {
    if (transactions.empty()) {
        out << "No transactions recorded yet.\n";
        return;
    }

    if (fromIdx < 1) fromIdx = 1;
    if (toIdx > transactions.size()) toIdx = transactions.size();
    if (fromIdx > toIdx) {
        out << "Invalid range specified.\n";
        return;
    }

    Date fromDate = transactions[fromIdx - 1].getTimestamp();
    Date toDate = transactions[toIdx - 1].getTimestamp();
    printPassbook(fromDate, toDate, out);
}

void Account::printFullPassbook(std::ostream& out) const {
    if (transactions.empty()) {
        out << "No transactions to print.\n";
        return;
    }
    Date fromDate = transactions.front().getTimestamp();
    Date toDate = transactions.back().getTimestamp();
    printPassbook(fromDate, toDate, out);
}

bool Account::exportPassbookToFile(const std::string& filePath, const Date& fromDate, const Date& toDate) const {
    std::ofstream outFile(filePath.c_str());
    if (!outFile.is_open()) {
        return false;
    }
    printPassbook(fromDate, toDate, outFile);
    outFile.close();
    return true;
}

std::string Account::serializeHeader() const {
    std::ostringstream oss;
    oss << getAccountType() << "|"
        << accountNumber << "|"
        << holderName << "|"
        << phoneNumber << "|"
        << email << "|"
        << pin << "|"
        << std::fixed << std::setprecision(2) << balance << "|"
        << creationDate.toDateTimeString() << "|"
        << std::fixed << std::setprecision(2) << dailyWithdrawalLimit << "|"
        << std::fixed << std::setprecision(2) << withdrawnToday << "|"
        << lastWithdrawalDate.toDateTimeString() << "|"
        << nextTxnId << "|"
        << transactions.size();
    return oss.str();
}

void Account::addRawTransaction(const Transaction& txn) {
    transactions.push_back(txn);
    // keep nextTxnId ahead
    std::string id = txn.getTxnId();
    if (id.rfind("TXN", 0) == 0) {
        try {
            int num = std::stoi(id.substr(3));
            if (num >= nextTxnId) {
                nextTxnId = num + 1;
            }
        } catch (...) {}
    }
}

void Account::restoreState(double bal, const Date& created, double withdrawn, const Date& lastWithdrawn, int nextId) {
    balance = bal;
    creationDate = created;
    withdrawnToday = withdrawn;
    lastWithdrawalDate = lastWithdrawn;
    nextTxnId = nextId;
}

std::ostream& operator<<(std::ostream& os, const Account& acc) {
    os << "[" << acc.getAccountType() << "] "
       << acc.accountNumber << " - " << acc.holderName
       << " | Balance: INR " << std::fixed << std::setprecision(2) << acc.balance;
    return os;
}
