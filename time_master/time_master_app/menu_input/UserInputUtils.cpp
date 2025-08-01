// common/UserInputUtils.cpp
#include "common/pch.h"
#include "UserInputUtils.h"
#include "common/common_utils.h" // For color constants
#include <iostream>
#include <filesystem>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

namespace UserInputUtils {

std::string get_valid_path_input(const std::string& prompt_message) {
    std::cout << prompt_message;
    std::string path_str;
    std::getline(std::cin, path_str);
    if (path_str.empty()) return "";
    if (!fs::exists(path_str)) {
        std::cerr << RED_COLOR << "Error: Path '" << path_str << "' does not exist. Aborting." << RESET_COLOR << std::endl;
        return "";
    }
    return path_str;
}

std::string get_valid_date_input() {
    std::string date_str;
    while (true) {
        std::cout << "Enter date (YYYYMMDD): ";
        if (!std::getline(std::cin, date_str) || date_str.empty()) { return ""; }
        if (date_str.length() == 8 && std::all_of(date_str.begin(), date_str.end(), ::isdigit)) {
            int year = std::stoi(date_str.substr(0, 4));
            int month = std::stoi(date_str.substr(4, 2));
            int day = std::stoi(date_str.substr(6, 2));
            if (year > 1900 && year < 3000 && month >= 1 && month <= 12 && day >= 1 && day <= 31) break;
        }
        std::cout << YELLOW_COLOR << "Invalid date format or value. " << RESET_COLOR << "Please use YYYYMMDD." << std::endl;
    }
    return date_str;
}

std::string get_valid_month_input() {
    std::string month_str;
    while (true) {
        std::cout << "Enter month (YYYYMM): ";
        if (!std::getline(std::cin, month_str) || month_str.empty()) { return ""; }
        if (month_str.length() == 6 && std::all_of(month_str.begin(), month_str.end(), ::isdigit)) {
            int year = std::stoi(month_str.substr(0, 4));
            int month = std::stoi(month_str.substr(4, 2));
            if (year > 1900 && year < 3000 && month >= 1 && month <= 12) break;
        }
        std::cout  << YELLOW_COLOR << "Invalid month format or value." << RESET_COLOR << " Please use YYYYMM." << std::endl;
    }
    return month_str;
}

ReportFormat get_report_format_from_user() {
    std::cout << "Select report format (1: Markdown, 2: TeX) [Default: 1]: ";
    std::string line;
    if (std::getline(std::cin, line) && (line == "2" || line == "tex")) {
        std::cout << "-> TeX format selected.\n";
        return ReportFormat::LaTeX;
    }
    std::cout << "-> Markdown format selected.\n";
    return ReportFormat::Markdown;
}

std::vector<int> get_integer_list_input(const std::string& prompt_message) {
    std::cout << prompt_message;
    std::string days_str;
    std::vector<int> days_list;

    if (!std::getline(std::cin, days_str) || days_str.empty()) {
        return days_list;
    }

    std::string token;
    std::istringstream tokenStream(days_str);
    while (std::getline(tokenStream, token, ',')) {
        try {
            days_list.push_back(std::stoi(token));
        } catch (const std::exception&) {
            std::cerr << RED_COLOR << "Invalid number '" << token << "' skipped." << RESET_COLOR << std::endl;
        }
    }
    return days_list;
}

} // namespace UserInputUtils