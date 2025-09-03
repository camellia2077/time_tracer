#ifndef REPORT_GENERATION_HANDLER_HPP
#define REPORT_GENERATION_HANDLER_HPP

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include "queries/shared/data/query_data_structs.hpp"
#include "queries/shared/ReportFormat.hpp"
#include "common/AppConfig.hpp"

class DatabaseManager;
class ReportExporter;
class DirectQueryManager;

class ReportGenerationHandler {
public:
    // 修改构造函数以接收报告专用路径
    ReportGenerationHandler(const std::string& db_path, const AppConfig& config, const std::filesystem::path& exported_files_path);
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

    std::filesystem::path export_root_path_; // 这个成员变量可以保持，但现在代表报告专用路径
};

#endif // REPORT_GENERATION_HANDLER_HPP