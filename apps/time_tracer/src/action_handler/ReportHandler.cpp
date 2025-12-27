// action_handler/ReportHandler.cpp
#include "action_handler/ReportHandler.hpp"
#include "action_handler/reporting/Exporter.hpp"
// 移除了 #include "action_handler/query/QueryManager.hpp"
#include "action_handler/reporting/ReportGenerator.hpp"
#include "common/AnsiColors.hpp"
#include <iostream>

/**
 * @brief [修改] 构造函数不再接收 QueryManager。
 */
ReportHandler::ReportHandler(
    std::unique_ptr<ReportGenerator> report_generator,
    std::unique_ptr<Exporter> exporter)
    : report_generator_(std::move(report_generator)),
      report_exporter_(std::move(exporter))
{}

ReportHandler::~ReportHandler() = default;

/**
 * @brief [修改] 实现已从 QueryManager 迁移到 ReportGenerator。
 */
std::string ReportHandler::run_daily_query(const std::string& date, ReportFormat format) {
    if (report_generator_) {
        return report_generator_->generate_daily_report(date, format);
    }
    return std::string(RED_COLOR) + "Error: ReportGenerator not available." + RESET_COLOR;
}

/**
 * @brief [修改] 实现已从 QueryManager 迁移到 ReportGenerator。
 */
std::string ReportHandler::run_monthly_query(const std::string& month, ReportFormat format) {
    if (report_generator_) {
        return report_generator_->generate_monthly_report(month, format);
    }
    return std::string(RED_COLOR) + "Error: ReportGenerator not available." + RESET_COLOR;
}

/**
 * @brief [修改] 实现已从 QueryManager 迁移到 ReportGenerator。
 */
std::string ReportHandler::run_period_query(int days, ReportFormat format) {
    if (report_generator_) {
        return report_generator_->generate_period_report(days, format);
    }
    return std::string(RED_COLOR) + "Error: ReportGenerator not available." + RESET_COLOR;
}

// --- 导出相关方法保持不变，它们已经在使用正确的依赖 ---

void ReportHandler::run_export_single_day_report(const std::string& date, ReportFormat format) {
    if (report_generator_ && report_exporter_) {
        std::string report_content = report_generator_->generate_daily_report(date, format);
        report_exporter_->export_single_day_report(date, report_content, format);
    } else {
        std::cerr << RED_COLOR << "Error: ReportGenerator or Exporter not available." << RESET_COLOR << std::endl;
    }
}

void ReportHandler::run_export_single_month_report(const std::string& month, ReportFormat format) {
    if (report_generator_ && report_exporter_) {
        std::string report_content = report_generator_->generate_monthly_report(month, format);
        report_exporter_->export_single_month_report(month, report_content, format);
    } else {
        std::cerr << RED_COLOR << "Error: ReportGenerator or Exporter not available." << RESET_COLOR << std::endl;
    }
}

void ReportHandler::run_export_single_period_report(int days, ReportFormat format) {
    if (report_generator_ && report_exporter_) {
        std::string report_content = report_generator_->generate_period_report(days, format);
        report_exporter_->export_single_period_report(days, report_content, format);
    } else {
        std::cerr << RED_COLOR << "Error: ReportGenerator or Exporter not available." << RESET_COLOR << std::endl;
    }
}

void ReportHandler::run_export_all_daily_reports_query(ReportFormat format) {
    if (report_generator_ && report_exporter_) {
        FormattedGroupedReports reports = report_generator_->generate_all_daily_reports(format);
        report_exporter_->export_all_daily_reports(reports, format);
    } else {
        std::cerr << RED_COLOR << "Error: ReportGenerator or Exporter not available." << RESET_COLOR << std::endl;
    }
}

void ReportHandler::run_export_all_monthly_reports_query(ReportFormat format) {
    if (report_generator_ && report_exporter_) {
        FormattedMonthlyReports reports = report_generator_->generate_all_monthly_reports(format);
        report_exporter_->export_all_monthly_reports(reports, format);
    } else {
        std::cerr << RED_COLOR << "Error: ReportGenerator or Exporter not available." << RESET_COLOR << std::endl;
    }
}

void ReportHandler::run_export_all_period_reports_query(const std::vector<int>& days_list, ReportFormat format) {
    if (report_generator_ && report_exporter_) {
        FormattedPeriodReports reports = report_generator_->generate_all_period_reports(days_list, format);
        report_exporter_->export_all_period_reports(reports, format);
    } else {
        std::cerr << RED_COLOR << "Error: ReportGenerator or Exporter not available." << RESET_COLOR << std::endl;
    }
}