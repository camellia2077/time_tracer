// time_master_app/menu_processor/LogProcessorMenu.cpp

#include "LogProcessorMenu.hpp"

#include "action_handler/FileProcessingHandler.hpp"
#include "common/common_utils.hpp"
#include "time_master_app/menu_input/UserInputUtils.hpp"

#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>

LogProcessorMenu::LogProcessorMenu(FileProcessingHandler* handler)
    : file_processing_handler_(handler) {}

void LogProcessorMenu::run() {
    while (true) {
        print_submenu();

        int choice = -1;
        std::string line;
        if (!std::getline(std::cin, line) || line.empty()) { continue; }
        
        try { 
            choice = std::stoi(line); 
        } catch (const std::exception&) {
            std::cout << YELLOW_COLOR << "Invalid input. Please enter a number." << RESET_COLOR << std::endl;
            continue;
        }

        if (choice == 6) {
            break; // 返回主菜单
        }
        
        handle_choice(choice);

        std::cout << "\nPress Enter to continue...";
        std::string dummy;
        std::getline(std::cin, dummy);
    }
}

void LogProcessorMenu::print_submenu() const {
    std::cout << "\n--- File Pre-processing & Validation Submenu ---\n";
    std::cout << "--- (Individual Steps) ---\n";
    std::cout << "1. Validate Source Files (*.txt)\n";
    std::cout << "2. Convert Source Files (*.txt -> *.json)\n";
    std::cout << "3. Validate Output Files (*.json)\n";
    std::cout << "--- (Pipelines) ---\n";
    std::cout << "4. Run Full Pipeline (Validate Source -> Convert -> Validate Output)\n";
    std::cout << "--- (Database) ---\n";
    std::cout << "5. Import Processed Files (*.json) into Database\n";
    std::cout << "6. Back to Main Menu\n";
    std::cout << "Enter your choice: ";
}

void LogProcessorMenu::handle_choice(int choice) {
    if ((choice < 1 || choice > 5)) {
        std::cout << YELLOW_COLOR << "Invalid choice. Please try again.\n" << RESET_COLOR;
        return;
    }

    if (choice == 5) {
        std::string path = UserInputUtils::get_valid_path_input("Enter the path to the DIRECTORY containing processed files (*.json): ");
        if (!path.empty()) {
            file_processing_handler_->run_database_import(path);
        }
        return;
    }
    
    std::string path;
    if (choice == 3) {
        path = UserInputUtils::get_valid_path_input("Enter the path to the PROCESSED file or directory (*.json) to validate: ");
    } else {
        path = UserInputUtils::get_valid_path_input("Enter the path to the SOURCE file or directory (*.txt) to process: ");
    }
    if (path.empty()) return;

    PreprocessingOptions options;
    // [修复] 移除了未使用的 enable_day_check 变量

    switch (choice) {
        case 1:
            options.validate_source = true;
            break;
        case 2:
            options.convert = true;
            break;
        case 3:
            options.validate_output = true;
            std::cout << "Enable check for day completeness in a month? (y/n) [Default: n]: ";
            if (UserInputUtils::get_yes_no_input()) {
                options.enable_day_check = true;
            }
            break;
        case 4:
            options.validate_source = true;
            options.convert = true;
            options.validate_output = true;
            options.enable_day_check = true;
            break;
    }

    try {
        file_processing_handler_->run_preprocessing(path, options);
    } catch (const std::runtime_error& e) {
        std::cerr << RED_COLOR << "An error occurred: " << e.what() << RESET_COLOR << std::endl;
    }
}