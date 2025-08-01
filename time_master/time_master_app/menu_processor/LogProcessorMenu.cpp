// time_master_app/LogProcessorMenu.cpp
#include "common/pch.h"
#include "LogProcessorMenu.h"
#include "action_handler/ActionHandler.h"
#include "common/common_utils.h"
#include "time_master_app/menu_input/UserInputUtils.h" // [新增] 使用新的输入工具
#include <iostream>
#include <string>

LogProcessorMenu::LogProcessorMenu(ActionHandler* action_handler)
    : action_handler_(action_handler) {}

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

        if (choice == 8) {
            break; // 返回主菜单
        }
        
        handle_choice(choice);

        std::cout << "\nPress Enter to continue...";
        std::string dummy;
        std::getline(std::cin, dummy);
    }
}

void LogProcessorMenu::print_submenu() const {
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
}

void LogProcessorMenu::handle_choice(int choice) {
    if ((choice < 1 || choice > 5) && choice != 7) {
        std::cout << YELLOW_COLOR << "Invalid choice. Please try again.\n" << RESET_COLOR;
        return;
    }

    if (choice == 7) {
        std::string path = UserInputUtils::get_valid_path_input("Enter the path to the DIRECTORY containing processed files: ");
        if (!path.empty()) {
            action_handler_->run_database_import(path);
        }
    } else {
        std::string path = UserInputUtils::get_valid_path_input("Enter the path to the SOURCE file or directory to process: ");
        if (path.empty()) return;

        if (!action_handler_->collectFiles(path)) {
            std::cout << RED_COLOR << "Failed to collect files. Please check the path and try again." << RESET_COLOR << std::endl;
            return;
        }

        switch (choice) {
            case 1: action_handler_->validateSourceFiles(); break;
            case 2: action_handler_->convertFiles(); break;
            case 3: if (action_handler_->validateSourceFiles()) action_handler_->convertFiles(); break;
            case 4: if (action_handler_->convertFiles()) action_handler_->validateOutputFiles(false); break;
            case 5: if (action_handler_->validateSourceFiles() && action_handler_->convertFiles()) action_handler_->validateOutputFiles(false); break;
        }
    }
}