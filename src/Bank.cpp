#include "Bank.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

Bank::Bank(const std::string& name, const std::string& dataPath)
    : bankName(name), nextSavingsSeq(1001), nextCurrentSeq(2001),
      dataFilePath(dataPath) {
    if (!loadFromFile(dataFilePath)) {
        loadSampleData();
        saveToFile(dataFilePath);
    }
}

std::string Bank::generateAccountNumber(const std::string& type) {
    std::string upperType = type;
    std::transform(upperType.begin(), upperType.end(), upperType.begin(), ::toupper);
    std::ostringstream oss;
    if (upperType == "CURRENT" || upperType == "CA") {
        oss << "CA" << nextCurrentSeq++;
    } else {
        oss << "SB" << nextSavingsSeq++;
    }
    return oss.str();
}

bool Bank::accountExists(const std::string& accNo) const {
    return accounts.find(accNo) != accounts.end();
}

std::shared_ptr<Account> Bank::getAccount(const std::string& accNo) const {
    auto it = accounts.find(accNo);
    if (it == accounts.end()) {
        throw AccountNotFoundException(accNo);
    }
    return it->second;
}

std::vector<std::shared_ptr<Account>> Bank::getAllAccounts() const {
    std::vector<std::shared_ptr<Account>> result;
    for (const auto& pair : accounts) {
        result.push_back(pair.second);
    }
    return result;
}

std::string Bank::createAccount(const std::string& name, const std::string& phone,
                                const std::string& email, const std::string& type,
                                const std::string& pin, double initialDeposit,
                                double customDailyLimit) {
    if (pin.length() != 4 || pin.find_first_not_of("0123456789") != std::string::npos) {
        throw BankException("PIN must be an exact 4-digit numeric code.");
    }

    std::string upperType = type;
    std::transform(upperType.begin(), upperType.end(), upperType.begin(), ::toupper);

    std::shared_ptr<Account> acc;
    std::string accNo = generateAccountNumber(upperType);

    if (upperType == "CURRENT" || upperType == "CA") {
        if (initialDeposit < 5000.0) {
            throw InvalidAmountException("Current Account requires a minimum initial deposit of INR 5000.00");
        }
        acc = std::make_shared<CurrentAccount>(accNo, name, phone, email, pin, initialDeposit, customDailyLimit);
    } else {
        if (initialDeposit < 1000.0) {
            throw InvalidAmountException("Savings Account requires a minimum initial deposit of INR 1000.00");
        }
        acc = std::make_shared<SavingsAccount>(accNo, name, phone, email, pin, initialDeposit, customDailyLimit);
    }

    accounts[accNo] = acc;
    saveToFile();
    return accNo;
}

void Bank::depositMoney(const std::string& accNo, double amount, const std::string& desc) {
    auto acc = getAccount(accNo);
    acc->deposit(amount, desc);
    saveToFile();
}

void Bank::withdrawMoney(const std::string& accNo, const std::string& pin,
                         double amount, const std::string& desc) {
    auto acc = getAccount(accNo);
    acc->withdraw(amount, pin, desc);
    saveToFile();
}

double Bank::honorDailyWithdrawalLimit(const std::string& accNo, const std::string& pin,
                                      double attemptedAmount) {
    auto acc = getAccount(accNo);
    if (!acc->authenticate(pin)) {
        throw AuthenticationException();
    }
    acc->checkDailyLimitReset();
    double remaining = acc->getRemainingDailyLimit();

    if (attemptedAmount > 0.0 && attemptedAmount > remaining) {
        throw DailyLimitExceededException(attemptedAmount, acc->getWithdrawnToday(), acc->getDailyWithdrawalLimit());
    }

    return remaining;
}

double Bank::checkBalance(const std::string& accNo, const std::string& pin) {
    auto acc = getAccount(accNo);
    return acc->getBalance(pin);
}

void Bank::displayAccountInformation(const std::string& accNo) const {
    auto acc = getAccount(accNo);
    acc->displayAccountInfo();
}

void Bank::passbookPrint(const std::string& accNo, const std::string& pin,
                         const Date& fromDate, const Date& toDate,
                         std::ostream& out) {
    auto acc = getAccount(accNo);
    if (!acc->authenticate(pin)) {
        throw AuthenticationException();
    }
    acc->printPassbook(fromDate, toDate, out);
}

void Bank::passbookPrintRange(const std::string& accNo, const std::string& pin,
                              size_t fromIdx, size_t toIdx,
                              std::ostream& out) {
    auto acc = getAccount(accNo);
    if (!acc->authenticate(pin)) {
        throw AuthenticationException();
    }
    acc->printPassbookRange(fromIdx, toIdx, out);
}

void Bank::passbookPrintFull(const std::string& accNo, const std::string& pin,
                             std::ostream& out) {
    auto acc = getAccount(accNo);
    if (!acc->authenticate(pin)) {
        throw AuthenticationException();
    }
    acc->printFullPassbook(out);
}

bool Bank::exportPassbookToFile(const std::string& accNo, const std::string& pin,
                               const std::string& filePath,
                               const Date& fromDate, const Date& toDate) {
    auto acc = getAccount(accNo);
    if (!acc->authenticate(pin)) {
        throw AuthenticationException();
    }
    return acc->exportPassbookToFile(filePath, fromDate, toDate);
}

void Bank::updateDailyLimit(const std::string& accNo, const std::string& pin, double newLimit) {
    auto acc = getAccount(accNo);
    if (!acc->authenticate(pin)) {
        throw AuthenticationException();
    }
    acc->setDailyWithdrawalLimit(newLimit);
    saveToFile();
}

void Bank::saveToFile(const std::string& filePath) {
    std::string path = filePath.empty() ? dataFilePath : filePath;
    std::ofstream out(path.c_str());
    if (!out.is_open()) return;

    out << bankName << "\n";
    out << nextSavingsSeq << "|" << nextCurrentSeq << "\n";
    out << accounts.size() << "\n";

    for (const auto& pair : accounts) {
        const auto& acc = pair.second;
        out << "ACCOUNT_BEGIN\n";
        out << acc->serializeHeader() << "\n";
        out << "TRANSACTIONS_BEGIN\n";
        for (const auto& txn : acc->getTransactions()) {
            out << txn.serialize() << "\n";
        }
        out << "TRANSACTIONS_END\n";
        out << "ACCOUNT_END\n";
    }
    out.close();
}

bool Bank::loadFromFile(const std::string& filePath) {
    std::string path = filePath.empty() ? dataFilePath : filePath;
    std::ifstream in(path.c_str());
    if (!in.is_open()) return false;

    std::string line;
    if (!std::getline(in, line)) return false;
    bankName = line;

    if (std::getline(in, line)) {
        size_t pipe = line.find('|');
        if (pipe != std::string::npos) {
            nextSavingsSeq = std::stoi(line.substr(0, pipe));
            nextCurrentSeq = std::stoi(line.substr(pipe + 1));
        }
    }

    size_t accountCount = 0;
    if (std::getline(in, line)) {
        try { accountCount = std::stoul(line); } catch (...) {}
    }

    accounts.clear();

    while (std::getline(in, line)) {
        if (line == "ACCOUNT_BEGIN") {
            if (!std::getline(in, line)) break;
            // parse account header
            std::stringstream ss(line);
            std::string token;
            std::vector<std::string> parts;
            while (std::getline(ss, token, '|')) {
                parts.push_back(token);
            }

            if (parts.size() >= 12) {
                std::string accType = parts[0];
                std::string accNo = parts[1];
                std::string name = parts[2];
                std::string phone = parts[3];
                std::string email = parts[4];
                std::string pin = parts[5];
                double bal = std::stod(parts[6]);
                Date creation = Date::parse(parts[7]);
                double dailyLimit = std::stod(parts[8]);
                double withdrawnToday = std::stod(parts[9]);
                Date lastWithdrawn = Date::parse(parts[10]);

                std::shared_ptr<Account> acc;
                if (accType == "CURRENT") {
                    acc = std::make_shared<CurrentAccount>(accNo, name, phone, email, pin, 0.0, dailyLimit);
                } else {
                    acc = std::make_shared<SavingsAccount>(accNo, name, phone, email, pin, 0.0, dailyLimit);
                }

                int nextId = (parts.size() >= 12) ? std::stoi(parts[11]) : 1001;
                acc->restoreState(bal, creation, withdrawnToday, lastWithdrawn, nextId);

                // We'll read transactions:
                while (std::getline(in, line) && line != "TRANSACTIONS_BEGIN") {}

                // Load all persisted transactions
                while (std::getline(in, line) && line != "TRANSACTIONS_END") {
                    Transaction t = Transaction::deserialize(line);
                    acc->addRawTransaction(t);
                }

                while (std::getline(in, line) && line != "ACCOUNT_END") {}

                accounts[accNo] = acc;
            }
        }
    }

    in.close();
    return (!accounts.empty());
}

void Bank::loadSampleData() {
    // Create pre-loaded realistic sample accounts for demonstration & testing
    std::string sb = createAccount("Vaishnavi Jare", "+91 9876543210", "vaishnavi@sppu.edu", "SAVINGS", "1234", 25000.0, 20000.0);
    // Add realistic activity
    depositMoney(sb, 5000.0, "Salary Credited");
    depositMoney(sb, 2500.0, "Freelance Payment");
    withdrawMoney(sb, "1234", 3000.0, "ATM Cash Withdrawal");
    withdrawMoney(sb, "1234", 1500.0, "Grocery Store POS");

    std::string ca = createAccount("Tech Innovations Corp", "+91 9123456780", "accounts@techcorp.in", "CURRENT", "9999", 75000.0, 50000.0);
    depositMoney(ca, 20000.0, "Client Invoice Payment");
    withdrawMoney(ca, "9999", 10000.0, "Vendor Settlement");
}
