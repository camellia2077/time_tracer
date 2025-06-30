#include "Menu.h"
#include "processing.h"
#include "query_handler.h"
#include "version.h"

#include <iostream>
#include <limits>
#include <sqlite3.h>
#include <algorithm>


// 【修改】构造函数，初始化所有成员变量
Menu::Menu(const std::string& db_name, const std::string& config_path) 
    : db(nullptr), db_name_(db_name), config_path_(config_path) {}

// Main application loop
void Menu::run() {
    while (true) {
        print_menu();
        int choice = -1;
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cout << "Invalid input. Please enter a number." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Consume newline

        if (!handle_user_choice(choice)) {
            break; 
        }
    }
    close_database();
}

void Menu::print_menu() {
    std::cout << "\n--- Time Tracking Menu ---" << std::endl;
    std::cout << "0. Process file(s) and import data" << std::endl;
    std::cout << "1. Query daily statistics" << std::endl;
    std::cout << "2. Query last 7 days" << std::endl;
    std::cout << "3. Query last 14 days" << std::endl;
    std::cout << "4. Query last 30 days" << std::endl;
    std::cout << "5. Generate study heatmap for a year" << std::endl;
    std::cout << "6. Query monthly statistics" << std::endl;
    std::cout << "7. --version" << std::endl;
    std::cout << "8. Exit" << std::endl;
    std::cout << "Enter your choice: ";
}

bool Menu::handle_user_choice(int choice) {
    if (choice >= 1 && choice <= 6) {
        if (!open_database_if_needed()) {
            return true;
        }
    }

    QueryHandler query_handler(db);

    switch (choice) {
        case 0:
            process_files_option();
            break;
        case 1: {
            std::string date_str = get_valid_date_input();
            query_handler.run_daily_query(date_str);
            break;
        }
        case 2: query_handler.run_period_query(7); break;
        case 3: query_handler.run_period_query(14); break;
        case 4: query_handler.run_period_query(30); break;
        case 5: std::cout << "\nFeature 'Generate study heatmap for a year' is not yet implemented." << std::endl; break;
        case 6: {
            std::string month_str = get_valid_month_input();
            query_handler.run_monthly_query(month_str);
            break;
        }
        case 7: {
            std::cout << "time_tracker_command Version: " << AppInfo::VERSION << std::endl;
            std::cout << "Last Updated: " << AppInfo::LAST_UPDATED << std::endl;
            break;
        }
        case 8:
            std::cout << "Exiting program." << std::endl;
            return false;
        default:
            std::cout << "Invalid choice. Please try again." << std::endl;
            break;
    }
    return true;
}

void Menu::process_files_option() {
    close_database(); // 在写入前关闭数据库连接
    // 【核心修复】调用 handle_process_files 时，传入存储的 config_path_
    handle_process_files(db_name_, config_path_);
}

// --- 其他私有方法 (保持不变) ---
bool Menu::open_database_if_needed() {
    if (db == nullptr) {
        if (sqlite3_open(db_name_.c_str(), &db)) {
            std::cerr << "Can't open database " << db_name_ << ": " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            db = nullptr;
            return false;
        }
    }
    return true;
}

void Menu::close_database() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

std::string Menu::get_valid_date_input() {
    std::string date_str;
    while (true) {
        std::cout << "Enter date (YYYYMMDD): ";
        std::cin >> date_str;
        if (date_str.length() == 8 && std::all_of(date_str.begin(), date_str.end(), ::isdigit)) {
            int year = std::stoi(date_str.substr(0, 4));
            int month = std::stoi(date_str.substr(4, 2));
            int day = std::stoi(date_str.substr(6, 2));
            if (year > 1900 && year < 3000 && month >= 1 && month <= 12 && day >= 1 && day <= 31) break;
        }
        std::cout << "Invalid date format or value. Please use YYYYMMDD." << std::endl;
    }
    return date_str;
}

std::string Menu::get_valid_month_input() {
    std::string month_str;
    while (true) {
        std::cout << "Enter month (YYYYMM): ";
        std::cin >> month_str;
        if (month_str.length() == 6 && std::all_of(month_str.begin(), month_str.end(), ::isdigit)) {
            int year = std::stoi(month_str.substr(0, 4));
            int month = std::stoi(month_str.substr(4, 2));
            if (year > 1900 && year < 3000 && month >= 1 && month <= 12) break;
        }
        std::cout << "Invalid month format or value. Please use YYYYMM." << std::endl;
    }
    return month_str;
}