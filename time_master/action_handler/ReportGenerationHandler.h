#ifndef REPORT_GENERATION_HANDLER_H
#define REPORT_GENERATION_HANDLER_H

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include "queries/shared/query_data_structs.h"
#include "queries/shared/ReportFormat.h"
#include "common/AppConfig.h"

class DatabaseManager;
class ReportExporter;
class DirectQueryManager;

class ReportGenerationHandler {
public:
    ReportGenerationHandler(const std::string& db_name, const AppConfig& config);
    ~ReportGenerationHandler();

    std::string run_daily_query(const std::string& date, ReportFormat format);
    std::string run_monthly_query(const std::string& month, ReportFormat format);
    std::string run_period_query(int days, ReportFormat format);

    void run_export_single_day_report(const std::string& date, ReportFormat format);
    void run_export_single_month_report(const std::string& month, ReportFormat format);
    void run_export_single_period_report(int days, ReportFormat format);
    void run_export_all_daily_reports_query(ReportFormat format);
    void run_export_all_monthly_reports_query(ReportFormat format);
    void run_export_all_period_reports_query(const std::vector<int>& days_list, ReportFormat format);

private:
    DirectQueryManager* get_direct_query_manager();
    ReportExporter* get_report_exporter();

    std::unique_ptr<DatabaseManager> db_manager_;
    std::unique_ptr<ReportExporter> report_exporter_;
    std::unique_ptr<DirectQueryManager> direct_query_manager_;

    std::filesystem::path export_root_path_;
};

#endif // REPORT_GENERATION_HANDLER_H