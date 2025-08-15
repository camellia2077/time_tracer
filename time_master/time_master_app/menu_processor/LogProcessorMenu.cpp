// time_master_app/menu_processor/LogProcessorMenu.cpp

#include "LogProcessorMenu.h"
// [修改] 引入新的处理器头文件
#include "action_handler/FileProcessingHandler.h"
#include "action_handler/file/FilePipelineManager.h"
#include "common/common_utils.h"
#include "time_master_app/menu_input/UserInputUtils.h"
#include <iostream>
#include <string>
#include <memory>

// [修改] 构造函数更新
LogProcessorMenu::LogProcessorMenu(FileProcessingHandler* handler)
    : file_processing_handler_(handler) {}

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

// [修改] handle_choice 现在调用新的处理器
void LogProcessorMenu::handle_choice(int choice) {
    if ((choice < 1 || choice > 5) && choice != 7) {
        std::cout << YELLOW_COLOR << "Invalid choice. Please try again.\n" << RESET_COLOR;
        return;
    }

    if (choice == 7) {
        std::string path = UserInputUtils::get_valid_path_input("Enter the path to the DIRECTORY containing processed files: ");
        if (!path.empty()) {
            file_processing_handler_->run_database_import(path);
        }
    } else {
        std::string path = UserInputUtils::get_valid_path_input("Enter the path to the SOURCE file or directory to process: ");
        if (path.empty()) return;

        // FilePipelineManager 现在从 FileProcessingHandler 获取配置
        FilePipelineManager pipeline(file_processing_handler_->get_config());

        if (!pipeline.collectFiles(path)) {
            std::cout << RED_COLOR << "Failed to collect files. Please check the path and try again." << RESET_COLOR << std::endl;
            return;
        }

        switch (choice) {
            case 1: 
                pipeline.validateSourceFiles(); 
                break;
            case 2: 
                pipeline.convertFiles(); 
                break;
            case 3: 
                if (pipeline.validateSourceFiles()) {
                    pipeline.convertFiles();
                }
                break;
            case 4: 
                if (pipeline.convertFiles()) {
                    pipeline.validateOutputFiles(false);
                }
                break;
            case 5: 
                if (pipeline.validateSourceFiles() && pipeline.convertFiles()) {
                    pipeline.validateOutputFiles(false);
                }
                break;
        }
    }
}