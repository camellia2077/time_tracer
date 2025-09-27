#include "ReportGenerator.hpp"
#include "queries/QueryHandler.hpp"

ReportGenerator::ReportGenerator(sqlite3* db, const AppConfig& config) {
    query_handler_ = std::make_unique<QueryHandler>(db, config);
}

ReportGenerator::~ReportGenerator() = default;

std::string ReportGenerator::generate_daily_report(const std::string& date, ReportFormat format) {
    return query_handler_->run_daily_query(date, format);
}

std::string ReportGenerator::generate_monthly_report(const std::string& month, ReportFormat format) {
    return query_handler_->run_monthly_query(month, format);
}

std::string ReportGenerator::generate_period_report(int days, ReportFormat format) {
    return query_handler_->run_period_query(days, format);
}

FormattedGroupedReports ReportGenerator::generate_all_daily_reports(ReportFormat format) {
    return query_handler_->run_export_all_daily_reports_query(format);
}

FormattedMonthlyReports ReportGenerator::generate_all_monthly_reports(ReportFormat format) {
    return query_handler_->run_export_all_monthly_reports_query(format);
}

FormattedPeriodReports ReportGenerator::generate_all_period_reports(const std::vector<int>& days_list, ReportFormat format) {
    return query_handler_->run_export_all_period_reports_query(days_list, format);
}