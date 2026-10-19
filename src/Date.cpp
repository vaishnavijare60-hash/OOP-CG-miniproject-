#include "Date.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cstdio>

Date::Date() : year(1970), month(1), day(1), hour(0), minute(0), second(0) {}

Date::Date(int y, int m, int d, int h, int min, int s)
    : year(y), month(m), day(d), hour(h), minute(min), second(s) {}

Date Date::now() {
    std::time_t t = std::time(nullptr);
    std::tm* localTime = std::localtime(&t);
    if (!localTime) {
        return Date(2026, 1, 1, 0, 0, 0);
    }
    return Date(
        localTime->tm_year + 1900,
        localTime->tm_mon + 1,
        localTime->tm_mday,
        localTime->tm_hour,
        localTime->tm_min,
        localTime->tm_sec
    );
}

bool Date::isValidDate(int y, int m, int d) {
    if (y < 1900 || y > 2100) return false;
    if (m < 1 || m > 12) return false;
    if (d < 1) return false;

    int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    bool isLeap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
    if (isLeap && m == 2) {
        return d <= 29;
    }
    return d <= daysInMonth[m - 1];
}

Date Date::parse(const std::string& str) {
    // Formats: YYYY-MM-DD or YYYY-MM-DD HH:MM:SS
    int y = 1970, m = 1, d = 1, h = 0, min = 0, s = 0;
    if (std::sscanf(str.c_str(), "%d-%d-%d %d:%d:%d", &y, &m, &d, &h, &min, &s) >= 3) {
        return Date(y, m, d, h, min, s);
    }
    return Date();
}

std::string Date::toDateString() const {
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(4) << year << "-"
        << std::setw(2) << month << "-"
        << std::setw(2) << day;
    return oss.str();
}

std::string Date::toDateTimeString() const {
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(4) << year << "-"
        << std::setw(2) << month << "-"
        << std::setw(2) << day << " "
        << std::setw(2) << hour << ":"
        << std::setw(2) << minute << ":"
        << std::setw(2) << second;
    return oss.str();
}

bool Date::isSameDay(const Date& other) const {
    return (year == other.year && month == other.month && day == other.day);
}

bool Date::operator<(const Date& other) const {
    if (year != other.year) return year < other.year;
    if (month != other.month) return month < other.month;
    if (day != other.day) return day < other.day;
    if (hour != other.hour) return hour < other.hour;
    if (minute != other.minute) return minute < other.minute;
    return second < other.second;
}

bool Date::operator<=(const Date& other) const {
    return (*this < other) || (*this == other);
}

bool Date::operator>(const Date& other) const {
    return !(*this <= other);
}

bool Date::operator>=(const Date& other) const {
    return !(*this < other);
}

bool Date::operator==(const Date& other) const {
    return (year == other.year && month == other.month && day == other.day &&
            hour == other.hour && minute == other.minute && second == other.second);
}

bool Date::operator!=(const Date& other) const {
    return !(*this == other);
}

std::ostream& operator<<(std::ostream& os, const Date& d) {
    os << d.toDateTimeString();
    return os;
}
