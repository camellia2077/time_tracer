// infrastructure/query/data/stats/report_chart_stats_calculator.hpp
#pragma once

#include <string_view>
#include <vector>

#include "infrastructure/query/data/data_query_types.hpp"
#include "infrastructure/query/data/stats/stats_models.hpp"

namespace tracer_core::infrastructure::query::data::stats {

[[nodiscard]] auto BuildReportChartSeries(
    std::string_view start_date, std::string_view end_date,
    const std::vector<DayDurationRow>& sparse_rows) -> ReportChartSeriesResult;

}  // namespace tracer_core::infrastructure::query::data::stats
