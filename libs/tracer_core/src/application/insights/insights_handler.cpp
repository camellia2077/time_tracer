// application/insights/insights_handler.cpp
#include "application/insights/insights_handler.hpp"

#include <sstream>
#include <string>
#include <utility>

#include "application/compat/insights/i_insights_query_service.hpp"

namespace {
constexpr int kSeparatorLength = 40;
}

InsightsHandler::InsightsHandler(std::unique_ptr<IInsightsQueryService> query_service)
    : query_service_(std::move(query_service)) {}

InsightsHandler::~InsightsHandler() = default;

auto InsightsHandler::RunDailyQuery(std::string_view date, InsightsFormat format)
    -> std::string {
  return query_service_->RunDailyQuery(date, format);
}

auto InsightsHandler::RunMonthlyQuery(std::string_view month, InsightsFormat format)
    -> std::string {
  return query_service_->RunMonthlyQuery(month, format);
}

auto InsightsHandler::RunPeriodQuery(int days, InsightsFormat format)
    -> std::string {
  return query_service_->RunPeriodQuery(days, format);
}

auto InsightsHandler::RunWeeklyQuery(std::string_view iso_week,
                                   InsightsFormat format) -> std::string {
  return query_service_->RunWeeklyQuery(iso_week, format);
}

auto InsightsHandler::RunYearlyQuery(std::string_view year, InsightsFormat format)
    -> std::string {
  return query_service_->RunYearlyQuery(year, format);
}

auto InsightsHandler::RunPeriodQueries(const std::vector<int>& days_list,
                                     InsightsFormat format) -> std::string {
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
