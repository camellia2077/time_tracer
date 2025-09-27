// action_handler/ReportHandler.hpp
#ifndef REPORT_HANDLER_HPP
#define REPORT_HANDLER_HPP

#include <string>
#include <vector>
#include <memory>
#include "reports/shared/types/ReportFormat.hpp"

class Exporter;
// QueryManager 已被移除
class ReportGenerator;

class ReportHandler {
public:
    /**
     * @brief [修改] 构造函数现在只接收 ReportGenerator 和 Exporter。
     */
    ReportHandler(
        std::unique_ptr<ReportGenerator> report_generator,
        std::unique_ptr<Exporter> exporter
    );
    ~ReportHandler();

    // 这些方法现在将通过 ReportGenerator 实现
    std::string run_daily_query(const std::string& date, ReportFormat format);
    std::string run_monthly_query(const std::string& month, ReportFormat format);
    std::string run_period_query(int days, ReportFormat format);

    // 导出方法的声明保持不变
    void run_export_single_day_report(const std::string& date, ReportFormat format);
    void run_export_single_month_report(const std::string& month, ReportFormat format);
    void run_export_single_period_report(int days, ReportFormat format);
    void run_export_all_daily_reports_query(ReportFormat format);
    void run_export_all_monthly_reports_query(ReportFormat format);
    void run_export_all_period_reports_query(const std::vector<int>& days_list, ReportFormat format);

private:
    //移除了 direct_query_manager_
    std::unique_ptr<ReportGenerator> report_generator_;
    std::unique_ptr<Exporter> report_exporter_;
};

#endif // REPORT_HANDLER_HPP