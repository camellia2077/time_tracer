// infrastructure/query/data/orchestrators/date_range_resolver.cpp
import tracer.core.infrastructure.query.data.internal.request;

#include "infrastructure/query/data/orchestrators/date_range_resolver.hpp"

#include <chrono>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace query_internal =
    tracer::core::infrastructure::query::data::internal;

namespace tracer::core::infrastructure::query::data::orchestrators {
namespace {

constexpr int kLookbackInclusiveOffsetDays = 1;

auto HasNonEmptyDateInput(const std::optional<std::string>& date_input)
    -> bool {
  return date_input.has_value() &&
         !query_internal::TrimCopy(*date_input).empty();
}

}  // namespace

auto ValidateDateRange(const DateRangeBoundaries& boundaries,
                       const DateRangeValidationErrors& errors) -> void {
  const auto kStartYmd = query_internal::ParseIsoDate(boundaries.start_date);
  const auto kEndYmd = query_internal::ParseIsoDate(boundaries.end_date);
  if (!kStartYmd.has_value() || !kEndYmd.has_value()) {
    throw std::runtime_error(std::string(errors.invalid_date_error));
  }
  const auto kStartDays = std::chrono::sys_days{*kStartYmd};
  const auto kEndDays = std::chrono::sys_days{*kEndYmd};
  if (kStartDays > kEndDays) {
    throw std::runtime_error(std::string(errors.invalid_range_error));
  }
}

auto ResolveExplicitDateRange(const std::optional<std::string>& from_date_input,
                              const std::optional<std::string>& to_date_input,
                              const ExplicitDateRangeErrors& errors)
    -> std::optional<ResolvedDateRange> {
  const bool kHasFromDate = HasNonEmptyDateInput(from_date_input);
  const bool kHasToDate = HasNonEmptyDateInput(to_date_input);
  if (kHasFromDate != kHasToDate) {
    throw std::runtime_error(std::string(errors.missing_boundary_error));
  }
  if (!kHasFromDate) {
    return std::nullopt;
  }

  const std::string kFromDate = from_date_input.value_or(std::string{});
  const std::string kToDate = to_date_input.value_or(std::string{});
  ResolvedDateRange range{
      .start_date = query_internal::NormalizeBoundaryDate(kFromDate, false),
      .end_date = query_internal::NormalizeBoundaryDate(kToDate, true),
  };
  ValidateDateRange(
      DateRangeBoundaries{
          .start_date = range.start_date,
          .end_date = range.end_date,
      },
      errors.validation);
  return range;
}

auto ResolveRollingDateRange(int lookback_days) -> ResolvedDateRange {
  const auto kEndYmd = query_internal::ResolveCurrentSystemLocalDate();
  if (!kEndYmd.ok()) {
    throw std::runtime_error("Invalid system local date.");
  }

  const auto kEndDays = std::chrono::sys_days{kEndYmd};
  const auto kStartDays =
      kEndDays -
      std::chrono::days{lookback_days - kLookbackInclusiveOffsetDays};
  return ResolvedDateRange{
      .start_date = query_internal::FormatIsoDate(
          std::chrono::year_month_day{kStartDays}),
      .end_date = query_internal::FormatIsoDate(kEndYmd),
  };
}

}  // namespace tracer::core::infrastructure::query::data::orchestrators
