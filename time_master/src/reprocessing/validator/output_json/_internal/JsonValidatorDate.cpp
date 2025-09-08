// reprocessing/validator/output_json/JsonValidatorDate.cpp
#include "JsonValidatorDate.hpp"
#include <map>
#include <string>

using json = nlohmann::json;

namespace {
    bool is_leap(int year) { return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)); }
    int days_in_month(int year, int month) {
        if (month < 1 || month > 12) return 0;
        if (month == 2) return is_leap(year) ? 29 : 28;
        if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
        return 31;
    }
}

void validateDateContinuity(const json& days_array, std::set<Error>& errors) {
    if (days_array.empty()) return;
    
    const auto& first_day_headers = days_array[0].value("Headers", json::object());
    std::string first_date = first_day_headers.value("Date", "");
    if (first_date.length() != 8) return;
    
    std::string yyyymm = first_date.substr(0, 6);
    int year = std::stoi(yyyymm.substr(0, 4));
    int month = std::stoi(yyyymm.substr(4, 2));

    std::map<std::string, std::set<int>> month_day_map;
    for (const auto& day : days_array) {
        const auto& headers = day.value("Headers", json::object());
        std::string date_str = headers.value("Date", "");
        if (date_str.rfind(yyyymm, 0) == 0 && date_str.length() == 8) {
            month_day_map[yyyymm].insert(std::stoi(date_str.substr(6, 2)));
        }
    }

    const auto& days_found = month_day_map[yyyymm];
    int num_days_in_this_month = days_in_month(year, month);
            
    for (int d = 1; d <= num_days_in_this_month; ++d) {
        if (days_found.find(d) == days_found.end()) {
            std::string missing_date_str = yyyymm + (d < 10 ? "0" : "") + std::to_string(d);
            errors.insert({0, "Missing date detected in month " + yyyymm.substr(0,4) + "-" + yyyymm.substr(4,2) + ": " + missing_date_str, ErrorType::DateContinuity});
        }
    }
}