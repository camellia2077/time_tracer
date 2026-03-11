module;

#include <string_view>
#include <vector>

export module tracer.core.infrastructure.query.data.stats.report_chart_stats_calculator;

import tracer.core.infrastructure.query.data.repository.types;
import tracer.core.infrastructure.query.data.stats.models;

export namespace tracer::core::infrastructure::query::data::stats {

#include "infrastructure/query/data/stats/detail/report_chart_stats_calculator_decl.inc"

}  // namespace tracer::core::infrastructure::query::data::stats
