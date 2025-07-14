#include "Menu.h"
#include "ActionHandler.h"
#include "version.h"
#include "common_utils.h"

#include <iostream>
#include <limits>
#include <string>
#include <filesystem>
#include <sstream>
#include <vector>

namespace fs = std::filesystem;

// Menu 构造函数: 创建 ActionHandler 实例
Menu::Menu(const std::string& db_name, const AppConfig& config, const std::string& main_config_path) {
    action_handler_ = new ActionHandler(db_name, config, main_config_path);
}

// Menu 析构函数: 释放 ActionHandler 实例
Menu::~Menu() {
    delete action_handler_;
}

void Menu::run() {
    while (true) {
        print_menu();
        int choice = -1;
        std::string line;

        if (!std::getline(std::cin, line) || line.empty()) {
            if (std::cin.eof()) {
                std::cout << "\nExiting due to End-of-File." << std::endl;
                break;
            }
            std::cin.clear();
            continue;
        }

        try {
            choice = std::stoi(line);
        } catch (const std::exception&) {
            std::cout << YELLOW_COLOR << "Invalid input. Please enter a number." << RESET_COLOR << std::endl;
            continue;
        }

        if (!handle_user_choice(choice)) {
            break;
        }
    }
}

void Menu::print_menu() {
    std::cout << "\n" << "--- Time Tracking Menu ---"  << std::endl;
    std::cout << "0. File Processing & Validation (Submenu)" << std::endl;
    std::cout << "1. Query daily statistics" << std::endl;
    std::cout << "2. Query Period Statistics" << std::endl; 
    std::cout << "3. Query monthly statistics" << std::endl; 
    std::cout << "4. Full Pipeline (Validate -> Convert -> Validate -> Import)" << std::endl; 
    std::cout << "5. Generate study heatmap for a year (Not Implemented)" << std::endl;
    std::cout << "6. Export all DAILY reports to .md files" << std::endl; 
    std::cout << "7. Export all MONTHLY reports to .md files" << std::endl; 
    std::cout << "8. Export PERIOD reports to .md files" << std::endl; 
    std::cout << "9. Show Version" << std::endl; 
    std::cout << "10. Exit" << std::endl; 
    std::cout << "Enter your choice: ";
}

bool Menu::handle_user_choice(int choice) {
    switch (choice) {
        case 0: run_log_processor_submenu(); break;
        case 1:
            {
                std::string date = get_valid_date_input();
                if (!date.empty()) {
                    std::cout << action_handler_->run_daily_query(date);
                }
            }
            break;
        case 2: run_period_query_prompt(); break;
        case 3: 
             {
                std::string month = get_valid_month_input();
                if (!month.empty()) {
                    std::cout << action_handler_->run_monthly_query(month);
                }
            }
            break;
        case 4: run_full_pipeline_and_import_prompt(); break; 
        case 5: std::cout << "\nFeature 'Generate study heatmap for a year' is not yet implemented." << std::endl; break; // [修改]
        case 6: 
            action_handler_->run_export_all_daily_reports_query();
            break;
        case 7: 
            action_handler_->run_export_all_monthly_reports_query();
            break;
        case 8: 
            run_export_period_reports_prompt();
            break;
        case 9: 
            std::cout << "TimeMaster Version: " << AppInfo::VERSION << " (Last Updated: " << AppInfo::LAST_UPDATED << ")" << std::endl; 
            break;
        case 10:
            std::cout << "Exiting program." << std::endl; 
            return false;
        default: 
            std::cout << YELLOW_COLOR << "Invalid choice. Please try again." << RESET_COLOR << std::endl; 
            break;
    }
    // 添加一个暂停，以便用户可以看到操作结果
    std::cout << "\nPress Enter to continue...";
    // 清空输入缓冲区，以防之前的输入影响 cin.get()
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return true;
}

// [新增] 实现周期查询的用户提示
void Menu::run_period_query_prompt() {
    std::cout << "Enter period days (e.g., 7 or 7,30,90): ";
    std::string days_str;
    if (!std::getline(std::cin, days_str) || days_str.empty()) {
        return;
    }

    std::string token;
    std::istringstream tokenStream(days_str);
    while (std::getline(tokenStream, token, ',')) {
        try {
            int days = std::stoi(token);
            std::cout << "\n--- Report for last " << days << " days ---\n";
            std::cout << action_handler_->run_period_query(days);
        } catch (const std::exception&) {
            std::cerr << RED_COLOR << "Invalid number '" << token << "' skipped." << RESET_COLOR << std::endl;
        }
    }
}

// [新增] 实现导出周期报告的用户提示
void Menu::run_export_period_reports_prompt() {
    std::cout << "Enter period days to export (e.g., 7 or 7,30,90): ";
    std::string days_str;
    if (!std::getline(std::cin, days_str) || days_str.empty()) {
        return;
    }

    std::vector<int> days_list;
    std::string token;
    std::istringstream tokenStream(days_str);
    while (std::getline(tokenStream, token, ',')) {
        try {
            days_list.push_back(std::stoi(token));
        } catch (const std::exception&) {
            std::cerr << RED_COLOR << "Invalid number '" << token << "' skipped." << RESET_COLOR << std::endl;
        }
    }

    if (!days_list.empty()) {
        action_handler_->run_export_all_period_reports_query(days_list);
    } else {
        std::cout << YELLOW_COLOR << "No valid days provided for export." << RESET_COLOR << std::endl;
    }
}


void Menu::run_log_processor_submenu() {
    while (true) {
        std::cout << "\n--- File Processing & Validation Submenu ---\n";
        std::cout << "--- (Step 1: File Operations) ---\n";
        std::cout << "1. Validate source file(s) only\n";
        std::cout << "2. Convert source file(s) only\n";
        std::cout << "3. Validate source, then Convert\n";
        std::cout << "4. Convert, then Validate Output\n";
        std::cout << "5. Full Pre-processing (Validate Source -> Convert -> Validate Output)\n";
        std::cout << "--- (Step 2: Database Operations) ---\n";
        std::cout << "7. Import processed files into database\n";
        std::cout << "8. Back to main menu\n";
        std::cout << "Enter your choice: ";

        int choice = -1;
        std::string line;
        if (!std::getline(std::cin, line) || line.empty()) { continue; }
        try { choice = std::stoi(line); } catch (const std::exception&) {
            std::cout << YELLOW_COLOR << "Invalid input. Please enter a number." << RESET_COLOR << std::endl;
            continue;
        }

        if (choice == 8) break;
        if ((choice < 1 || choice > 5) && choice != 7) {
            std::cout << YELLOW_COLOR << "Invalid choice. Please try again.\n" << RESET_COLOR;
            continue;
        }

        if (choice == 7) {
            std::string path = get_valid_path_input("Enter the path to the DIRECTORY containing processed files: ");
            if (path.empty()) continue;
            action_handler_->run_database_import(path);
        } else {
            std::string path = get_valid_path_input("Enter the path to the SOURCE file or directory to process: ");
            if (path.empty()) continue;

            if (!action_handler_->collectFiles(path)) {
                std::cout << RED_COLOR << "Failed to collect files. Please check the path and try again." << RESET_COLOR << std::endl;
                continue;
            }

            switch (choice) {
                case 1:
                    action_handler_->validateSourceFiles();
                    break;
                case 2:
                    action_handler_->convertFiles();
                    break;
                case 3:
                    if (action_handler_->validateSourceFiles()) {
                        action_handler_->convertFiles();
                    }
                    break;
                case 4:
                    if (action_handler_->convertFiles()) {
                        action_handler_->validateOutputFiles(false);
                    }
                    break;
                case 5:
                    if (action_handler_->validateSourceFiles()) {
                        if (action_handler_->convertFiles()) {
                           action_handler_->validateOutputFiles(false);
                        }
                    }
                    break;
            }
        }
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

void Menu::run_full_pipeline_and_import_prompt() {
    std::string path_str = get_valid_path_input("Enter the path to the SOURCE directory to process: ");
    if (!path_str.empty()) {
        action_handler_->run_full_pipeline_and_import(path_str);
    }
}

std::string Menu::get_valid_path_input(const std::string& prompt_message) {
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

std::string Menu::get_valid_date_input() {
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

std::string Menu::get_valid_month_input() {
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