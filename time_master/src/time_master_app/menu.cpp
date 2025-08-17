// time_master_app/menu.cpp

#include "Menu.h"
#include "action_handler/FileProcessingHandler.h"
#include "action_handler/ReportGenerationHandler.h"
#include "common/version.h"
#include "common/common_utils.h"
#include "time_master_app/menu_processor/LogProcessorMenu.h"
#include "time_master_app/menu_query/QueryMenu.h"
#include "time_master_app/menu_export/ExportMenu.h"
#include "time_master_app/menu_input/UserInputUtils.h"

#include <iostream>
#include <string>
#include <vector>
#include <memory>

Menu::Menu(const std::string& db_name, const AppConfig& config, const std::string& main_config_path) {
    file_processing_handler_ = std::make_unique<FileProcessingHandler>(db_name, config, main_config_path);
    report_generation_handler_ = std::make_unique<ReportGenerationHandler>(db_name, config);
}

Menu::~Menu() = default;

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
    std::cout << " 1. Full Pipeline (Validate -> Convert -> Import)" << std::endl;
    std::cout << "--- Reprocessing ---" << std::endl;
    std::cout << " 2. File Processing & Validation (Submenu)" << std::endl;
    std::cout << "--- Query ---" << std::endl;
    std::cout << " 3. Query (Submenu)" << std::endl;
    std::cout << "--- Export ---" << std::endl;
    std::cout << " 4. Export (Submenu)" << std::endl;
    std::cout << "--- Other ---" << std::endl;
    std::cout << " 5. Show Version" << std::endl;
    std::cout << " 6. Exit" << std::endl;
    std::cout << "Enter your choice: ";
}

bool Menu::handle_user_choice(int choice) {
    switch (choice) {
        case 1: run_full_pipeline_and_import_prompt(); break;
        case 2: run_log_processor_submenu(); break;
        case 3: run_query_submenu(); break;
        case 4: run_export_submenu(); break;
        case 5:
            std::cout << "TimeMaster Version: " << AppInfo::VERSION << " (Last Updated: " << AppInfo::LAST_UPDATED << ")" << std::endl;
            break;
        case 6:
            std::cout << "Exiting program." << std::endl;
            return false;
        default:
            std::cout << YELLOW_COLOR << "Invalid choice. Please try again." << RESET_COLOR << std::endl;
            break;
    }
    if (choice != 6) { // Don't wait for "Enter" if exiting
        std::cout << "\nPress Enter to continue...";
        std::string dummy;
        std::getline(std::cin, dummy);
    }
    return true;
}

void Menu::run_log_processor_submenu() {
    LogProcessorMenu submenu(file_processing_handler_.get());
    submenu.run();
}

void Menu::run_query_submenu() {
    QueryMenu submenu(report_generation_handler_.get());
    submenu.run();
}

void Menu::run_export_submenu() {
    ExportMenu submenu(report_generation_handler_.get());
    submenu.run();
}

void Menu::run_full_pipeline_and_import_prompt() {
    std::string path_str = UserInputUtils::get_valid_path_input("Enter the path to the SOURCE directory to process: ");
    if (!path_str.empty()) {
        file_processing_handler_->run_full_pipeline_and_import(path_str);
    }
}