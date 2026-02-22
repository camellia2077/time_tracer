#include "infrastructure/query/data/orchestrators/date_range_resolver.hpp"

#include <chrono>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "infrastructure/persistence/sqlite_data_query_service_internal.hpp"

namespace query_internal =
    infrastructure::persistence::data_query_service_internal;

namespace time_tracer::infrastructure::query::data::orchestrators {
namespace {

constexpr int kLookbackInclusiveOffsetDays = 1;

auto HasNonEmptyDateInput(const std::optional<std::string>& date_input)
    -> bool {
  return date_input.has_value() &&
         !query_internal::TrimCopy(*date_input).empty();
}

}  // namespace

auto ValidateDateRange(std::string_view start_date, std::string_view end_date,
                       std::string_view invalid_range_error,
                       std::string_view invalid_date_error) -> void {
  const auto kStartYmd = query_internal::ParseIsoDate(start_date);
  const auto kEndYmd = query_internal::ParseIsoDate(end_date);
  if (!kStartYmd.has_value() || !kEndYmd.has_value()) {
    throw std::runtime_error(std::string(invalid_date_error));
  }
  const auto kStartDays = std::chrono::sys_days{*kStartYmd};
  const auto kEndDays = std::chrono::sys_days{*kEndYmd};
  if (kStartDays > kEndDays) {
    throw std::runtime_error(std::string(invalid_range_error));
  }
}

auto ResolveExplicitDateRange(const std::optional<std::string>& from_date_input,
                              const std::optional<std::string>& to_date_input,
                              std::string_view missing_boundary_error,
                              std::string_view invalid_range_error,
                              std::string_view invalid_date_error)
    -> std::optional<ResolvedDateRange> {
  const bool kHasFromDate = HasNonEmptyDateInput(from_date_input);
  const bool kHasToDate = HasNonEmptyDateInput(to_date_input);
  if (kHasFromDate != kHasToDate) {
    throw std::runtime_error(std::string(missing_boundary_error));
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
  ValidateDateRange(range.start_date, range.end_date, invalid_range_error,
                    invalid_date_error);
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

}  // namespace time_tracer::infrastructure::query::data::orchestrators
