// time_master_app/menu.cpp
#include "common/pch.h"
#include "Menu.h"
#include "action_handler/ActionHandler.h"
#include "common/version.h"
#include "common/common_utils.h"
#include "time_master_app/menu_processor/LogProcessorMenu.h"
#include "time_master_app/menu_input/UserInputUtils.h"

#include <iostream>
#include <string>
#include <vector>

Menu::Menu(const std::string& db_name, const AppConfig& config, const std::string& main_config_path) {
    action_handler_ = new ActionHandler(db_name, config, main_config_path);
}

Menu::~Menu() {
    delete action_handler_;
}

void Menu::run() {
    while (true) {
        print_menu();
        int choice = -1;
        std::string line;

        if (!std::getline(std::cin, line) || line.empty()) {
            if (std::cin.eof()) { std::cout << "\nExiting due to End-of-File." << std::endl; break; }
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
    std::cout << "--- Query ---" << std::endl;
    std::cout << " 1. Query daily statistics" << std::endl;
    std::cout << " 2. Query Period Statistics" << std::endl;
    std::cout << " 3. Query monthly statistics" << std::endl;
    std::cout << "--- Export (Single Report) ---" << std::endl;
    std::cout << " 4. Export single DAY report" << std::endl;
    std::cout << " 5. Export single MONTH report" << std::endl;
    std::cout << " 6. Export single PERIOD report" << std::endl;
    std::cout << "--- Export (Bulk) ---" << std::endl;
    std::cout << " 7. Export ALL daily reports" << std::endl;
    std::cout << " 8. Export ALL monthly reports" << std::endl;
    std::cout << " 9. Export ALL period reports" << std::endl;
    std::cout << "--- Other ---" << std::endl;
    std::cout << " 10. File Processing & Validation (Submenu)" << std::endl;
    std::cout << " 11. Full Pipeline (Validate -> Convert -> Import)" << std::endl;
    std::cout << " 12. Show Version" << std::endl;
    std::cout << " 13. Exit" << std::endl;
    std::cout << "Enter your choice: ";
}

bool Menu::handle_user_choice(int choice) {
    switch (choice) {
        case 1: { // Query Daily
            std::string date = UserInputUtils::get_valid_date_input();
            if (!date.empty()) {
                ReportFormat format = UserInputUtils::get_report_format_from_user();
                std::cout << action_handler_->run_daily_query(date, format);
            }
            break;
        }
        case 2: run_period_query_prompt(); break; // Query Period
        case 3: { // Query Monthly
            std::string month = UserInputUtils::get_valid_month_input();
            if (!month.empty()) {
                ReportFormat format = UserInputUtils::get_report_format_from_user();
                std::cout << action_handler_->run_monthly_query(month, format);
            }
            break;
        }
        case 4: run_export_single_day_report_prompt(); break;    // NEW
        case 5: run_export_single_month_report_prompt(); break;   // NEW
        case 6: run_export_single_period_report_prompt(); break;  // NEW
        case 7: { // Export ALL Daily
            ReportFormat format = UserInputUtils::get_report_format_from_user();
            action_handler_->run_export_all_daily_reports_query(format);
            break;
        }
        case 8: { // Export ALL Monthly
            ReportFormat format = UserInputUtils::get_report_format_from_user();
            action_handler_->run_export_all_monthly_reports_query(format);
            break;
        }
        case 9: run_export_all_period_reports_prompt(); break; // Export ALL Period
        case 10: run_log_processor_submenu(); break;
        case 11: run_full_pipeline_and_import_prompt(); break;
        case 12:
            std::cout << "TimeMaster Version: " << AppInfo::VERSION << " (Last Updated: " << AppInfo::LAST_UPDATED << ")" << std::endl;
            break;
        case 13:
            std::cout << "Exiting program." << std::endl;
            return false;
        default:
            std::cout << YELLOW_COLOR << "Invalid choice. Please try again." << RESET_COLOR << std::endl;
            break;
    }
    std::cout << "\nPress Enter to continue...";
    std::string dummy;
    std::getline(std::cin, dummy);
    return true;
}

// =========================================================================
//                      新的单个报告导出函数实现
// =========================================================================

void Menu::run_export_single_day_report_prompt() {
    std::string date = UserInputUtils::get_valid_date_input();
    if (!date.empty()) {
        ReportFormat format = UserInputUtils::get_report_format_from_user();
        action_handler_->run_export_single_day_report(date, format);
    }
}

void Menu::run_export_single_month_report_prompt() {
    std::string month = UserInputUtils::get_valid_month_input();
    if (!month.empty()) {
        ReportFormat format = UserInputUtils::get_report_format_from_user();
        action_handler_->run_export_single_month_report(month, format);
    }
}

void Menu::run_export_single_period_report_prompt() {
    std::vector<int> days_list = UserInputUtils::get_integer_list_input("Enter period days to export (e.g., 7 or 7,30,90): ");
    if (days_list.empty()) {
        std::cout << YELLOW_COLOR << "No valid days provided for export." << RESET_COLOR << std::endl;
        return;
    }
    
    ReportFormat format = UserInputUtils::get_report_format_from_user();
    for (int days : days_list) {
        action_handler_->run_export_single_period_report(days, format);
    }
}


// --- 其他辅助函数 (保持不变) ---

void Menu::run_period_query_prompt() {
    std::vector<int> days_list = UserInputUtils::get_integer_list_input("Enter period days (e.g., 7 or 7,30,90): ");
    if (days_list.empty()) return;

    ReportFormat format = UserInputUtils::get_report_format_from_user();

    for (int days : days_list) {
        std::cout << "\n--- Report for last " << days << " days ---\n";
        std::cout << action_handler_->run_period_query(days, format);
    }
}

void Menu::run_export_all_period_reports_prompt() {
    std::vector<int> days_list = UserInputUtils::get_integer_list_input("Enter period days to export (e.g., 7 or 7,30,90): ");

    if (!days_list.empty()) {
        ReportFormat format = UserInputUtils::get_report_format_from_user();
        action_handler_->run_export_all_period_reports_query(days_list, format);
    } else {
        std::cout << YELLOW_COLOR << "No valid days provided for export." << RESET_COLOR << std::endl;
    }
}

void Menu::run_log_processor_submenu() {
    LogProcessorMenu submenu(action_handler_);
    submenu.run();
}

void Menu::run_full_pipeline_and_import_prompt() {
    std::string path_str = UserInputUtils::get_valid_path_input("Enter the path to the SOURCE directory to process: ");
    if (!path_str.empty()) {
        action_handler_->run_full_pipeline_and_import(path_str);
    }
}