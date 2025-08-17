// time_master_app/menu_export/ExportMenu.cpp
#include "ExportMenu.h"
#include "action_handler/ReportGenerationHandler.h"
#include "time_master_app/menu_input/UserInputUtils.h"
#include "common/common_utils.h"
#include <iostream>
#include <string>
#include <vector>

ExportMenu::ExportMenu(ReportGenerationHandler* handler)
    : report_generation_handler_(handler) {}

void ExportMenu::run() {
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

        if (choice == 7) {
            break; // 返回主菜单
        }
        handle_choice(choice);
        std::cout << "\nPress Enter to continue...";
        std::string dummy;
        std::getline(std::cin, dummy);
    }
}

void ExportMenu::print_menu() const {
    std::cout << "\n--- Export Submenu ---\n";
    std::cout << "--- (Single Report) ---\n";
    std::cout << "1. Export single DAY report\n";
    std::cout << "2. Export single MONTH report\n";
    std::cout << "3. Export single PERIOD report\n";
    std::cout << "--- (Bulk) ---\n";
    std::cout << "4. Export ALL daily reports\n";
    std::cout << "5. Export ALL monthly reports\n";
    std::cout << "6. Export ALL period reports\n";
    std::cout << "7. Back to main menu\n";
    std::cout << "Enter your choice: ";
}

void ExportMenu::handle_choice(int choice) {
    switch (choice) {
        case 1: run_export_single_day_report_prompt(); break;
        case 2: run_export_single_month_report_prompt(); break;
        case 3: run_export_single_period_report_prompt(); break;
        case 4: { // Export ALL Daily
            ReportFormat format = UserInputUtils::get_report_format_from_user();
            report_generation_handler_->run_export_all_daily_reports_query(format);
            break;
        }
        case 5: { // Export ALL Monthly
            ReportFormat format = UserInputUtils::get_report_format_from_user();
            report_generation_handler_->run_export_all_monthly_reports_query(format);
            break;
        }
        case 6: run_export_all_period_reports_prompt(); break;
        default:
            std::cout << YELLOW_COLOR << "Invalid choice. Please try again." << RESET_COLOR << std::endl;
            break;
    }
}

void ExportMenu::run_export_single_day_report_prompt() {
    std::string date = UserInputUtils::get_valid_date_input();
    if (!date.empty()) {
        ReportFormat format = UserInputUtils::get_report_format_from_user();
        report_generation_handler_->run_export_single_day_report(date, format);
    }
}

void ExportMenu::run_export_single_month_report_prompt() {
    std::string month = UserInputUtils::get_valid_month_input();
    if (!month.empty()) {
        ReportFormat format = UserInputUtils::get_report_format_from_user();
        report_generation_handler_->run_export_single_month_report(month, format);
    }
}

void ExportMenu::run_export_single_period_report_prompt() {
    std::vector<int> days_list = UserInputUtils::get_integer_list_input("Enter period days to export (e.g., 7 or 7,30,90): ");
    if (days_list.empty()) {
        std::cout << YELLOW_COLOR << "No valid days provided for export." << RESET_COLOR << std::endl;
        return;
    }
    
    ReportFormat format = UserInputUtils::get_report_format_from_user();
    for (int days : days_list) {
        report_generation_handler_->run_export_single_period_report(days, format);
    }
}

void ExportMenu::run_export_all_period_reports_prompt() {
    std::vector<int> days_list = UserInputUtils::get_integer_list_input("Enter period days to export (e.g., 7 or 7,30,90): ");

    if (!days_list.empty()) {
        ReportFormat format = UserInputUtils::get_report_format_from_user();
        report_generation_handler_->run_export_all_period_reports_query(days_list, format);
    } else {
        std::cout << YELLOW_COLOR << "No valid days provided for export." << RESET_COLOR << std::endl;
    }
}