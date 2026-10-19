#ifndef DATE_H
#define DATE_H

#include <string>
#include <iostream>

class Date {
private:
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;

public:
    Date();
    Date(int y, int m, int d, int h = 0, int min = 0, int s = 0);

    // Factory methods
    static Date now();
    static Date parse(const std::string& str);
    static bool isValidDate(int y, int m, int d);

    // Getters
    int getYear() const { return year; }
    int getMonth() const { return month; }
    int getDay() const { return day; }
    int getHour() const { return hour; }
    int getMinute() const { return minute; }
    int getSecond() const { return second; }

    // Helpers
    std::string toDateString() const;
    std::string toDateTimeString() const;
    bool isSameDay(const Date& other) const;

    // Operator overloads
    bool operator<(const Date& other) const;
    bool operator<=(const Date& other) const;
    bool operator>(const Date& other) const;
    bool operator>=(const Date& other) const;
    bool operator==(const Date& other) const;
    bool operator!=(const Date& other) const;

    friend std::ostream& operator<<(std::ostream& os, const Date& d);
};

#endif // DATE_H
