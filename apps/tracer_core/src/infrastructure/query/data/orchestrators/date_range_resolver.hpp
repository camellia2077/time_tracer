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

struct DateRangeValidationErrors {
  std::string_view invalid_range_error;
  std::string_view invalid_date_error;
};

struct DateRangeBoundaries {
  std::string_view start_date;
  std::string_view end_date;
};

struct ExplicitDateRangeErrors {
  std::string_view missing_boundary_error;
  DateRangeValidationErrors validation;
};

auto ValidateDateRange(const DateRangeBoundaries& boundaries,
                       const DateRangeValidationErrors& errors) -> void;

auto ResolveExplicitDateRange(const std::optional<std::string>& from_date_input,
                              const std::optional<std::string>& to_date_input,
                              const ExplicitDateRangeErrors& errors)
    -> std::optional<ResolvedDateRange>;

auto ResolveRollingDateRange(int lookback_days) -> ResolvedDateRange;

}  // namespace tracer_core::infrastructure::query::data::orchestrators
