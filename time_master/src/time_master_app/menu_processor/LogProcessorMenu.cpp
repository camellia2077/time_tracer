// time_master_app/menu_processor/LogProcessorMenu.cpp

#include "LogProcessorMenu.hpp"

#include "action_handler/FileProcessingHandler.hpp"
#include "common/common_utils.hpp"
#include "time_master_app/menu_input/UserInputUtils.hpp"

#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>

// 构造函数保持不变
LogProcessorMenu::LogProcessorMenu(FileProcessingHandler* handler)
    : file_processing_handler_(handler) {}

// run() 和 print_submenu() 方法保持不变
void LogProcessorMenu::run() {
    // ... 此函数保持不变 ...
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
    // ... 此函数保持不变 ...
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


// [修改] handle_choice 现在调用封装好的 run_preprocessing 方法
void LogProcessorMenu::handle_choice(int choice) {
    if ((choice < 1 || choice > 5) && choice != 7) {
        std::cout << YELLOW_COLOR << "Invalid choice. Please try again.\n" << RESET_COLOR;
        return;
    }

    // 数据库导入逻辑保持不变
    if (choice == 7) {
        std::string path = UserInputUtils::get_valid_path_input("Enter the path to the DIRECTORY containing processed files: ");
        if (!path.empty()) {
            file_processing_handler_->run_database_import(path);
        }
        return; // 处理完后直接返回
    }
    
    // 预处理逻辑 (choices 1-5)
    std::string path = UserInputUtils::get_valid_path_input("Enter the path to the SOURCE file or directory to process: ");
    if (path.empty()) return;

    // 1. 根据用户选择构建 PreprocessingOptions
    PreprocessingOptions options;
    switch (choice) {
        case 1: // Validate source only
            options.validate_source = true;
            break;
        case 2: // Convert only
            options.convert = true;
            break;
        case 3: // Validate source, then Convert
            options.validate_source = true;
            options.convert = true;
            break;
        case 4: // Convert, then Validate Output
            options.convert = true;
            options.validate_output = true;
            break;
        case 5: // Full Pre-processing
            options.validate_source = true;
            options.convert = true;
            options.validate_output = true;
            break;
    }

    // 2. 调用处理器执行任务，并捕获可能的错误
    try {
        file_processing_handler_->run_preprocessing(path, options);
    } catch (const std::runtime_error& e) {
        std::cerr << RED_COLOR << "An error occurred: " << e.what() << RESET_COLOR << std::endl;
    }
}