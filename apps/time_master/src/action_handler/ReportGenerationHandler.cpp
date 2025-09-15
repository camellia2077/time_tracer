// action_handler/ReportGenerationHandler.cpp
#include "action_handler/ReportGenerationHandler.hpp"
#include "action_handler/database/DatabaseManager.hpp"
#include "action_handler/reporting/ReportExporter.hpp"
#include "action_handler/query/DirectQueryManager.hpp"
#include "common/AnsiColors.hpp" // For colored console output
#include <iostream>

namespace fs = std::filesystem;

ReportGenerationHandler::ReportGenerationHandler(const std::string& db_path, const AppConfig& config, const fs::path& exported_files_path)
    : app_config_(config),
      export_root_path_(exported_files_path)
{
    db_manager_ = std::make_unique<DatabaseManager>(db_path);
}

ReportGenerationHandler::~ReportGenerationHandler() = default;

DirectQueryManager* ReportGenerationHandler::get_direct_query_manager() {
    if (!db_manager_->open_database_if_needed()) {
        std::cerr << RED_COLOR << "Error: Could not open database, query operation aborted." << RESET_COLOR << std::endl;
        return nullptr;
    }
    if (!direct_query_manager_) {
        direct_query_manager_ = std::make_unique<DirectQueryManager>(db_manager_->get_db_connection(), app_config_);
    }
    return direct_query_manager_.get();
}

ReportExporter* ReportGenerationHandler::get_report_exporter() {
    if (!db_manager_->open_database_if_needed()) {
        std::cerr << RED_COLOR << "Error: Could not open database, export operation aborted." << RESET_COLOR << std::endl;
        return nullptr;
    }
    if (!report_exporter_) {
        // [MODIFIED] Pass the AppConfig to the ReportExporter
        report_exporter_ = std::make_unique<ReportExporter>(db_manager_->get_db_connection(), export_root_path_, app_config_);
    }
    return report_exporter_.get();
}

std::string ReportGenerationHandler::run_daily_query(const std::string& date, ReportFormat format) {
    if (auto* qm = get_direct_query_manager()) {
        return qm->run_daily_query(date, format);
    }
    return std::string(RED_COLOR) + "Error: Query failed." + RESET_COLOR;
}

std::string ReportGenerationHandler::run_monthly_query(const std::string& month, ReportFormat format) {
    if (auto* qm = get_direct_query_manager()) {
        return qm->run_monthly_query(month, format);
    }
    return std::string(RED_COLOR) + "Error: Query failed." + RESET_COLOR;
}

std::string ReportGenerationHandler::run_period_query(int days, ReportFormat format) {
    if (auto* qm = get_direct_query_manager()) {
        return qm->run_period_query(days, format);
    }
    return std::string(RED_COLOR) + "Error: Query failed." + RESET_COLOR;
}

void ReportGenerationHandler::run_export_single_day_report(const std::string& date, ReportFormat format) {
    if (auto* exporter = get_report_exporter()) {
        exporter->run_export_single_day_report(date, format);
    }
}

void ReportGenerationHandler::run_export_single_month_report(const std::string& month, ReportFormat format) {
    if (auto* exporter = get_report_exporter()) {
        exporter->run_export_single_month_report(month, format);
    }
}

void ReportGenerationHandler::run_export_single_period_report(int days, ReportFormat format) {
    if (auto* exporter = get_report_exporter()) {
        exporter->run_export_single_period_report(days, format);
    }
}

void ReportGenerationHandler::run_export_all_daily_reports_query(ReportFormat format) {
    if (auto* exporter = get_report_exporter()) {
        exporter->run_export_all_daily_reports_query(format);
    }
}

void ReportGenerationHandler::run_export_all_monthly_reports_query(ReportFormat format) {
    if (auto* exporter = get_report_exporter()) {
        exporter->run_export_all_monthly_reports_query(format);
    }
}

void ReportGenerationHandler::run_export_all_period_reports_query(const std::vector<int>& days_list, ReportFormat format) {
    if (auto* exporter = get_report_exporter()) {
        exporter->run_export_all_period_reports_query(days_list, format);
    }
}