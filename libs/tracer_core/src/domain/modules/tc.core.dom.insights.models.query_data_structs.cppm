module;

#include "domain/insights/models/query_data_structs.hpp"

export module tracer.core.domain.insights.models.query_data_structs;

export namespace tracer::core::domain::modinsights {

using ::FormattedDailyInsightsEntry;
using ::FormattedDailyInsightsByMonth;
using ::FormattedGroupedInsights;
using ::FormattedMonthlyInsights;
using ::FormattedPeriodInsights;
using ::FormattedInsightsByIsoWeek;
using ::FormattedInsightsByMonth;
using ::FormattedWeeklyInsights;
using ::FormattedYearlyInsights;

}  // namespace tracer::core::domain::modinsights
