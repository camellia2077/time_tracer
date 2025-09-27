#ifndef REPORT_HANDLER_HPP
#define REPORT_HANDLER_HPP

#include <string>
#include <vector>
#include <memory>
#include "queries/shared/types/ReportFormat.hpp"

class Exporter;
class QueryManager;
class ReportGenerator;

class ReportHandler {
public:
    ReportHandler(
        std::unique_ptr<QueryManager> query_manager,
        std::unique_ptr<ReportGenerator> report_generator,
        std::unique_ptr<Exporter> exporter
    );
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
    std::unique_ptr<QueryManager> direct_query_manager_;
    std::unique_ptr<ReportGenerator> report_generator_;
    std::unique_ptr<Exporter> report_exporter_;
};

#endif // REPORT_HANDLER_HPP