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
        std::cout << YELLOW_COLOR << "Info: No exportable content for " << output_path.stem().string() << "." << RESET_COLOR << std::endl;
        return;
    }

    fs::path output_dir = output_path.parent_path();
    try {
        fs::create_directories(output_dir);
    } catch (const fs::filesystem_error& e) {
        std::cerr << RED_COLOR << "Error: Failed to create directory: " << output_dir << " - " << e.what() << RESET_COLOR << std::endl;
        return;
    }

    std::ofstream output_file(output_path);
    if (!output_file) {
        std::cerr << RED_COLOR << "Error: Could not create or open file: " << output_path << RESET_COLOR << std::endl;
        return;
    }
    
    output_file << report_content;
    std::cout << GREEN_COLOR << "Success: Report exported to " << fs::absolute(output_path) << RESET_COLOR << std::endl;
}

void Exporter::export_single_day_report(const std::string& date, const std::string& content, ReportFormat format) const {
    fs::path path = file_manager_->get_single_day_report_path(date, format);
    write_report_to_file(content, path);
}

void Exporter::export_single_month_report(const std::string& month, const std::string& content, ReportFormat format) const {
    fs::path path = file_manager_->get_single_month_report_path(month, format);
    write_report_to_file(content, path);
}

void Exporter::export_single_period_report(int days, const std::string& content, ReportFormat format) const {
    fs::path path = file_manager_->get_single_period_report_path(days, format);
    write_report_to_file(content, path);
}

void Exporter::export_all_daily_reports(const FormattedGroupedReports& reports, ReportFormat format) const {
    // Mark parameters as unused to prevent compiler errors.
    (void)reports;
    (void)format;
    // Implementation for exporting all daily reports
}

void Exporter::export_all_monthly_reports(const FormattedMonthlyReports& reports, ReportFormat format) const {
    // Mark parameters as unused to prevent compiler errors.
    (void)reports;
    (void)format;
    // Implementation for exporting all monthly reports
}

void Exporter::export_all_period_reports(const FormattedPeriodReports& reports, ReportFormat format) const {
    // Mark parameters as unused to prevent compiler errors.
    (void)reports;
    (void)format;
    // Implementation for exporting all period reports
}