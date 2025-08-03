// time_master_app/LogProcessorMenu.cpp
#include "common/pch.h"
#include "LogProcessorMenu.h"
#include "action_handler/ActionHandler.h"
#include "action_handler/file/FilePipelineManager.h" // [新增] 引入新的管理器
#include "common/common_utils.h"
#include "time_master_app/menu_input/UserInputUtils.h"
#include <iostream>
#include <string>
#include <memory> // [新增] 为了 std::unique_ptr

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

// [修改] handle_choice 函数的完整实现
void LogProcessorMenu::handle_choice(int choice) {
    if ((choice < 1 || choice > 5) && choice != 7) {
        std::cout << YELLOW_COLOR << "Invalid choice. Please try again.\n" << RESET_COLOR;
        return;
    }

    if (choice == 7) {
        // 数据库导入功能仍然由 ActionHandler 负责
        std::string path = UserInputUtils::get_valid_path_input("Enter the path to the DIRECTORY containing processed files: ");
        if (!path.empty()) {
            action_handler_->run_database_import(path);
        }
    } else {
        // 其他所有文件处理选项都使用 FilePipelineManager
        std::string path = UserInputUtils::get_valid_path_input("Enter the path to the SOURCE file or directory to process: ");
        if (path.empty()) return;

        // 为本次操作创建一个 FilePipelineManager 实例
        // 注意：这里需要 ActionHandler 提供 AppConfig
        // 请确保 ActionHandler 有一个 public 的 get_config() 方法
        FilePipelineManager pipeline(action_handler_->get_config());

        // 收集文件是所有选项的第一步
        if (!pipeline.collectFiles(path)) {
            std::cout << RED_COLOR << "Failed to collect files. Please check the path and try again." << RESET_COLOR << std::endl;
            return;
        }

        // 根据用户的选择执行相应的步骤
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