#include "action_handler.h"
#include "query_handler.h"
#include "processing.h"
#include "FormatValidator.h"
#include "IntervalProcessor.h"
#include "common_utils.h"
#include "file_handler.h"

#include <iostream>
#include <sqlite3.h>
#include <filesystem>
#include <set>
#include <map>
#include <ctime>

namespace fs = std::filesystem;

ActionHandler::ActionHandler(const std::string& db_name, const AppConfig& config, const std::string& main_config_path)
    : db_(nullptr),
      db_name_(db_name),
      app_config_(config),
      main_config_path_(main_config_path) {}

ActionHandler::~ActionHandler() {
    close_database();
}

// --- 查询逻辑实现 ---
void ActionHandler::run_daily_query(const std::string& date) {
    if (!open_database_if_needed()) return;
    QueryHandler(db_).run_daily_query(date);
}

void ActionHandler::run_period_query(int days) {
    if (!open_database_if_needed()) return;
    QueryHandler(db_).run_period_query(days);
}

void ActionHandler::run_monthly_query(const std::string& month) {
    if (!open_database_if_needed()) return;
    QueryHandler(db_).run_monthly_query(month);
}

// --- 文件处理逻辑实现 ---
void ActionHandler::run_log_processing(const AppOptions& options) {
    close_database(); // 确保数据库已关闭，避免文件锁定
    LogProcessor processor(app_config_);
    processor.run(options);
}

void ActionHandler::run_database_import(const std::string& processed_path_str) {
    fs::path processed_path(processed_path_str);
    if (!fs::exists(processed_path) || !fs::is_directory(processed_path)) {
        std::cerr << RED_COLOR << "Error: " << RESET_COLOR << "Path does not exist or is not a directory. Aborting import." << std::endl;
        return;
    }
    close_database();
    std::cout << "Starting import process..." << std::endl;
    handle_process_files(db_name_, processed_path.string(), main_config_path_);
    std::cout << "Import process finished." << std::endl;
}

// --- 完整流水线逻辑实现  ---
void ActionHandler::run_full_pipeline_and_import(const std::string& source_path) {
    fs::path input_path(source_path);
    if (!fs::exists(input_path) || !fs::is_directory(input_path)) {
        std::cerr << RED_COLOR << "Error: " << RESET_COLOR << "Path does not exist or is not a directory. Aborting." << std::endl;
        return;
    }

    std::cout << "\n--- Starting Full Pipeline ---" << std::endl;
    
    // 步骤 1: 验证源文件、转换、验证输出文件
    std::cout << "\n--- Phase 1: Processing files (Validate -> Convert -> Validate) ---\n";
    close_database(); // 文件处理前关闭数据库
    
    AppOptions options;
    options.input_path = input_path.string();
    options.run_all = true; // 使用 run_all 保证遇到错误时停止
    options.validate_source = true;
    options.convert = true;
    options.validate_output = true;
    options.enable_day_count_check = true; // 默认在流水线中进行严格检查

    LogProcessor processor(app_config_);
    bool processing_succeeded = processor.run(options);

    if (!processing_succeeded) {
        std::cerr << RED_COLOR << "Error: " << RESET_COLOR << "File processing phase failed. Aborting database import." << std::endl;
        return;
    }
    
    std::cout << GREEN_COLOR << "\nFile processing phase completed successfully." << RESET_COLOR << std::endl;

    // 步骤 2: 导入到数据库
    std::cout << "\n--- Phase 2: Importing all processed files into database... ---\n";
    
    // 推断输出目录的路径 (与 LogProcessor 的逻辑保持一致)
    fs::path output_root_path = input_path.parent_path() / ("Processed_" + input_path.filename().string());
    
    run_database_import(output_root_path.string());
    
    std::cout << GREEN_COLOR << "\nSuccess: Full pipeline completed and data imported." << RESET_COLOR << std::endl;
}


// --- 数据库连接管理 ---
bool ActionHandler::open_database_if_needed() {
    if (db_ == nullptr) {
        if (sqlite3_open(db_name_.c_str(), &db_)) {
            std::cerr << RED_COLOR << "Error: " << RESET_COLOR << "Can't open database " << db_name_ << ": " << sqlite3_errmsg(db_) << std::endl;
            sqlite3_close(db_);
            db_ = nullptr;
            return false;
        }
    }
    return true;
}

void ActionHandler::close_database() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}