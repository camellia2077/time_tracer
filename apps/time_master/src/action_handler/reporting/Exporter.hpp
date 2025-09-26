// action_handler/reporting/Exporter.hpp
#ifndef EXPORTER_HPP
#define EXPORTER_HPP

#include <string>
#include <vector>
#include <filesystem>
#include "queries/shared/types/ReportFormat.hpp"
#include "common/AppConfig.hpp" // [ADDED] Include AppConfig

// Forward declaration
class QueryHandler;
namespace fs = std::filesystem;

class Exporter {
public:
    // [MODIFIED] Constructor to accept AppConfig
    Exporter(sqlite3* db, const fs::path& export_root_path, const AppConfig& config);
    
    // --- Single Report Export Methods ---
    void run_export_single_day_report(const std::string& date, ReportFormat format) const;
    void run_export_single_month_report(const std::string& month, ReportFormat format) const;
    void run_export_single_period_report(int days, ReportFormat format) const;

    // --- Bulk Export Methods ---
    void run_export_all_daily_reports_query(ReportFormat format) const;
    void run_export_all_monthly_reports_query(ReportFormat format) const;
    void run_export_all_period_reports_query(const std::vector<int>& days_list, ReportFormat format) const;

private:
    sqlite3* db_;
    fs::path export_root_path_;
    const AppConfig& app_config_; // [ADDED] Store a reference to the AppConfig
};

#endif // EXPORTER_HPP