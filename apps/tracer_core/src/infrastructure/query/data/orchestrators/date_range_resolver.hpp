// infrastructure/query/data/orchestrators/date_range_resolver.hpp
#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace tracer_core::infrastructure::query::data::orchestrators {

struct ResolvedDateRange {
  std::string start_date;
  std::string end_date;
};

auto ValidateDateRange(std::string_view start_date, std::string_view end_date,
                       std::string_view invalid_range_error,
                       std::string_view invalid_date_error) -> void;

auto ResolveExplicitDateRange(const std::optional<std::string>& from_date_input,
                              const std::optional<std::string>& to_date_input,
                              std::string_view missing_boundary_error,
                              std::string_view invalid_range_error,
                              std::string_view invalid_date_error)
    -> std::optional<ResolvedDateRange>;

auto ResolveRollingDateRange(int lookback_days) -> ResolvedDateRange;

}  // namespace tracer_core::infrastructure::query::data::orchestrators
