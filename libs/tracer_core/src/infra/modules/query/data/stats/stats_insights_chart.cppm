module;

#include "infra/query/data/stats/insights_chart_stats_calculator.hpp"

export module tracer.core.infrastructure.query.data.stats
    .insights_chart_stats_calculator;

export namespace tracer::core::infrastructure::query::data::stats {

using ::tracer::core::infrastructure::query::data::stats::
    CalculateAverageOrZero;
using ::tracer::core::infrastructure::query::data::stats::
    ResolveAverageDenominator;

using ::tracer::core::infrastructure::query::data::stats::
    BuildInsightsCompositionTreeView;
using ::tracer::core::infrastructure::query::data::stats::
    CalculateInclusiveDateRangeDays;
using ::tracer::core::infrastructure::query::data::stats::
    InsightsCompositionStats;

using ::tracer::core::infrastructure::query::data::stats::
    BuildInsightsChartSeries;
using ::tracer::core::infrastructure::query::data::stats::
    BuildInsightsCompositionStats;

}  // namespace tracer::core::infrastructure::query::data::stats
