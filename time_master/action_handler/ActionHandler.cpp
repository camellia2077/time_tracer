// action_handler\ActionHandler.cpp
#include "action_handler/ActionHandler.h"
#include "ExportUtils.h" 

#include "queries/QueryHandler.h"
#include "db_inserter/DataImporter.h"
#include "common/common_utils.h"
#include "reprocessing/LogProcessor.h"
#include "file_handler/FileUtils.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <set>
#include <map>
#include <ctime>
#include <stdexcept>

namespace fs = std::filesystem;

ActionHandler::ActionHandler(const std::string& db_name, const AppConfig& config, const std::string& main_config_path)
    : db_(nullptr),
      db_name_(db_name),
      app_config_(config),
      main_config_path_(main_config_path),
      processor_(config),
      export_root_path_(config.export_path.value_or("exported_files")) // 导出文件的根目录命名
{}

ActionHandler::~ActionHandler() {
    close_database();
}

std::string ActionHandler::run_daily_query(const std::string& date, ReportFormat format) const {
    if (!open_database_if_needed()) return "";
    QueryHandler query_handler(db_);
    return query_handler.run_daily_query(date, format);
}

std::string ActionHandler::run_period_query(int days, ReportFormat format) const {
    if (!open_database_if_needed()) return "";
    QueryHandler query_handler(db_);
    return query_handler.run_period_query(days, format);
}

std::string ActionHandler::run_monthly_query(const std::string& month, ReportFormat format) const {
    if (!open_database_if_needed()) return "";
    QueryHandler query_handler(db_);
    return query_handler.run_monthly_query(month, format);
}

void ActionHandler::run_export_all_daily_reports_query(ReportFormat format) const {
    if (!open_database_if_needed()) {
        std::cerr << RED_COLOR << "错误: 无法打开数据库，导出操作已中止。" << RESET_COLOR << std::endl;
        return;
    }

    QueryHandler query_handler(db_);
    FormattedGroupedReports grouped_reports = query_handler.run_export_all_daily_reports_query(format);

    if (grouped_reports.empty()) {
        std::cout << YELLOW_COLOR << "信息: 数据库中没有可导出的日报内容。" << RESET_COLOR << std::endl;
        return;
    }

    auto format_details_opt = ExportUtils::get_report_format_details(format);
    if (!format_details_opt) {
        return;
    }
    const auto& format_details = *format_details_opt;

    // 使用成员变量替换硬编码的 "Export"
    fs::path export_base_dir = export_root_path_ / format_details.dir_name / "days";    

    auto daily_export_logic = [&]() -> int {
        int files_created = 0;
        for (const auto& year_pair : grouped_reports) {
            int year = year_pair.first;
            for (const auto& month_pair : year_pair.second) {
                int month = month_pair.first;
                std::stringstream month_ss;
                month_ss << std::setw(2) << std::setfill('0') << month;
                fs::path month_dir = export_base_dir / std::to_string(year) / month_ss.str();
                fs::create_directories(month_dir);

                for (const auto& report_pair : month_pair.second) {
                    const std::string& date_str = report_pair.first;
                    const std::string& report_content = report_pair.second;
                    
                    fs::path output_path = month_dir / (date_str + format_details.extension);
                    std::ofstream output_file(output_path);
                    if (!output_file) {
                        std::cerr << RED_COLOR << "错误: 无法创建或打开文件: " << output_path << RESET_COLOR << std::endl;
                        continue;
                    }
                    output_file << report_content;
                    files_created++;
                }
            }
        }
        return files_created;
    };
    
    ExportUtils::execute_export_task("日报", export_base_dir, daily_export_logic);
}

void ActionHandler::run_export_all_monthly_reports_query(ReportFormat format) const {
    if (!open_database_if_needed()) {
        std::cerr << RED_COLOR << "错误: 无法打开数据库，导出操作已中止。" << RESET_COLOR << std::endl;
        return;
    }

    QueryHandler query_handler(db_);
    FormattedMonthlyReports grouped_reports = query_handler.run_export_all_monthly_reports_query(format);

    if (grouped_reports.empty()) {
        std::cout << YELLOW_COLOR << "信息: 数据库中没有可导出的月报内容。" << RESET_COLOR << std::endl;
        return;
    }

    auto format_details_opt = ExportUtils::get_report_format_details(format);
    if (!format_details_opt) {
        return;
    }
    const auto& format_details = *format_details_opt;
    
    fs::path export_base_dir = export_root_path_ / format_details.dir_name / "months";

    auto monthly_export_logic = [&]() -> int {
        int files_created = 0;
        for (const auto& year_pair : grouped_reports) {
            int year = year_pair.first;
            fs::path year_dir = export_base_dir / std::to_string(year);
            fs::create_directories(year_dir);

            for (const auto& month_pair : year_pair.second) {
                int month = month_pair.first;
                const std::string& report_content = month_pair.second;

                std::stringstream filename_ss;
                filename_ss << year << "_" << std::setw(2) << std::setfill('0') << month << format_details.extension;
                fs::path output_path = year_dir / filename_ss.str();

                std::ofstream output_file(output_path);
                if (!output_file) {
                    std::cerr << RED_COLOR << "错误: 无法创建或打开文件: " << output_path << RESET_COLOR << std::endl;
                    continue;
                }
                output_file << report_content;
                files_created++;
            }
        }
        return files_created;
    };

    ExportUtils::execute_export_task("月度报告", export_base_dir, monthly_export_logic);
}

void ActionHandler::run_export_all_period_reports_query(const std::vector<int>& days_list, ReportFormat format) const {
    if (!open_database_if_needed()) {
        std::cerr << RED_COLOR << "错误: 无法打开数据库，导出操作已中止。" << RESET_COLOR << std::endl;
        return;
    }

    QueryHandler query_handler(db_);
    FormattedPeriodReports grouped_reports = query_handler.run_export_all_period_reports_query(days_list, format);

    if (grouped_reports.empty()) {
        std::cout << YELLOW_COLOR << "信息: 数据库中没有可导出的周期报告内容。" << RESET_COLOR << std::endl;
        return;
    }
    
    auto format_details_opt = ExportUtils::get_report_format_details(format);
    if (!format_details_opt) {
        return;
    }
    const auto& format_details = *format_details_opt;

    fs::path export_base_dir = export_root_path_ / format_details.dir_name / "periods";
    fs::create_directories(export_base_dir);

    auto period_export_logic = [&]() -> int {
        int files_created = 0;
        for (const auto& report_pair : grouped_reports) {
            int days = report_pair.first;
            const std::string& report_content = report_pair.second;

            std::stringstream filename_ss;
            filename_ss << "Last_" << days << "_Days_Report" << format_details.extension;
            fs::path output_path = export_base_dir / filename_ss.str();

            std::ofstream output_file(output_path);
            if (!output_file) {
                std::cerr << RED_COLOR << "错误: 无法创建或打开文件: " << output_path << RESET_COLOR << std::endl;
                continue;
            }
            output_file << report_content;
            files_created++;
        }
        return files_created;
    };

    ExportUtils::execute_export_task("周期报告", export_base_dir, period_export_logic);
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

void ActionHandler::run_full_pipeline_and_import(const std::string& source_path) {
    std::cout << "\n--- 开始完整流水线处理 ---" << std::endl;
    close_database();

    if (!collectFiles(source_path)) {
        std::cerr << RED_COLOR << "错误: 文件收集失败，流水线终止。" << RESET_COLOR << std::endl;
        return;
    }

    if (!validateSourceFiles()) {
        std::cerr << RED_COLOR << "错误: 源文件检验失败，流水线终止。" << RESET_COLOR << std::endl;
        return;
    }

    if (!convertFiles()) {
        std::cerr << RED_COLOR << "错误: 文件转换失败，流水线终止。" << RESET_COLOR << std::endl;
        return;
    }

    if (!validateOutputFiles(true)) {
        std::cerr << RED_COLOR << "错误: 输出文件检验失败，流水线终止。" << RESET_COLOR << std::endl;
        return;
    }

    fs::path output_root_path = input_root_.parent_path() / ("Processed_" + input_root_.filename().string());
    run_database_import(output_root_path.string());
    
    std::cout << GREEN_COLOR << "\n成功: 完整流水线处理完毕并已导入数据。" << RESET_COLOR << std::endl;
}


bool ActionHandler::open_database_if_needed() const {
    if (db_ == nullptr) {
        if (sqlite3_open(db_name_.c_str(), (sqlite3**)&db_)) {
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