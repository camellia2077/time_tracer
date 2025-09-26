// action_handler/ReportHandler.cpp
#include "action_handler/ReportHandler.hpp"
#include "action_handler/reporting/Exporter.hpp"
#include "action_handler/query/QueryManager.hpp"
#include "common/AnsiColors.hpp"
#include <iostream>

// [核心修改] 构造函数接收依赖项的所有权 (std::move)
ReportHandler::ReportHandler(
    std::unique_ptr<QueryManager> query_manager,
    std::unique_ptr<Exporter> exporter)
    : direct_query_manager_(std::move(query_manager)),
      report_exporter_(std::move(exporter))
{
}

ReportHandler::~ReportHandler() = default;

// --- 所有公共方法的实现都得到简化 ---

std::string ReportHandler::run_daily_query(const std::string& date, ReportFormat format) {
    if (direct_query_manager_) {
        return direct_query_manager_->run_daily_query(date, format);
    }
    return std::string(RED_COLOR) + "Error: QueryManager not available." + RESET_COLOR;
}

std::string ReportHandler::run_monthly_query(const std::string& month, ReportFormat format) {
    if (direct_query_manager_) {
        return direct_query_manager_->run_monthly_query(month, format);
    }
    return std::string(RED_COLOR) + "Error: QueryManager not available." + RESET_COLOR;
}

std::string ReportHandler::run_period_query(int days, ReportFormat format) {
    if (direct_query_manager_) {
        return direct_query_manager_->run_period_query(days, format);
    }
    return std::string(RED_COLOR) + "Error: QueryManager not available." + RESET_COLOR;
}

void ReportHandler::run_export_single_day_report(const std::string& date, ReportFormat format) {
    if (report_exporter_) {
        report_exporter_->run_export_single_day_report(date, format);
    } else {
        std::cerr << RED_COLOR << "Error: Exporter not available." << RESET_COLOR << std::endl;
    }
}

void ReportHandler::run_export_single_month_report(const std::string& month, ReportFormat format) {
    if (report_exporter_) {
        report_exporter_->run_export_single_month_report(month, format);
    } else {
        std::cerr << RED_COLOR << "Error: Exporter not available." << RESET_COLOR << std::endl;
    }
}

void ReportHandler::run_export_single_period_report(int days, ReportFormat format) {
    if (report_exporter_) {
        report_exporter_->run_export_single_period_report(days, format);
    } else {
        std::cerr << RED_COLOR << "Error: Exporter not available." << RESET_COLOR << std::endl;
    }
}

void ReportHandler::run_export_all_daily_reports_query(ReportFormat format) {
    if (report_exporter_) {
        report_exporter_->run_export_all_daily_reports_query(format);
    } else {
        std::cerr << RED_COLOR << "Error: Exporter not available." << RESET_COLOR << std::endl;
    }
}

void ReportHandler::run_export_all_monthly_reports_query(ReportFormat format) {
    if (report_exporter_) {
        report_exporter_->run_export_all_monthly_reports_query(format);
    } else {
        std::cerr << RED_COLOR << "Error: Exporter not available." << RESET_COLOR << std::endl;
    }
}

void ReportHandler::run_export_all_period_reports_query(const std::vector<int>& days_list, ReportFormat format) {
    if (report_exporter_) {
        report_exporter_->run_export_all_period_reports_query(days_list, format);
    } else {
        std::cerr << RED_COLOR << "Error: Exporter not available." << RESET_COLOR << std::endl;
    }
}