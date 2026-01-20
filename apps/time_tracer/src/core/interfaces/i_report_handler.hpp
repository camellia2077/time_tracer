// core/interfaces/i_report_handler.hpp
#ifndef CORE_INTERFACES_I_REPORT_HANDLER_HPP_
#define CORE_INTERFACES_I_REPORT_HANDLER_HPP_

#include <string>
#include <vector>
#include "reports/shared/types/report_format.hpp"

class IReportHandler {
public:
    virtual ~IReportHandler() = default;

    // 查询方法
    virtual std::string run_daily_query(const std::string& date, ReportFormat format) = 0;
    virtual std::string run_monthly_query(const std::string& month, ReportFormat format) = 0;
    virtual std::string run_period_query(int days, ReportFormat format) = 0;
    
    // [新增] 批量周期查询接口
    virtual std::string run_period_queries(const std::vector<int>& days_list, ReportFormat format) = 0;

    // 导出方法
    virtual void run_export_single_day_report(const std::string& date, ReportFormat format) = 0;
    virtual void run_export_single_month_report(const std::string& month, ReportFormat format) = 0;
    virtual void run_export_single_period_report(int days, ReportFormat format) = 0;
    virtual void run_export_all_daily_reports_query(ReportFormat format) = 0;
    virtual void run_export_all_monthly_reports_query(ReportFormat format) = 0;
    virtual void run_export_all_period_reports_query(const std::vector<int>& days_list, ReportFormat format) = 0;
};

#endif // CORE_INTERFACES_I_REPORT_HANDLER_HPP_