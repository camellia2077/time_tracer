// action_handler/reporting/ReportExporter.hpp
#ifndef REPORT_EXPORTER_HPP
#define REPORT_EXPORTER_HPP

#include <string>
#include <vector>
#include <filesystem>
#include "queries/shared/ReportFormat.hpp"

// 前向声明
class QueryHandler;
namespace fs = std::filesystem;

/**
 * @brief 报告导出器类
 * * 负责从数据库查询数据并将其导出为不同格式的文件。
 */
class ReportExporter {
public:
    /**
     * @brief 构造函数。
     * @param db 一个有效的 sqlite3 数据库连接指针。
     * @param export_root_path 报告文件的导出根目录。
     */
    ReportExporter(sqlite3* db, const fs::path& export_root_path);
    
    // --- 导出单个报告的方法 ---
    void run_export_single_day_report(const std::string& date, ReportFormat format) const;
    void run_export_single_month_report(const std::string& month, ReportFormat format) const;
    void run_export_single_period_report(int days, ReportFormat format) const;

    // --- 批量导出方法 ---
    void run_export_all_daily_reports_query(ReportFormat format) const;
    void run_export_all_monthly_reports_query(ReportFormat format) const;
    void run_export_all_period_reports_query(const std::vector<int>& days_list, ReportFormat format) const;

private:
    sqlite3* db_; // 不拥有该指针，仅使用
    fs::path export_root_path_;
};

#endif // REPORT_EXPORTER_H