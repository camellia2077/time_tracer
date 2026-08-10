#include "infra/sqlite_fwd.hpp"

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include "infra/insights/insights_service.hpp"
#include "application/compat/insights/i_insights_query_service.hpp"
#include "application/ports/insights/i_platform_clock.hpp"
#include "infra/config/models/insights_catalog.hpp"
#include "infra/insights/data/queriers/daily/daily_querier.hpp"
#include "infra/insights/data/queriers/monthly/monthly_querier.hpp"
#include "infra/insights/data/queriers/period/period_querier.hpp"
#include "infra/insights/data/queriers/weekly/weekly_querier.hpp"
#include "infra/insights/data/queriers/yearly/yearly_querier.hpp"
#include "infra/insights/shared/generators/base_generator.hpp"
#include "infra/insights/shared/factories/generic_formatter_factory.hpp"
#include "infra/insights/shared/interfaces/i_insights_formatter.hpp"

import tracer.core.domain.insights.models.daily_insights_data;

namespace modinsights = tracer::core::domain::modinsights;
namespace tracer::core::infrastructure::insights {

InsightsService::InsightsService(
    sqlite3* sqlite_db, const InsightsCatalog& catalog,
    std::shared_ptr<tracer_core::application::ports::IPlatformClock>
        platform_clock)
    : db_(sqlite_db),
      insights_catalog_(catalog),
      platform_clock_(std::move(platform_clock)) {
  if (!platform_clock_) {
    throw std::invalid_argument(
        "InsightsService platform clock must not be null.");
  }
}

auto InsightsService::RunDailyQuery(std::string_view date,
                                  InsightsFormat format) const -> std::string {
  DayQuerier querier(db_, date, &insights_catalog_.statuses.day);
  const modinsights::DailyInsightsData insights_data = querier.FetchData();
  auto formatter = GenericFormatterFactory<modinsights::DailyInsightsData>::Create(
      format, insights_catalog_);
  return formatter->FormatInsights(insights_data);
}

auto InsightsService::RunMonthlyQuery(std::string_view year_month_str,
                                    InsightsFormat format) const -> std::string {
  BaseGenerator<MonthlyInsightsData, MonthQuerier, std::string_view> generator(
      db_, insights_catalog_, insights_catalog_.statuses.month);
  return generator.GenerateInsights(year_month_str, format);
}

auto InsightsService::RunPeriodQuery(int days, InsightsFormat format) const
    -> std::string {
  PeriodQuerier querier(db_, days, *platform_clock_,
                        &insights_catalog_.statuses.recent);
  PeriodInsightsData insights_data = querier.FetchData();
  auto& formatter = GetOrCreatePeriodFormatter(format);
  return formatter.FormatInsights(insights_data);
}

auto InsightsService::RunWeeklyQuery(std::string_view iso_week_str,
                                   InsightsFormat format) const -> std::string {
  BaseGenerator<WeeklyInsightsData, WeekQuerier, std::string_view> generator(
      db_, insights_catalog_, insights_catalog_.statuses.week);
  return generator.GenerateInsights(iso_week_str, format);
}

auto InsightsService::RunYearlyQuery(std::string_view year_str,
                                   InsightsFormat format) const -> std::string {
  BaseGenerator<YearlyInsightsData, YearQuerier, std::string_view> generator(
      db_, insights_catalog_, insights_catalog_.statuses.year);
  return generator.GenerateInsights(year_str, format);
}

auto InsightsService::GetOrCreatePeriodFormatter(InsightsFormat format) const
    -> IInsightsFormatter<PeriodInsightsData>& {
  if (auto formatter_iter = period_formatter_cache_.find(format);
      formatter_iter != period_formatter_cache_.end()) {
    return *(formatter_iter->second);
  }

  auto formatter = GenericFormatterFactory<PeriodInsightsData>::Create(
      format, insights_catalog_);
  auto [inserted_iter, inserted] =
      period_formatter_cache_.emplace(format, std::move(formatter));
  if (!inserted || !(inserted_iter->second)) {
    throw std::runtime_error(
        "Failed to cache period formatter for selected insights format.");
  }

  return *(inserted_iter->second);
}

}  // namespace tracer::core::infrastructure::insights
