// --- START OF FILE menu.cpp --- (CORRECTED AND COMPLETE)

#include "Menu.h"
#include "processing.h"
#include "query_handler.h"
#include "version.h"
#include "LogProcessor.h"

#include "FormatValidator.h"
#include "IntervalProcessor.h"
#include "common_utils.h" // 获取颜色

#include <iostream>
#include <limits>
#include <sqlite3.h>
#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>
#include <set>    // 用于 std::set
#include <map>    // 用于 std::map
#include <ctime>  // 用于 std::time and std::localtime

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
        if (!std::getline(std::cin, line)) {
            if (std::cin.eof()) {
                break;
            }
            std::cin.clear();
            continue; 
        }

        try {
            choice = std::stoi(line); 
        } catch (const std::invalid_argument&) {
            std::cout << YELLOW_COLOR << "Invalid input." << RESET_COLOR << " Please enter a number." << std::endl;
            continue;
        } catch (const std::out_of_range&) {
            std::cout <<  YELLOW_COLOR << "Input out of range. " << RESET_COLOR << "Please enter a valid number." << std::endl;
            continue;
        }
    
        if (!handle_user_choice(choice)) {
            break; 
        }
    }
    close_database();
}
    
void Menu::print_menu() {
    std::cout << "\n--- Time Tracking Menu ---" << std::endl;
    std::cout << "0. File Processing & Validation (Submenu)" << std::endl;
    std::cout << "1. Query daily statistics" << std::endl;
    std::cout << "2. Query last 7 days" << std::endl;
    std::cout << "3. Query last 14 days" << std::endl;
    std::cout << "4. Query last 30 days" << std::endl;
    std::cout << "5. Full Pipeline (Validate -> Convert -> Validate -> Import)" << std::endl;
    std::cout << "6. Generate study heatmap for a year" << std::endl;
    std::cout << "7. Query monthly statistics" << std::endl;
    std::cout << "8. --version" << std::endl;
    std::cout << "9. Exit" << std::endl;
    std::cout << "Enter your choice: ";
}

bool Menu::handle_user_choice(int choice) {
    // 【核心修复】为 '&&' 和 '||' 添加括号以消除警告
    if ((choice >= 1 && choice <= 4) || choice == 7) { 
        if (!open_database_if_needed()) {
            return true;
        }
        
        QueryHandler query_handler(db);
        switch (choice) {
            case 1: { std::string date_str = get_valid_date_input(); query_handler.run_daily_query(date_str); break; }
            case 2: query_handler.run_period_query(7); break;
            case 3: query_handler.run_period_query(14); break;
            case 4: query_handler.run_period_query(30); break;
            case 7: { std::string month_str = get_valid_month_input(); query_handler.run_monthly_query(month_str); break; }
        }
    } else {
        switch (choice) {
            case 0: run_log_processor_submenu(); break;
            case 5: run_full_pipeline_and_import(); break;
            case 6: std::cout << "\nFeature 'Generate study heatmap for a year' is not yet implemented." << std::endl; break; 
            case 8: { std::cout << "time_tracker_command Version: " << AppInfo::VERSION << std::endl; std::cout << "Last Updated: " << AppInfo::LAST_UPDATED << std::endl; break; }
            case 9: std::cout << "Exiting program." << std::endl; return false; 
            default: std::cout << YELLOW_COLOR << "Invalid choice. " << RESET_COLOR <<"Please try again." << std::endl; break;
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

        if (choice == 4) {
             std::cout << "Enter the path to the PROCESSED file or directory to validate: ";
        } else {
             std::cout << "Enter the path to the SOURCE file or directory to process: ";
        }
        std::getline(std::cin, path); 
        options.input_path = path;
        
        options.validate_source = false;
        options.convert = false;
        options.validate_output = false;
        options.run_all = false;
        options.enable_day_count_check = false;

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

void Menu::run_full_pipeline_and_import() {
    std::cout << "\n--- Full Pipeline (Validate -> Convert -> Validate -> Import) ---\n";
    std::cout << "This process will stop on the first error. Only if all steps succeed for all files, will the data be imported.\n";
    std::cout << "Enter the path to the SOURCE directory to process: ";

    std::string path_str;
    std::getline(std::cin, path_str);
    fs::path input_path(path_str);

    if (!fs::exists(input_path) || !fs::is_directory(input_path)) {
        std::cerr << RED_COLOR << "Error: " << RESET_COLOR << "Path does not exist or is not a directory. Aborting." << std::endl;
        return;
    }

    std::vector<fs::path> files_to_process;
    for (const auto& entry : fs::recursive_directory_iterator(input_path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            files_to_process.push_back(entry.path());
        }
    }
    if (files_to_process.empty()) {
        std::cout << YELLOW_COLOR << "Warning: " << RESET_COLOR << "No .txt files found in the specified directory." << std::endl;
        return;
    }
    std::sort(files_to_process.begin(), files_to_process.end());

    FormatValidator validator(app_config_.format_validator_config_path, app_config_.interval_processor_config_path, true);
    IntervalProcessor processor(app_config_.interval_processor_config_path);
    
    fs::path output_root_path = input_path.parent_path() / ("Processed_" + input_path.filename().string());
    fs::create_directories(output_root_path);
    
    std::cout << "\n--- Phase 1: Validating all source files... ---\n";
    for (const auto& file : files_to_process) {
        std::set<FormatValidator::Error> errors;
        if (!validator.validateSourceFile(file.string(), errors)) {
            std::cerr << RED_COLOR << "Error: Source file validation failed for " << file.string() << "." << RESET_COLOR << std::endl;
            FormatValidator::printGroupedErrors(file.string(), errors, app_config_.error_log_path);
            std::cerr << "Pipeline aborted. No files will be imported." << std::endl;
            return;
        }
    }
    std::cout << GREEN_COLOR << "Success: All source files are valid." << RESET_COLOR << std::endl;

    std::cout << "\n--- Phase 2: Converting all files... ---\n";
    std::map<fs::path, fs::path> source_to_output_map;
    for (const auto& file : files_to_process) {
        fs::path target_path = output_root_path / fs::relative(file, input_path);
        fs::create_directories(target_path.parent_path());
        
        std::string year_str;
        fs::path current_path = file.parent_path();
        auto is_four_digit_string = [](const std::string& s) { return s.length() == 4 && std::all_of(s.begin(), s.end(), ::isdigit); };
        while (!current_path.empty() && current_path.has_filename()) {
            if (is_four_digit_string(current_path.filename().string())) {
                year_str = current_path.filename().string();
                break;
            }
            current_path = current_path.parent_path();
        }
        if (year_str.empty()) {
             std::time_t now = std::time(nullptr);
             std::tm* ltm = std::localtime(&now);
             year_str = std::to_string(1900 + ltm->tm_year);
        }

        if (!processor.executeConversion(file.string(), target_path.string(), year_str)) {
            std::cerr << RED_COLOR << "Error: Conversion failed for " << file.string() << "." << RESET_COLOR << std::endl;
            std::cerr << "Pipeline aborted. No files will be imported." << std::endl;
            return;
        }
        source_to_output_map[file] = target_path;
        std::cout << "Converted " << file.filename().string() << " -> " << target_path.filename().string() << std::endl;
    }
    std::cout << GREEN_COLOR << "Success: All files converted." << RESET_COLOR << std::endl;

    std::cout << "\n--- Phase 3: Validating all output files... ---\n";
    for (const auto& pair : source_to_output_map) {
        const fs::path& output_file = pair.second;
        std::set<FormatValidator::Error> errors;
        if (!validator.validateOutputFile(output_file.string(), errors)) {
            std::cerr << RED_COLOR << "Error: Output file validation failed for " << output_file.string() << "." << RESET_COLOR << std::endl;
            FormatValidator::printGroupedErrors(output_file.string(), errors, app_config_.error_log_path);
            std::cerr << "Pipeline aborted. No files will be imported." << std::endl;
            return;
        }
    }
    std::cout << GREEN_COLOR << "Success: All output files are valid." << RESET_COLOR << std::endl;

    std::cout << "\n--- Phase 4: Importing all processed files into database... ---\n";
    close_database();
    handle_process_files(db_name_, output_root_path.string(), main_config_path_);
    std::cout << GREEN_COLOR << "Success: Full pipeline completed and data imported." << RESET_COLOR << std::endl;
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