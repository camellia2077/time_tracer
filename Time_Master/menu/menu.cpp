、#include "Menu.h"
#include "processing.h"
#include "query_handler.h"
#include "version.h"
#include "LogProcessor.h"     
#include <iostream>
#include <limits>
#include <sqlite3.h>
#include <algorithm>
#include <filesystem>
#include <string>

#include "common_utils.h" // 获取颜色


namespace fs = std::filesystem;

Menu::Menu(const std::string& db_name, const AppConfig& config, const std::string& main_config_path) 
    : db(nullptr), 
      db_name_(db_name), 
      app_config_(config),
      main_config_path_(main_config_path) {}


void Menu::run() {
    while (true) {
        print_menu();
        int choice = -1;

        std::string line;
        if (!std::getline(std::cin, line)) { // 只检查读取是否成功
            if (std::cin.eof()) {
                break; // 如果是文件结束符(Ctrl+D/Z)，则退出
            }
            std::cin.clear();
            continue; 
        }

        try {
            // 如果 line 是空字符串，std::stoi 会抛出 std::invalid_argument 异常
            choice = std::stoi(line); 
        } catch (const std::invalid_argument&) {
            // 空行和 "abc" 都会进入这个 catch 块
            std::cout << YELLOW_COLOR << "Invalid input." << RESET_COLOR << " Please enter a number." << std::endl;
            continue; // 不是有效数字，继续循环
        } catch (const std::out_of_range&) {
            std::cout <<  YELLOW_COLOR << "Input out of range. " << RESET_COLOR << "Please enter a valid number." << std::endl;
            continue; // 数字超出范围，继续循环
        }
    
        // 加入对 handle_user_choice 的调用
        if (!handle_user_choice(choice)) {
            break; 
        }
}
close_database();
}
    
void Menu::print_menu() {
    // ... 此函数保持不变 ...
    std::cout << "\n--- Time Tracking Menu ---" << std::endl;
    std::cout << "0. File Processing & Validation (Submenu)" << std::endl;
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
        QueryHandler query_handler(db);
        switch (choice) {
            case 1: { std::string date_str = get_valid_date_input(); query_handler.run_daily_query(date_str); break; }
            case 2: query_handler.run_period_query(7); break;
            case 3: query_handler.run_period_query(14); break;
            case 4: query_handler.run_period_query(30); break;
            case 6: { std::string month_str = get_valid_month_input(); query_handler.run_monthly_query(month_str); break; }
        }
    } else {
        switch (choice) {
            case 0: run_log_processor_submenu(); break;
            case 5: std::cout << "\nFeature 'Generate study heatmap for a year' is not yet implemented." << std::endl; break;
            case 7: { std::cout << "time_tracker_command Version: " << AppInfo::VERSION << std::endl; std::cout << "Last Updated: " << AppInfo::LAST_UPDATED << std::endl; break; }
            case 8: std::cout << "Exiting program." << std::endl; return false;
            default: std::cout << "Invalid choice. Please try again." << std::endl; break;
        }
    }
    return true;
}

void Menu::run_log_processor_submenu() {
    AppOptions options;
    std::string path;

    while (true) {
        std::cout << "\n--- File Processing & Validation Submenu ---\n";
        std::cout << "--- (Step 1: File Operations) ---\n";
        std::cout << "1. Validate source file(s) only\n";
        std::cout << "2. Convert source file(s) only\n";
        std::cout << "3. Validate source, then Convert\n";
        std::cout << "4. Validate processed file(s) only\n";
        std::cout << "5. Full Pipeline (Validate Source -> Convert -> Validate Output)\n";
        std::cout << "6. Full Pipeline - All-in-one (stops on first error)\n";
        std::cout << "--- (Step 2: Database Operations) ---\n";
        std::cout << "7. Import processed files into database\n";
        std::cout << "8. Back to main menu\n";
        std::cout << "Enter your choice: ";

        int choice = -1;
        std::string line;
        if (!std::getline(std::cin, line) || line.empty()) {
            if (std::cin.eof()) break;
            std::cin.clear();
            continue;
        }
        try {
            choice = std::stoi(line);
        } catch (const std::exception&) {
            std::cout << YELLOW_COLOR <<  "Invalid input. " << RESET_COLOR << "Please enter a number." << std::endl;
            continue;
        }
        
        if (choice == 8) break;

        if (choice < 1 || choice > 7) {
            std::cout  << YELLOW_COLOR << "Invalid choice. " << RESET_COLOR <<"Please try again.\n";
            continue;
        }

        // 处理数据库导入
        if (choice == 7) {
            std::cout << "Enter the path to the DIRECTORY containing processed files (e.g., 'Processed_MyLogs'): ";
            std::getline(std::cin, path); 
            if (!fs::exists(path) || !fs::is_directory(path)) {
                std::cerr << RED_COLOR << "Error: " <<  RESET_COLOR << "Path does not exist or is not a directory. Aborting import." << std::endl;
                continue;
            }
            close_database(); 
            std::cout << "Starting import process..." << std::endl;
            handle_process_files(db_name_, path, main_config_path_); 
            std::cout << "Import process finished." << std::endl;
            continue;
        }

        // 处理文件操作
        if (choice == 4) {
             std::cout << "Enter the path to the PROCESSED file or directory to validate: ";
        } else {
             std::cout << "Enter the path to the SOURCE file or directory to process: ";
        }
        std::getline(std::cin, path); 
        options.input_path = path;
        
        // 重置所有标志
        options.validate_source = false;
        options.convert = false;
        options.validate_output = false;
        options.run_all = false;
        options.enable_day_count_check = false;

        // 根据选择设置标志
        switch (choice) {
            case 1: options.validate_source = true; break;
            case 2: options.convert = true; break;
            case 3: options.validate_source = true; options.convert = true; break;
            case 4: options.validate_output = true; break;
            case 5: options.validate_source = true; options.convert = true; options.validate_output = true; break;
            case 6: options.run_all = true; options.validate_source = true; options.convert = true; options.validate_output = true; break;
        }
        
        if (options.validate_output) {
            std::cout << "Enable strict day count check for output files? [Y/n]: ";
            std::string user_input;
            std::getline(std::cin, user_input);
            options.enable_day_count_check = (user_input.empty() || (user_input[0] != 'n' && user_input[0] != 'N'));
        }
        
        close_database(); 
        
        LogProcessor processor(app_config_);
        processor.run(options);

    }
}

bool Menu::open_database_if_needed() {
    if (db == nullptr) {
        if (sqlite3_open(db_name_.c_str(), &db)) {
            std::cerr << RED_COLOR << "Error: "<< RESET_COLOR << "Can't open database " << db_name_ << ": " << sqlite3_errmsg(db) << std::endl;
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
        // 在读取后立即清除缓冲区，为返回主菜单后的 getline 做准备
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 
        if (date_str.length() == 8 && std::all_of(date_str.begin(), date_str.end(), ::isdigit)) {
            int year = std::stoi(date_str.substr(0, 4));
            int month = std::stoi(date_str.substr(4, 2));
            int day = std::stoi(date_str.substr(6, 2));
            if (year > 1900 && year < 3000 && month >= 1 && month <= 12 && day >= 1 && day <= 31) break;
        }
        std::cout << YELLOW_COLOR << "Invalid date format or value. "<< RESET_COLOR << "Please use YYYYMMDD." << std::endl;
    }
    return date_str;
}

std::string Menu::get_valid_month_input() {
    std::string month_str;
    while (true) {
        std::cout << "Enter month (YYYYMM): ";
        std::cin >> month_str;
        // 在读取后立即清除缓冲区，为返回主菜单后的 getline 做准备
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (month_str.length() == 6 && std::all_of(month_str.begin(), month_str.end(), ::isdigit)) {
            int year = std::stoi(month_str.substr(0, 4));
            int month = std::stoi(month_str.substr(4, 2));
            if (year > 1900 && year < 3000 && month >= 1 && month <= 12) break;
        }
        std::cout  << YELLOW_COLOR << "Invalid month format or value." << RESET_COLOR << " Please use YYYYMM." << std::endl;
    }
    return month_str;
}