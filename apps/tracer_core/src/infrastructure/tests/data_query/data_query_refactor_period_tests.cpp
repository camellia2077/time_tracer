// infrastructure/tests/data_query/data_query_refactor_period_tests.cpp
#include <exception>
#include <optional>
#include <string>

#include "infrastructure/query/data/orchestrators/date_range_resolver.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"
#include "infrastructure/tests/data_query/data_query_refactor_test_internal.hpp"

namespace android_runtime_tests::data_query_refactor_internal {
namespace {

auto TestDateRangeResolver(int& failures) -> void {
  using tracer_core::infrastructure::query::data::orchestrators::
      ResolveExplicitDateRange;
  using tracer_core::infrastructure::query::data::orchestrators::
      ResolveRollingDateRange;

  const auto kExplicitRange = ResolveExplicitDateRange(
      "20260201", "20260203",
      "report-chart requires both --from-date and "
      "--to-date.",
      "report-chart invalid range: from_date must be <= "
      "to_date.",
      "report-chart resolved invalid date range.");
  Expect(kExplicitRange.has_value(),
         "explicit date range should be produced when both boundaries exist.",
         failures);
  if (kExplicitRange.has_value()) {
    Expect(kExplicitRange->start_date == "2026-02-01",
           "explicit range should normalize start date.", failures);
    Expect(kExplicitRange->end_date == "2026-02-03",
           "explicit range should normalize end date.", failures);
  }

  bool threw_missing_boundary = false;
  try {
    static_cast<void>(ResolveExplicitDateRange(
        "20260201", std::nullopt,
        "report-chart requires both --from-date and --to-date.",
        "report-chart invalid range: from_date must be <= to_date.",
        "report-chart resolved invalid date range."));
  } catch (const std::exception& ex) {
    threw_missing_boundary =
        Contains(ex.what(), "requires both --from-date and --to-date");
  }
  Expect(threw_missing_boundary,
         "explicit resolver should reject missing boundary parameter.",
         failures);

  constexpr size_t kIsoDateLength = 10;
  const auto kRollingRange = ResolveRollingDateRange(3);
  Expect(kRollingRange.start_date.size() == kIsoDateLength &&
             kRollingRange.end_date.size() == kIsoDateLength,
         "rolling range should return ISO yyyy-mm-dd boundaries.", failures);
}

}  // namespace

auto RunDataQueryRefactorPeriodTests(int& failures) -> void {
  TestDateRangeResolver(failures);
}

}  // namespace android_runtime_tests::data_query_refactor_internal

namespace android_runtime_tests {

auto RunDataQueryRefactorTests(int& failures) -> void {
  data_query_refactor_internal::RunDataQueryRefactorPeriodTests(failures);
  data_query_refactor_internal::RunDataQueryRefactorTreeTests(failures);
  data_query_refactor_internal::RunDataQueryRefactorStatsTests(failures);
}

}  // namespace android_runtime_tests
