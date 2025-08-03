#include "common/pch.h"
#include "ActionHandler.h"


#include "action_handler/database/DatabaseManager.h"
#include "action_handler/reporting/ReportExporter.h"
#include "action_handler/query/DirectQueryManager.h" // 新增: 引入新的查询管理器
#include "db_inserter/DataImporter.h"
#include "common/common_utils.h"
#include "file_handler/FileUtils.h"

#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string> // 引入 <string> 以便使用 std::string

namespace fs = std::filesystem;

// 构造函数: 修正了初始化列表的顺序以匹配头文件中的声明顺序，并初始化了新的 db_name_ 成员
ActionHandler::ActionHandler(const std::string& db_name, const AppConfig& config, const std::string& main_config_path)
    : app_config_(config),
      main_config_path_(main_config_path),
      db_name_(db_name), // 新增: 初始化 db_name_
      export_root_path_(config.export_path.value_or("exported_files")),
      processor_(config)
{
    db_manager_ = std::make_unique<DatabaseManager>(db_name);
}

ActionHandler::~ActionHandler() = default;

// =========================================================================
//                      新增: 私有辅助函数
// =========================================================================

/**
 * @brief [私有] 获取并确保 DirectQueryManager 实例可用。
 * @details 封装了数据库连接检查和 DirectQueryManager 的懒加载逻辑。
 * @return 指向 DirectQueryManager 实例的指针，如果失败则返回 nullptr。
 */
 DirectQueryManager* ActionHandler::get_direct_query_manager() {
    if (!db_manager_->open_database_if_needed()) {
        std::cerr << RED_COLOR << "错误: 无法打开数据库，查询操作中止。" << RESET_COLOR << std::endl;
        return nullptr;
    }
    if (!direct_query_manager_) {
        direct_query_manager_ = std::make_unique<DirectQueryManager>(db_manager_->get_db_connection());
    }
    return direct_query_manager_.get();
}

/**
 * @brief [私有] 获取并确保 ReportExporter 实例可用。
 * @details 封装了数据库连接检查和 ReportExporter 的懒加载逻辑。
 * @return 指向 ReportExporter 实例的指针，如果失败则返回 nullptr。
 */
ReportExporter* ActionHandler::get_report_exporter() {
    if (!db_manager_->open_database_if_needed()) {
        std::cerr << RED_COLOR << "错误: 无法打开数据库，导出操作已中止。" << RESET_COLOR << std::endl;
        return nullptr;
    }
    if (!report_exporter_) {
        report_exporter_ = std::make_unique<ReportExporter>(db_manager_->get_db_connection(), export_root_path_);
    }
    return report_exporter_.get();
}

// =========================================================================
//                      直接查询实现 (已重构)
// =========================================================================

std::string ActionHandler::run_daily_query(const std::string& date, ReportFormat format) {
    if (auto* qm = get_direct_query_manager()) { // [重构]
        return qm->run_daily_query(date, format);
    }
    return std::string(RED_COLOR) + "错误: 查询失败。" + RESET_COLOR;
}

std::string ActionHandler::run_monthly_query(const std::string& month, ReportFormat format) {
    if (auto* qm = get_direct_query_manager()) { // [重构]
        return qm->run_monthly_query(month, format);
    }
    return std::string(RED_COLOR) + "错误: 查询失败。" + RESET_COLOR;
}

std::string ActionHandler::run_period_query(int days, ReportFormat format) {
    if (auto* qm = get_direct_query_manager()) { // [重构]
        return qm->run_period_query(days, format);
    }
    return std::string(RED_COLOR) + "错误: 查询失败。" + RESET_COLOR;
}

// =========================================================================
//                      报告导出实现 (已重构)
// =========================================================================

void ActionHandler::run_export_single_day_report(const std::string& date, ReportFormat format) {
    if (auto* exporter = get_report_exporter()) { // [重构]
        exporter->run_export_single_day_report(date, format);
    }
}

void ActionHandler::run_export_single_month_report(const std::string& month, ReportFormat format) {
    if (auto* exporter = get_report_exporter()) { // [重构]
        exporter->run_export_single_month_report(month, format);
    }
}

void ActionHandler::run_export_single_period_report(int days, ReportFormat format) {
    if (auto* exporter = get_report_exporter()) { // [重构]
        exporter->run_export_single_period_report(days, format);
    }
}

void ActionHandler::run_export_all_daily_reports_query(ReportFormat format) {
    if (auto* exporter = get_report_exporter()) { // [重构]
        exporter->run_export_all_daily_reports_query(format);
    }
}

void ActionHandler::run_export_all_monthly_reports_query(ReportFormat format) {
    if (auto* exporter = get_report_exporter()) { // [重构]
        exporter->run_export_all_monthly_reports_query(format);
    }
}

void ActionHandler::run_export_all_period_reports_query(const std::vector<int>& days_list, ReportFormat format) {
    if (auto* exporter = get_report_exporter()) { // [重构]
        exporter->run_export_all_period_reports_query(days_list, format);
    }
}

// =========================================================================
//                      文件处理与导入
// =========================================================================

void ActionHandler::run_database_import(const std::string& processed_path_str) {
    fs::path processed_path(processed_path_str);
    if (!fs::exists(processed_path) || !fs::is_directory(processed_path)) {
        std::cerr << RED_COLOR << "错误: " << RESET_COLOR << "路径不存在或不是目录。导入中止。" << std::endl;
        return;
    }
    db_manager_->close_database();
    std::cout << "开始导入过程..." << std::endl;
    handle_process_files(db_name_, processed_path.string(), main_config_path_);
    std::cout << "导入过程结束。" << std::endl;
}
void ActionHandler::run_full_pipeline_and_import(const std::string& source_path) {
    std::cout << "\n--- 开始完整流水线处理 ---" << std::endl;
    db_manager_->close_database();

    if (!collectFiles(source_path) || !validateSourceFiles() || !convertFiles() || !validateOutputFiles(true)) {
        std::cerr << RED_COLOR << "错误: 文件处理流水线失败，操作终止。" << RESET_COLOR << std::endl;
        return;
    }

    fs::path output_root_path = input_root_.parent_path() / ("Processed_" + input_root_.filename().string());
    run_database_import(output_root_path.string());
    
    std::cout << GREEN_COLOR << "\n成功: 完整流水线处理完毕并已导入数据。" << RESET_COLOR << std::endl;
}

bool ActionHandler::collectFiles(const std::string& input_path) {
    input_root_ = fs::path(input_path);
    if (!fs::exists(input_root_)) {
        std::cerr << RED_COLOR << "错误: 输入的路径不存在: " << input_path << RESET_COLOR << std::endl;
        return false;
    }

    files_to_process_.clear(); 
    source_to_output_map_.clear(); 

    files_to_process_ = FileUtils::find_files_by_extension_recursively(input_root_, ".txt");

    if (fs::is_regular_file(input_root_) && input_root_.extension() == ".txt") {
        if (files_to_process_.empty()) {
            files_to_process_.push_back(input_root_);
        }
    }

    std::cout << "信息: 成功收集到 " << files_to_process_.size() << " 个待处理文件。" << std::endl;
    return !files_to_process_.empty();
}

bool ActionHandler::validateSourceFiles() {
    const std::string current_operation_name = "validateSourceFiles";
    std::cout << "\n--- 阶段: 检验源文件 ---" << std::endl;
    if (files_to_process_.empty()) {
        std::cerr << YELLOW_COLOR << "警告: 没有已收集的文件可供检验。" << RESET_COLOR << std::endl;
        return true;
    }

    bool all_ok = true;
    double total_validation_time_ms = 0.0;

    for (const auto& file : files_to_process_) {
        AppOptions opts;
        opts.validate_source = true;
        
        ProcessingResult result = processor_.processFile(file, "", opts);
        total_validation_time_ms += result.timings.validation_source_ms;
        
        if (!result.success) {
            all_ok = false;
        }
    }

    printTimingStatistics(current_operation_name, total_validation_time_ms);
    std::cout << (all_ok ? GREEN_COLOR : RED_COLOR) << "源文件检验阶段 " << (all_ok ? "全部通过" : "存在失败项") << "。" << RESET_COLOR << std::endl;
    return all_ok;
}

bool ActionHandler::convertFiles() {
    const std::string current_operation_name = "convertFiles";
    std::cout << "\n--- 阶段: 转换文件 ---" << std::endl;
    if (files_to_process_.empty()) {
        std::cerr << YELLOW_COLOR << "警告: 没有已收集的文件可供转换。" << RESET_COLOR << std::endl;
        return true;
    }

    bool is_dir = fs::is_directory(input_root_);
    fs::path output_root_path;

    if (is_dir) {
        output_root_path = input_root_.parent_path() / ("Processed_" + input_root_.filename().string());
        fs::create_directories(output_root_path);
    }
    
    bool all_ok = true;
    double total_conversion_time_ms = 0.0;

    for (const auto& file : files_to_process_) {
        fs::path output_file_path;
        if (is_dir) {
            output_file_path = output_root_path / fs::relative(file, input_root_);
            fs::create_directories(output_file_path.parent_path());
        } else {
            output_file_path = input_root_.parent_path() / ("Processed_" + file.filename().string());
        }

        AppOptions opts;
        opts.convert = true;
        
        ProcessingResult result = processor_.processFile(file, output_file_path, opts);
        total_conversion_time_ms += result.timings.conversion_ms;

        if (result.success) {
            source_to_output_map_[file] = output_file_path;
        } else {
            all_ok = false;
        }
    }

    printTimingStatistics(current_operation_name, total_conversion_time_ms);
    std::cout << (all_ok ? GREEN_COLOR : RED_COLOR) << "文件转换阶段 " << (all_ok ? "全部成功" : "存在失败项") << "。" << RESET_COLOR << std::endl;
    return all_ok;
}

bool ActionHandler::validateOutputFiles(bool enable_day_count_check) {
    const std::string current_operation_name = "validateOutputFiles";
    std::cout << "\n--- 阶段: 检验输出文件 ---" << std::endl;
    if (source_to_output_map_.empty()) {
        std::cerr << YELLOW_COLOR << "警告: 没有已转换的文件可供检验。请先运行转换操作。" << RESET_COLOR << std::endl;
        return true;
    }

    bool all_ok = true;
    double total_validation_time_ms = 0.0;

    for (const auto& pair : source_to_output_map_) {
        const auto& output_file = pair.second;
        AppOptions opts;
        opts.validate_output = true;
        opts.enable_day_count_check = enable_day_count_check;
        
        ProcessingResult result = processor_.processFile("", output_file, opts);
        total_validation_time_ms += result.timings.validation_output_ms;

        if (!result.success) {
            all_ok = false;
        }
    }

    printTimingStatistics(current_operation_name, total_validation_time_ms);
    std::cout << (all_ok ? GREEN_COLOR : RED_COLOR) << "输出文件检验阶段 " << (all_ok ? "全部通过" : "存在失败项") << "。" << RESET_COLOR << std::endl;
    return all_ok;
}

void ActionHandler::printTimingStatistics(const std::string& operation_name, double total_time_ms) const {
    double total_time_s = total_time_ms / 1000.0;
    std::cout << "--------------------------------------\n";
    std::cout << "Timing Statistics:\n";
    std::cout << operation_name << "\n\n"; 
    std::cout << "Total time: " << std::fixed << std::setprecision(4) << total_time_s 
              << " seconds (" << total_time_ms << " ms)\n";
    std::cout << "--------------------------------------\n";
}