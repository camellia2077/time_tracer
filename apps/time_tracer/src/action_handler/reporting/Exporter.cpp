// action_handler/reporting/Exporter.cpp
#include "Exporter.hpp"
#include "ExportUtils.hpp"
#include "ReportFileManager.hpp"
#include "common/AnsiColors.hpp"
#include <fstream>
#include <iostream>

Exporter::Exporter(const fs::path& export_root_path) {
    file_manager_ = std::make_unique<ReportFileManager>(export_root_path);
}

Exporter::~Exporter() = default;

void Exporter::write_report_to_file(const std::string& report_content, const fs::path& output_path) const {
    if (report_content.empty() || report_content.find("No time records") != std::string::npos) {
        // 对于批量导出，单个文件的跳过信息可以不打印，以免刷屏
        // 对于单个文件导出，打印提示是合适的
        // std::cout << YELLOW_COLOR << "Info: No exportable content for " << output_path.stem().string() << "." << RESET_COLOR << std::endl;
        return;
    }

    fs::path output_dir = output_path.parent_path();
    try {
        fs::create_directories(output_dir);
    } catch (const fs::filesystem_error& e) {
        std::cerr << RED_COLOR << "Error: Failed to create directory: " << output_dir << " - " << e.what() << RESET_COLOR << std::endl;
        return; // 在批量任务中，一个目录创建失败不应阻止其他任务
    }

    std::ofstream output_file(output_path);
    if (!output_file) {
        std::cerr << RED_COLOR << "Error: Could not create or open file: " << output_path << RESET_COLOR << std::endl;
        return;
    }
    
    output_file << report_content;
    // 在批量导出中，成功信息由 ExportUtils::execute_export_task 统一打印
}

// --- 单文件导出方法保持不变 ---

void Exporter::export_single_day_report(const std::string& date, const std::string& content, ReportFormat format) const {
    fs::path path = file_manager_->get_single_day_report_path(date, format);
    std::cout << GREEN_COLOR << "Success: Report exported to " << fs::absolute(path) << RESET_COLOR << std::endl;
    write_report_to_file(content, path);
}

void Exporter::export_single_month_report(const std::string& month, const std::string& content, ReportFormat format) const {
    fs::path path = file_manager_->get_single_month_report_path(month, format);
    std::cout << GREEN_COLOR << "Success: Report exported to " << fs::absolute(path) << RESET_COLOR << std::endl;
    write_report_to_file(content, path);
}

void Exporter::export_single_period_report(int days, const std::string& content, ReportFormat format) const {
    fs::path path = file_manager_->get_single_period_report_path(days, format);
    std::cout << GREEN_COLOR << "Success: Report exported to " << fs::absolute(path) << RESET_COLOR << std::endl;
    write_report_to_file(content, path);
}


// --- [核心实现] 批量导出方法 ---

void Exporter::export_all_daily_reports(const FormattedGroupedReports& reports, ReportFormat format) const {
    fs::path base_dir = file_manager_->get_all_daily_reports_base_dir(format);
    
    ExportUtils::execute_export_task("日报", base_dir, [&]() {
        int files_created = 0;
        for (const auto& year_pair : reports) {
            for (const auto& month_pair : year_pair.second) {
                for (const auto& day_report : month_pair.second) {
                    const std::string& date = day_report.first;
                    const std::string& content = day_report.second;
                    fs::path report_path = file_manager_->get_single_day_report_path(date, format);
                    write_report_to_file(content, report_path);
                    if (!content.empty()) files_created++;
                }
            }
        }
        return files_created;
    });
}

void Exporter::export_all_monthly_reports(const FormattedMonthlyReports& reports, ReportFormat format) const {
    fs::path base_dir = file_manager_->get_all_monthly_reports_base_dir(format);

    ExportUtils::execute_export_task("月报", base_dir, [&]() {
        int files_created = 0;
        for (const auto& year_pair : reports) {
            int year = year_pair.first;
            for (const auto& month_pair : year_pair.second) {
                int month = month_pair.first;
                const std::string& content = month_pair.second;
                std::string month_str = std::to_string(year) + (month < 10 ? "0" : "") + std::to_string(month);
                
                fs::path report_path = file_manager_->get_single_month_report_path(month_str, format);
                write_report_to_file(content, report_path);
                if (!content.empty()) files_created++;
            }
        }
        return files_created;
    });
}

void Exporter::export_all_period_reports(const FormattedPeriodReports& reports, ReportFormat format) const {
    fs::path base_dir = file_manager_->get_all_period_reports_base_dir(format);
    
    ExportUtils::execute_export_task("周期报告", base_dir, [&]() {
        int files_created = 0;
        for (const auto& report_pair : reports) {
            int days = report_pair.first;
            const std::string& content = report_pair.second;
            fs::path report_path = file_manager_->get_single_period_report_path(days, format);
            write_report_to_file(content, report_path);
            if (!content.empty()) files_created++;
        }
        return files_created;
    });
}