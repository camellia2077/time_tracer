#ifndef CORE_REPORTING_REPORT_HANDLER_HPP_
#define CORE_REPORTING_REPORT_HANDLER_HPP_

#include "core/interfaces/i_report_handler.hpp" // [新增]
#include <string>
#include <vector>
#include <memory>
#include "reports/shared/types/report_format.hpp"

class Exporter;
class ReportGenerator;

// [修改] 继承 IReportHandler
class ReportHandler : public IReportHandler {
public:
    ReportHandler(
        std::unique_ptr<ReportGenerator> report_generator,
        std::unique_ptr<Exporter> exporter
    );
    ~ReportHandler() override;

    // 实现接口方法
    std::string run_daily_query(const std::string& date, ReportFormat format) override;
    std::string run_monthly_query(const std::string& month, ReportFormat format) override;
    std::string run_period_query(int days, ReportFormat format) override;

    void run_export_single_day_report(const std::string& date, ReportFormat format) override;
    void run_export_single_month_report(const std::string& month, ReportFormat format) override;
    void run_export_single_period_report(int days, ReportFormat format) override;
    void run_export_all_daily_reports_query(ReportFormat format) override;
    void run_export_all_monthly_reports_query(ReportFormat format) override;
    void run_export_all_period_reports_query(const std::vector<int>& days_list, ReportFormat format) override;

private:
    std::unique_ptr<ReportGenerator> report_generator_;
    std::unique_ptr<Exporter> report_exporter_;
};

#endif // CORE_REPORTING_REPORT_HANDLER_HPP_