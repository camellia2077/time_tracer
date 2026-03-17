// infrastructure/query/data/stats/stats_boundary.hpp
#pragma once

namespace tracer::core::infrastructure::query::data::stats {

[[nodiscard]] auto BoundaryReady() -> bool;

}  // namespace tracer::core::infrastructure::query::data::stats

namespace tracer_core::infrastructure::query::data::stats {

using tracer::core::infrastructure::query::data::stats::BoundaryReady;

}  // namespace tracer_core::infrastructure::query::data::stats
