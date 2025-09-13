// action_handler/reporting/ReportExporter.cpp

#include "ReportExporter.hpp"
#include "ExportUtils.hpp"
#include "queries/QueryHandler.hpp"
#include "common/AnsiColors.hpp" // For colored console output

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

/**
 * @brief 构造函数。
 * @param db 一个有效的 sqlite3 数据库连接指针。
 * @param export_root_path 报告文件的导出根目录。
 */
ReportExporter::ReportExporter(sqlite3* db, const fs::path& export_root_path)
    : db_(db), export_root_path_(export_root_path) {}

/**
 * @brief 导出单个指定日期的日报。
 * @param date 日期字符串 (例如, "2023-10-27")。
 * @param format 报告的格式 (例如, Markdown)。
 */
void ReportExporter::run_export_single_day_report(const std::string& date, ReportFormat format) const {
    QueryHandler query_handler(db_);
    std::string report_content = query_handler.run_daily_query(date, format);
    
    if (report_content.empty() || report_content.find("No time records") != std::string::npos) {
        std::cout << YELLOW_COLOR << "信息: 没有为日期 " << date << " 找到可导出的内容。" << RESET_COLOR << std::endl;
        return;
    }

    // 使用 ExportUtils 获取格式详情
    auto format_details_opt = ExportUtils::get_report_format_details(format);
    if (!format_details_opt) return;
    const auto& format_details = *format_details_opt;

    // 构建输出路径
    fs::path output_dir = export_root_path_ / format_details.dir_name / "daily";
    fs::path output_path = output_dir / (date + format_details.extension);

    try {
        fs::create_directories(output_dir);
    } catch (const fs::filesystem_error& e) {
        std::cerr << RED_COLOR << "错误: 创建目录失败: " << output_dir << " - " << e.what() << RESET_COLOR << std::endl;
        return;
    }

    std::ofstream output_file(output_path);
    if (!output_file) {
        std::cerr << RED_COLOR << "错误: 无法创建或打开文件: " << output_path << RESET_COLOR << std::endl;
        return;
    }
    
    output_file << report_content;
    std::cout << GREEN_COLOR << "成功: 日报已成功导出到 " << fs::absolute(output_path) << RESET_COLOR << std::endl;
}

/**
 * @brief 导出单个指定月份的月报。
 * @param month 月份字符串 (例如, "2023-10")。
 * @param format 报告的格式。
 */
void ReportExporter::run_export_single_month_report(const std::string& month, ReportFormat format) const {
    QueryHandler query_handler(db_);
    std::string report_content = query_handler.run_monthly_query(month, format);

    if (report_content.empty() || report_content.find("No time records") != std::string::npos) {
        std::cout << YELLOW_COLOR << "信息: 没有为月份 " << month << " 找到可导出的内容。" << RESET_COLOR << std::endl;
        return;
    }

    auto format_details_opt = ExportUtils::get_report_format_details(format);
    if (!format_details_opt) return;
    const auto& format_details = *format_details_opt;

    fs::path output_dir = export_root_path_ / format_details.dir_name / "monthly";
    fs::path output_path = output_dir / (month + format_details.extension);

    try {
        fs::create_directories(output_dir);
    } catch (const fs::filesystem_error& e) {
        std::cerr << RED_COLOR << "错误: 创建目录失败: " << output_dir << " - " << e.what() << RESET_COLOR << std::endl;
        return;
    }

    std::ofstream output_file(output_path);
    if (!output_file) {
        std::cerr << RED_COLOR << "错误: 无法创建或打开文件: " << output_path << RESET_COLOR << std::endl;
        return;
    }
    
    output_file << report_content;
    std::cout << GREEN_COLOR << "成功: 月报已成功导出到 " << fs::absolute(output_path) << RESET_COLOR << std::endl;
}

/**
 * @brief 导出单个指定天数周期的报告。
 * @param days 过去的天数。
 * @param format 报告的格式。
 */
void ReportExporter::run_export_single_period_report(int days, ReportFormat format) const {
    QueryHandler query_handler(db_);
    std::string report_content = query_handler.run_period_query(days, format);

    if (report_content.empty() || report_content.find("No time records") != std::string::npos) {
        std::cout << YELLOW_COLOR << "信息: 没有为 " << days << " 天周期找到可导出的内容。" << RESET_COLOR << std::endl;
        return;
    }
    
    auto format_details_opt = ExportUtils::get_report_format_details(format);
    if (!format_details_opt) return;
    const auto& format_details = *format_details_opt;

    fs::path output_dir = export_root_path_ / format_details.dir_name / "periods";
    fs::path output_path = output_dir / ("Last_" + std::to_string(days) + "_Days_Report" + format_details.extension);

    try {
        fs::create_directories(output_dir);
    } catch (const fs::filesystem_error& e) {
        std::cerr << RED_COLOR << "错误: 创建目录失败: " << output_dir << " - " << e.what() << RESET_COLOR << std::endl;
        return;
    }

    std::ofstream output_file(output_path);
    if (!output_file) {
        std::cerr << RED_COLOR << "错误: 无法创建或打开文件: " << output_path << RESET_COLOR << std::endl;
        return;
    }
    
    output_file << report_content;
    std::cout << GREEN_COLOR << "成功: 周期报告已成功导出到 " << fs::absolute(output_path) << RESET_COLOR << std::endl;
}

/**
 * @brief 批量导出数据库中所有日报。
 * @param format 报告的格式。
 */
void ReportExporter::run_export_all_daily_reports_query(ReportFormat format) const {
    QueryHandler query_handler(db_);
    FormattedGroupedReports grouped_reports = query_handler.run_export_all_daily_reports_query(format);

    if (grouped_reports.empty()) {
        std::cout << YELLOW_COLOR << "信息: 数据库中没有可导出的日报内容。" << RESET_COLOR << std::endl;
        return;
    }

    auto format_details_opt = ExportUtils::get_report_format_details(format);
    if (!format_details_opt) return;
    const auto& format_details = *format_details_opt;

    fs::path export_base_dir = export_root_path_ / format_details.dir_name / "days";    

    // 定义文件写入的 Lambda 表达式
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

/**
 * @brief 批量导出数据库中所有月报。
 * @param format 报告的格式。
 */
void ReportExporter::run_export_all_monthly_reports_query(ReportFormat format) const {
    QueryHandler query_handler(db_);
    FormattedMonthlyReports grouped_reports = query_handler.run_export_all_monthly_reports_query(format);

    if (grouped_reports.empty()) {
        std::cout << YELLOW_COLOR << "信息: 数据库中没有可导出的月报内容。" << RESET_COLOR << std::endl;
        return;
    }

    auto format_details_opt = ExportUtils::get_report_format_details(format);
    if (!format_details_opt) return;
    const auto& format_details = *format_details_opt;

    fs::path export_base_dir = export_root_path_ / format_details.dir_name / "monthly";

    auto monthly_export_logic = [&]() -> int {
        int files_created = 0;
        for (const auto& year_pair : grouped_reports) {
            fs::path year_dir = export_base_dir / std::to_string(year_pair.first);
            fs::create_directories(year_dir);
            for (const auto& month_pair : year_pair.second) {
                fs::path output_path = year_dir / ((std::stringstream() << year_pair.first << "_" << std::setw(2) << std::setfill('0') << month_pair.first).str() + format_details.extension);
                std::ofstream output_file(output_path);
                if (output_file) {
                    output_file << month_pair.second;
                    files_created++;
                } else {
                    std::cerr << RED_COLOR << "错误: 无法创建或打开文件: " << output_path << RESET_COLOR << std::endl;
                }
            }
        }
        return files_created;
    };

    ExportUtils::execute_export_task("月报", export_base_dir, monthly_export_logic);
}

/**
 * @brief 批量导出指定周期列表的所有报告。
 * @param days_list 包含多个天数的向量。
 * @param format 报告的格式。
 */
void ReportExporter::run_export_all_period_reports_query(const std::vector<int>& days_list, ReportFormat format) const {
    QueryHandler query_handler(db_);
    FormattedPeriodReports grouped_reports = query_handler.run_export_all_period_reports_query(days_list, format);

    if (grouped_reports.empty()) {
        std::cout << YELLOW_COLOR << "信息: 数据库中没有可导出的周期报告内容。" << RESET_COLOR << std::endl;
        return;
    }

    auto format_details_opt = ExportUtils::get_report_format_details(format);
    if (!format_details_opt) return;
    const auto& format_details = *format_details_opt;

    fs::path export_base_dir = export_root_path_ / format_details.dir_name / "periods";
    fs::create_directories(export_base_dir);

    auto period_export_logic = [&]() -> int {
        int files_created = 0;
        for (const auto& report_pair : grouped_reports) {
            fs::path output_path = export_base_dir / ("Last_" + std::to_string(report_pair.first) + "_Days_Report" + format_details.extension);
            std::ofstream output_file(output_path);
            if (output_file) {
                output_file << report_pair.second;
                files_created++;
            } else {
                std::cerr << RED_COLOR << "错误: 无法创建或打开文件: " << output_path << RESET_COLOR << std::endl;
            }
        }
        return files_created;
    };

    ExportUtils::execute_export_task("周期报告", export_base_dir, period_export_logic);
}