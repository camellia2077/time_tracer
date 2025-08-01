// time_master_app/menu.cpp
#include "common/pch.h"
#include "Menu.h"
#include "action_handler/ActionHandler.h"
#include "common/version.h"
#include "common/common_utils.h"
#include "time_master_app/menu_processor/LogProcessorMenu.h"           // [新增] 包含新的子菜单头文件
#include "time_master_app/menu_input/UserInputUtils.h"      // [新增] 包含新的输入工具头文件


#include <iostream>
#include <string>
#include <vector>

// Menu 构造函数和析构函数保持不变
Menu::Menu(const std::string& db_name, const AppConfig& config, const std::string& main_config_path) {
    action_handler_ = new ActionHandler(db_name, config, main_config_path);
}

Menu::~Menu() {
    delete action_handler_;
}

// [移除] get_report_format_from_user 函数，已移至 UserInputUtils

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

// print_menu 保持不变
void Menu::print_menu() {
    std::cout << "\n" << "--- Time Tracking Menu ---"  << std::endl;
    std::cout << "0. File Processing & Validation (Submenu)" << std::endl;
    std::cout << "1. Query daily statistics" << std::endl;
    std::cout << "2. Query Period Statistics" << std::endl;
    std::cout << "3. Query monthly statistics" << std::endl;
    std::cout << "4. Full Pipeline (Validate -> Convert -> Validate -> Import)" << std::endl;
    std::cout << "5. Generate study heatmap for a year (Not Implemented)" << std::endl;
    std::cout << "6. Export all DAILY reports" << std::endl;
    std::cout << "7. Export all MONTHLY reports" << std::endl;
    std::cout << "8. Export PERIOD reports" << std::endl;
    std::cout << "9. Show Version" << std::endl;
    std::cout << "10. Exit" << std::endl;
    std::cout << "Enter your choice: ";
}

// [修改] handle_user_choice 现在调用 UserInputUtils
bool Menu::handle_user_choice(int choice) {
    switch (choice) {
        case 0: run_log_processor_submenu(); break;
        case 1: {
            std::string date = UserInputUtils::get_valid_date_input();
            if (!date.empty()) {
                ReportFormat format = UserInputUtils::get_report_format_from_user();
                std::cout << action_handler_->run_daily_query(date, format);
            }
            break;
        }
        case 2: run_period_query_prompt(); break;
        case 3: {
            std::string month = UserInputUtils::get_valid_month_input();
            if (!month.empty()) {
                ReportFormat format = UserInputUtils::get_report_format_from_user();
                std::cout << action_handler_->run_monthly_query(month, format);
            }
            break;
        }
        case 4: run_full_pipeline_and_import_prompt(); break;
        case 5: std::cout << "\nFeature 'Generate study heatmap for a year' is not yet implemented." << std::endl; break;
        case 6: {
            ReportFormat format = UserInputUtils::get_report_format_from_user();
            action_handler_->run_export_all_daily_reports_query(format);
            break;
        }
        case 7: {
            ReportFormat format = UserInputUtils::get_report_format_from_user();
            action_handler_->run_export_all_monthly_reports_query(format);
            break;
        }
        case 8: run_export_period_reports_prompt(); break;
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
    std::cout << "\nPress Enter to continue...";
    std::string dummy;
    std::getline(std::cin, dummy);
    return true;
}

// [修改] 简化函数，使用 UserInputUtils
void Menu::run_period_query_prompt() {
    std::vector<int> days_list = UserInputUtils::get_integer_list_input("Enter period days (e.g., 7 or 7,30,90): ");
    if (days_list.empty()) return;

    ReportFormat format = UserInputUtils::get_report_format_from_user();

    for (int days : days_list) {
        std::cout << "\n--- Report for last " << days << " days ---\n";
        std::cout << action_handler_->run_period_query(days, format);
    }
}

// [修改] 简化函数，使用 UserInputUtils
void Menu::run_export_period_reports_prompt() {
    std::vector<int> days_list = UserInputUtils::get_integer_list_input("Enter period days to export (e.g., 7 or 7,30,90): ");

    if (!days_list.empty()) {
        ReportFormat format = UserInputUtils::get_report_format_from_user();
        action_handler_->run_export_all_period_reports_query(days_list, format);
    } else {
        std::cout << YELLOW_COLOR << "No valid days provided for export." << RESET_COLOR << std::endl;
    }
}

// [修改] 此函数现在实例化并运行 LogProcessorMenu
void Menu::run_log_processor_submenu() {
    LogProcessorMenu submenu(action_handler_);
    submenu.run();
}

// [修改] 简化函数，使用 UserInputUtils
void Menu::run_full_pipeline_and_import_prompt() {
    std::string path_str = UserInputUtils::get_valid_path_input("Enter the path to the SOURCE directory to process: ");
    if (!path_str.empty()) {
        action_handler_->run_full_pipeline_and_import(path_str);
    }
}
