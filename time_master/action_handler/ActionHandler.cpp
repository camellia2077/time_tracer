#include "ActionHandler.h"
#include "QueryHandler.h"
#include "DataImporter.h"
#include "common_utils.h"
#include "LogProcessor.h"
#include "FileUtils.h"
#include <iomanip>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <set>
#include <map>
#include <ctime>
#include <stdexcept> // 用于 std::invalid_argument

namespace fs = std::filesystem;

ActionHandler::ActionHandler(const std::string& db_name, const AppConfig& config, const std::string& main_config_path)
    : db_(nullptr),
      db_name_(db_name),
      app_config_(config),
      main_config_path_(main_config_path),
      processor_(config)
{}

ActionHandler::~ActionHandler() {
    close_database();
}

// [修改] 实现更新后的 run_daily_query 接口
std::string ActionHandler::run_daily_query(const std::string& date, ReportFormat format) const {
    if (!open_database_if_needed()) return "";
    QueryHandler query_handler(db_);
    // 将 format 参数向下传递
    return query_handler.run_daily_query(date, format);
}

// [修改] 实现更新后的 run_period_query 接口
std::string ActionHandler::run_period_query(int days, ReportFormat format) const {
    if (!open_database_if_needed()) return "";
    QueryHandler query_handler(db_);
    // 将 format 参数向下传递给 QueryHandler
    return query_handler.run_period_query(days, format);
}

// [修改] 实现更新后的 run_monthly_query 接口
std::string ActionHandler::run_monthly_query(const std::string& month, ReportFormat format) const {
    if (!open_database_if_needed()) return "";
    QueryHandler query_handler(db_);
    // 将 format 参数向下传递给 QueryHandler
    return query_handler.run_monthly_query(month, format);
}

// [修改] 实现更新后的 run_export_all_daily_reports_query 接口
void ActionHandler::run_export_all_daily_reports_query(ReportFormat format) const {
    if (!open_database_if_needed()) {
        std::cerr << RED_COLOR << "错误: 无法打开数据库，导出操作已中止。" << RESET_COLOR << std::endl;
        return;
    }

    QueryHandler query_handler(db_);
    // 将 format 参数传递给 QueryHandler
    FormattedGroupedReports grouped_reports = query_handler.run_export_all_daily_reports_query(format);

    if (grouped_reports.empty()) {
        std::cout << YELLOW_COLOR << "信息: 数据库中没有可导出的日报内容。" << RESET_COLOR << std::endl;
        return;
    }

    // [新增] 根据格式动态确定文件扩展名和子目录名
    std::string format_str;
    std::string extension;
    switch (format) {
        case ReportFormat::Markdown:
            format_str = "markdown";
            extension = ".md";
            break;
        // case ReportFormat::Json: // 未来若支持JSON，取消此段注释
        //     format_str = "json";
        //     extension = ".json";
        //     break;
        default:
            std::cerr << RED_COLOR << "错误: 不支持的导出格式。" << RESET_COLOR << std::endl;
            return;
    }

    int files_created = 0;
    try {
        for (const auto& year_pair : grouped_reports) {
            int year = year_pair.first;
            for (const auto& month_pair : year_pair.second) {
                int month = month_pair.first;

                // [修改] 使用动态格式字符串创建目录路径
                std::stringstream month_ss;
                month_ss << std::setw(2) << std::setfill('0') << month;
                fs::path month_dir = fs::path("Export") / format_str / "days" / std::to_string(year) / month_ss.str();
                fs::create_directories(month_dir);

                for (const auto& report_pair : month_pair.second) {
                    const std::string& date_str = report_pair.first;
                    const std::string& report_content = report_pair.second;
                    
                    // [修改] 使用动态扩展名创建报告文件名
                    fs::path output_path = month_dir / (date_str + extension);
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

        if (files_created > 0) {
            fs::path final_path = fs::absolute(fs::path("Export") / format_str / "days");
            std::cout << GREEN_COLOR << "成功: 共创建 " << files_created << " 个日报文件，已保存至: " << final_path.string() << RESET_COLOR << std::endl;
        } else {
             std::cout << YELLOW_COLOR << "信息: 没有可导出的日报内容。" << RESET_COLOR << std::endl;
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << RED_COLOR << "文件系统错误: " << e.what() << RESET_COLOR << std::endl;
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "导出过程中发生未知错误: " << e.what() << RESET_COLOR << std::endl;
    }
}

void ActionHandler::run_export_all_monthly_reports_query(ReportFormat format) const {
    if (!open_database_if_needed()) {
        std::cerr << RED_COLOR << "错误: 无法打开数据库，导出操作已中止。" << RESET_COLOR << std::endl;
        return;
    }

    QueryHandler query_handler(db_);
    // 将 format 参数传递给 QueryHandler
    FormattedMonthlyReports grouped_reports = query_handler.run_export_all_monthly_reports_query(format);

    if (grouped_reports.empty()) {
        std::cout << YELLOW_COLOR << "信息: 数据库中没有可导出的月报内容。" << RESET_COLOR << std::endl;
        return;
    }

    // [修改] 根据格式动态确定文件扩展名和子目录名，移除硬编码
    std::string format_str;
    std::string extension;
    switch (format) {
        case ReportFormat::Markdown:
            format_str = "markdown";
            extension = ".md";
            break;
        // case ReportFormat::Json: // 未来若支持JSON，取消此段注释
        //     format_str = "json";
        //     extension = ".json";
        //     break;
        default:
            std::cerr << RED_COLOR << "错误: 不支持的导出格式。" << RESET_COLOR << std::endl;
            return;
    }
    
    int files_created = 0;
    try {
        for (const auto& year_pair : grouped_reports) {
            int year = year_pair.first;
            const auto& monthly_reports = year_pair.second;
            
            // [修改] 使用动态格式字符串创建目录路径
            fs::path year_dir = fs::path("Export") / format_str / "months" / std::to_string(year);
            fs::create_directories(year_dir);

            for (const auto& month_pair : monthly_reports) {
                int month = month_pair.first;
                const std::string& report_content = month_pair.second;

                std::stringstream filename_ss;
                filename_ss << year << "_" << std::setw(2) << std::setfill('0') << month << extension;
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
        if (files_created > 0) {
            // [修改] 使用动态格式字符串创建最终路径
            fs::path final_path = fs::absolute(fs::path("Export") / format_str / "months");
            std::cout << GREEN_COLOR << "成功: 共创建 " << files_created << " 个月度报告文件，已保存至: " << final_path.string() << RESET_COLOR << std::endl;
        } else {
             std::cout << YELLOW_COLOR << "信息: 没有可导出的月报内容。" << RESET_COLOR << std::endl;
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << RED_COLOR << "文件系统错误: " << e.what() << RESET_COLOR << std::endl;
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "导出过程中发生未知错误: " << e.what() << RESET_COLOR << std::endl;
    }
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
// [修改] run_export_all_period_reports_query 现在接收 format 参数并动态处理
void ActionHandler::run_export_all_period_reports_query(const std::vector<int>& days_list, ReportFormat format) const {
    if (!open_database_if_needed()) {
        std::cerr << RED_COLOR << "错误: 无法打开数据库，导出操作已中止。" << RESET_COLOR << std::endl;
        return;
    }

    QueryHandler query_handler(db_);
    // 将 format 参数传递给 QueryHandler
    FormattedPeriodReports grouped_reports = query_handler.run_export_all_period_reports_query(days_list, format);

    if (grouped_reports.empty()) {
        std::cout << YELLOW_COLOR << "信息: 数据库中没有可导出的周期报告内容。" << RESET_COLOR << std::endl;
        return;
    }

    // [修改] 根据格式动态确定文件扩展名和子目录名，移除硬编码
    std::string format_str;
    std::string extension;
    switch (format) {
        case ReportFormat::Markdown:
            format_str = "markdown";
            extension = ".md";
            break;
        // case ReportFormat::Json:
        //     format_str = "json";
        //     extension = ".json";
        //     break;
        default:
            std::cerr << RED_COLOR << "错误: 不支持的导出格式。" << RESET_COLOR << std::endl;
            return;
    }

    int files_created = 0;
    try {
        // [修改] 使用动态格式字符串创建目录路径
        fs::path export_dir = fs::path("Export") / format_str / "periods";
        fs::create_directories(export_dir);

        for (const auto& report_pair : grouped_reports) {
            int days = report_pair.first;
            const std::string& report_content = report_pair.second;

            std::stringstream filename_ss;
            filename_ss << "Last_" << days << "_Days_Report" << extension;
            fs::path output_path = export_dir / filename_ss.str();

            std::ofstream output_file(output_path);
            if (!output_file) {
                std::cerr << RED_COLOR << "错误: 无法创建或打开文件: " << output_path << RESET_COLOR << std::endl;
                continue;
            }
            output_file << report_content;
            files_created++;
        }

        if (files_created > 0) {
            fs::path final_path = fs::absolute(export_dir);
            std::cout << GREEN_COLOR << "成功: 共创建 " << files_created << " 个周期报告文件，已保存至: " << final_path.string() << RESET_COLOR << std::endl;
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << RED_COLOR << "文件系统错误: " << e.what() << RESET_COLOR << std::endl;
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "导出过程中发生未知错误: " << e.what() << RESET_COLOR << std::endl;
    }
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