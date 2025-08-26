// time_master_app/menu_query/QueryMenu.cpp
#include "QueryMenu.hpp"

#include "action_handler/ReportGenerationHandler.hpp"
#include "time_master_app/menu_input/UserInputUtils.hpp"
#include "common/common_utils.hpp"

#include <iostream>
#include <string>
#include <vector>

QueryMenu::QueryMenu(ReportGenerationHandler* handler)
    : report_generation_handler_(handler) {}

void QueryMenu::run() {
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

        if (choice == 4) {
            break; // 返回主菜单
        }
        handle_choice(choice);
        std::cout << "\nPress Enter to continue...";
        std::string dummy;
        std::getline(std::cin, dummy);
    }
}

void QueryMenu::print_menu() const {
    std::cout << "\n--- Query Submenu ---\n";
    std::cout << "1. Query daily statistics\n";
    std::cout << "2. Query Period Statistics\n";
    std::cout << "3. Query monthly statistics\n";
    std::cout << "4. Back to main menu\n";
    std::cout << "Enter your choice: ";
}

void QueryMenu::handle_choice(int choice) {
    switch (choice) {
        case 1: { // Query Daily
            std::string date = UserInputUtils::get_valid_date_input();
            if (!date.empty()) {
                ReportFormat format = UserInputUtils::get_report_format_from_user();
                std::cout << report_generation_handler_->run_daily_query(date, format);
            }
            break;
        }
        case 2:
            run_period_query_prompt();
            break;
        case 3: { // Query Monthly
            std::string month = UserInputUtils::get_valid_month_input();
            if (!month.empty()) {
                ReportFormat format = UserInputUtils::get_report_format_from_user();
                std::cout << report_generation_handler_->run_monthly_query(month, format);
            }
            break;
        }
        default:
            std::cout << YELLOW_COLOR << "Invalid choice. Please try again." << RESET_COLOR << std::endl;
            break;
    }
}

void QueryMenu::run_period_query_prompt() {
    std::vector<int> days_list = UserInputUtils::get_integer_list_input("Enter period days (e.g., 7 or 7,30,90): ");
    if (days_list.empty()) return;

    ReportFormat format = UserInputUtils::get_report_format_from_user();

    for (int days : days_list) {
        std::cout << "\n--- Report for last " << days << " days ---\n";
        std::cout << report_generation_handler_->run_period_query(days, format);
    }
}