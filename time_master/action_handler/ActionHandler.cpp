#include "ActionHandler.h "
#include "QueryHandler.h" // 查询数据库
#include "DataImporter.h" // 解析文本并且插入数据库

#include "common_utils.h"

#include "LogProcessor.h" // 包含格式验证和格式转换

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
      main_config_path_(main_config_path),
      processor_(config) // 初始化 LogProcessor 成员
{}

ActionHandler::~ActionHandler() {
    close_database();
}

bool ActionHandler::validateSourceFiles() {
    std::cout << "\n--- 阶段: 检验源文件 ---" << std::endl;
    if (files_to_process_.empty()) {
        std::cerr << YELLOW_COLOR << "警告: 没有已收集的文件可供检验。" << RESET_COLOR << std::endl;
        return true; // 没有文件也算成功
    }

    bool all_ok = true;
    for (const auto& file : files_to_process_) {
        AppOptions opts;
        opts.validate_source = true;
        // 调用 processFile，但只关心源文件验证部分
        if (!processor_.processFile(file, "", opts)) {
            all_ok = false;
        }
    }
    std::cout << (all_ok ? GREEN_COLOR : RED_COLOR) << "源文件检验阶段 " << (all_ok ? "全部通过" : "存在失败项") << "。" << RESET_COLOR << std::endl;
    return all_ok;
}

bool ActionHandler::convertFiles() {
    std::cout << "\n--- 阶段: 转换文件 ---" << std::endl;
    if (files_to_process_.empty()) {
        std::cerr << YELLOW_COLOR << "警告: 没有已收集的文件可供转换。" << RESET_COLOR << std::endl;
        return true;
    }

    bool is_dir = fs::is_directory(input_root_);
    fs::path output_root_path;

    // 创建总的输出目录
    if (is_dir) {
        output_root_path = input_root_.parent_path() / ("Processed_" + input_root_.filename().string());
        fs::create_directories(output_root_path);
    }
    
    bool all_ok = true;
    for (const auto& file : files_to_process_) {
        fs::path output_file_path;
        // 构建输出路径
        if (is_dir) {
            output_file_path = output_root_path / fs::relative(file, input_root_);
            fs::create_directories(output_file_path.parent_path());
        } else {
            output_file_path = input_root_.parent_path() / ("Processed_" + file.filename().string());
        }

        AppOptions opts;
        opts.convert = true;
        // 调用 processFile，只关心转换部分
        if (processor_.processFile(file, output_file_path, opts)) {
            source_to_output_map_[file] = output_file_path; // 记录路径映射
        } else {
            all_ok = false;
        }
    }
    std::cout << (all_ok ? GREEN_COLOR : RED_COLOR) << "文件转换阶段 " << (all_ok ? "全部成功" : "存在失败项") << "。" << RESET_COLOR << std::endl;
    return all_ok;
}

bool ActionHandler::validateOutputFiles(bool enable_day_count_check) {
    std::cout << "\n--- 阶段: 检验输出文件 ---" << std::endl;
    if (source_to_output_map_.empty()) {
        std::cerr << YELLOW_COLOR << "警告: 没有已转换的文件可供检验。请先运行转换操作。" << RESET_COLOR << std::endl;
        return true;
    }

    bool all_ok = true;
    for (const auto& pair : source_to_output_map_) {
        const auto& output_file = pair.second;
        AppOptions opts;
        opts.validate_output = true;
        opts.enable_day_count_check = enable_day_count_check;
        // 调用 processFile，只关心输出文件验证部分
        if (!processor_.processFile("", output_file, opts)) {
            all_ok = false;
        }
    }
    std::cout << (all_ok ? GREEN_COLOR : RED_COLOR) << "输出文件检验阶段 " << (all_ok ? "全部通过" : "存在失败项") << "。" << RESET_COLOR << std::endl;
    return all_ok;
}

bool ActionHandler::collectFiles(const std::string& input_path) {
    input_root_ = fs::path(input_path);
    if (!fs::exists(input_root_)) {
        std::cerr << RED_COLOR << "错误: 输入的路径不存在: " << input_path << RESET_COLOR << std::endl;
        return false;
    }

    files_to_process_.clear(); // 清空旧数据
    source_to_output_map_.clear(); // 清空旧数据

    if (fs::is_directory(input_root_)) {
        for (const auto& entry : fs::recursive_directory_iterator(input_root_)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                files_to_process_.push_back(entry.path());
            }
        }
        std::sort(files_to_process_.begin(), files_to_process_.end());
    } else if (fs::is_regular_file(input_root_)) {
        files_to_process_.push_back(input_root_);
    }
    
    std::cout << "信息: 成功收集到 " << files_to_process_.size() << " 个待处理文件。" << std::endl;
    return !files_to_process_.empty();
}


// --- 查询逻辑实现 ---
std::string ActionHandler::run_daily_query(const std::string& date) {
    if (!open_database_if_needed()) return "";
    return QueryHandler(db_).run_daily_query(date);
}

std::string ActionHandler::run_period_query(int days) {
    if (!open_database_if_needed()) return "";
    return QueryHandler(db_).run_period_query(days);
}

std::string ActionHandler::run_monthly_query(const std::string& month) {
    if (!open_database_if_needed()) return "";
    return QueryHandler(db_).run_monthly_query(month);
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

// --- 完整流水线逻辑实现 ---
void ActionHandler::run_full_pipeline_and_import(const std::string& source_path) {
    std::cout << "\n--- 开始完整流水线处理 ---" << std::endl;
    close_database();

    // 阶段 1: 收集文件
    if (!collectFiles(source_path)) {
        std::cerr << RED_COLOR << "错误: 文件收集失败，流水线终止。" << RESET_COLOR << std::endl;
        return;
    }

    // 阶段 2: 验证源文件
    if (!validateSourceFiles()) {
        std::cerr << RED_COLOR << "错误: 源文件检验失败，流水线终止。" << RESET_COLOR << std::endl;
        return;
    }

    // 阶段 3: 转换文件
    if (!convertFiles()) {
        std::cerr << RED_COLOR << "错误: 文件转换失败，流水线终止。" << RESET_COLOR << std::endl;
        return;
    }

    // 阶段 4: 验证输出文件
    if (!validateOutputFiles(true)) { // 在完整流水线中启用天数检查
        std::cerr << RED_COLOR << "错误: 输出文件检验失败，流水线终止。" << RESET_COLOR << std::endl;
        return;
    }

    // 阶段 5: 导入数据库
    fs::path output_root_path = input_root_.parent_path() / ("Processed_" + input_root_.filename().string());
    run_database_import(output_root_path.string());
    
    std::cout << GREEN_COLOR << "\n成功: 完整流水线处理完毕并已导入数据。" << RESET_COLOR << std::endl;
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