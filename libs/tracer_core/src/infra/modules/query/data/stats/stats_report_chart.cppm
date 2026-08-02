module;

#include "infra/query/data/stats/report_chart_stats_calculator.hpp"

export module tracer.core.infrastructure.query.data.stats
    .report_chart_stats_calculator;

export namespace tracer::core::infrastructure::query::data::stats {

using ::tracer::core::infrastructure::query::data::stats::
    CalculateInclusiveDateRangeDays;
using ::tracer::core::infrastructure::query::data::stats::
    BuildReportCompositionTreeView;
using ::tracer::core::infrastructure::query::data::stats::
    ReportCompositionStats;

using ::tracer::core::infrastructure::query::data::stats::
    BuildReportChartSeries;
using ::tracer::core::infrastructure::query::data::stats::
    BuildReportCompositionStats;

}  // namespace tracer::core::infrastructure::query::data::stats
