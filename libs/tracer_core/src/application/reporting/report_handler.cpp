// application/reporting/report_handler.cpp
#include "application/reporting/report_handler.hpp"

#include <sstream>
#include <string>
#include <utility>

#include "application/compat/reporting/i_report_query_service.hpp"

namespace {
constexpr int kSeparatorLength = 40;
}

ReportHandler::ReportHandler(std::unique_ptr<IReportQueryService> query_service)
    : query_service_(std::move(query_service)) {}

ReportHandler::~ReportHandler() = default;

auto ReportHandler::RunDailyQuery(std::string_view date, ReportFormat format)
    -> std::string {
  return query_service_->RunDailyQuery(date, format);
}

auto ReportHandler::RunMonthlyQuery(std::string_view month, ReportFormat format)
    -> std::string {
  return query_service_->RunMonthlyQuery(month, format);
}

auto ReportHandler::RunPeriodQuery(int days, ReportFormat format)
    -> std::string {
  return query_service_->RunPeriodQuery(days, format);
}

auto ReportHandler::RunWeeklyQuery(std::string_view iso_week,
                                   ReportFormat format) -> std::string {
  return query_service_->RunWeeklyQuery(iso_week, format);
}

auto ReportHandler::RunYearlyQuery(std::string_view year, ReportFormat format)
    -> std::string {
  return query_service_->RunYearlyQuery(year, format);
}

auto ReportHandler::RunPeriodQueries(const std::vector<int>& days_list,
                                     ReportFormat format) -> std::string {
  std::ostringstream output;
  for (size_t index = 0; index < days_list.size(); ++index) {
    if (index > 0) {
      output << "\n" << std::string(kSeparatorLength, '-') << "\n";
    }
    try {
      output << RunPeriodQuery(days_list[index], format);
    } catch (const std::exception& exception) {
      output << "Error querying period " << days_list[index]
             << " days: " << exception.what();
    }
  }
  return output.str();
}
