// action_handler/ReportHandler.hpp
#ifndef REPORT_GENERATION_HANDLER_HPP
#define REPORT_GENERATION_HANDLER_HPP

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include "queries/shared/data/query_data_structs.hpp"
#include "queries/shared/types/ReportFormat.hpp"
#include "common/AppConfig.hpp"

class DBManager;
class Exporter;
class QueryManager;

class ReportHandler {
public:
    // Modify constructor to receive and store the AppConfig
    ReportHandler(const std::string& db_path, const AppConfig& config, const std::filesystem::path& exported_files_path);
    ~ReportHandler();

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
    QueryManager* get_direct_query_manager();
    Exporter* get_report_exporter();

    const AppConfig& app_config_; // [ADDED] Store a reference to the config
    std::unique_ptr<DBManager> db_manager_;
    std::unique_ptr<Exporter> report_exporter_;
    std::unique_ptr<QueryManager> direct_query_manager_;

    std::filesystem::path export_root_path_;
};

#endif // REPORT_GENERATION_HANDLER_HPP