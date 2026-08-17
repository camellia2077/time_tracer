// application/tests/modules/insights_semantics_tests.cpp
#include "application/tests/modules/insights_tests.hpp"

#include <stdexcept>

#include "application/tests/support/fakes.hpp"
#include "application/tests/support/test_support.hpp"
#include "application/use_cases/insights_api_support.hpp"

namespace tracer_core::application::tests {

namespace insights_support =
    tracer::core::application::use_cases::insights_support;
using tracer_core::core::dto::InsightsDisplayMode;
using tracer_core::core::dto::TemporalSelectionKind;
using tracer_core::core::dto::TemporalSelectionPayload;

namespace {

auto BuildRangeSelection(std::string start_date, std::string end_date)
    -> TemporalSelectionPayload {
  return {.kind = TemporalSelectionKind::kDateRange,
          .start_date = std::move(start_date),
          .end_date = std::move(end_date)};
}

auto BuildRecentSelection(int days,
                          std::optional<std::string> anchor_date = std::nullopt)
    -> TemporalSelectionPayload {
  return {.kind = TemporalSelectionKind::kRecentDays,
          .days = days,
          .anchor_date = std::move(anchor_date)};
}

auto BuildDaySelection(std::string date) -> TemporalSelectionPayload {
  return {.kind = TemporalSelectionKind::kSingleDay, .date = std::move(date)};
}

auto TestParseRecentDaysArgument(TestState& state) -> void {
  Expect(state, insights_support::ParseRecentDaysArgument("7") == 7,
         "ParseRecentDaysArgument should parse positive integer.");
  Expect(state, insights_support::ParseRecentDaysArgument(" 14 ") == 14,
         "ParseRecentDaysArgument should ignore ASCII whitespace.");

  bool threw_zero = false;
  try {
    static_cast<void>(insights_support::ParseRecentDaysArgument("0"));
  } catch (const std::invalid_argument&) {
    threw_zero = true;
  }
  Expect(state, threw_zero,
         "ParseRecentDaysArgument should reject non-positive values.");

  bool threw_alpha = false;
  try {
    static_cast<void>(insights_support::ParseRecentDaysArgument("abc"));
  } catch (const std::invalid_argument&) {
    threw_alpha = true;
  }
  Expect(state, threw_alpha,
         "ParseRecentDaysArgument should reject non-numeric values.");
}

auto TestParseRangeArgument(TestState& state) -> void {
  const auto canonical =
      insights_support::ParseRangeArgument("2026-01-01|2026-01-31");
  Expect(state, canonical.start_date == "2026-01-01",
         "ParseRangeArgument should preserve ISO start_date.");
  Expect(state, canonical.end_date == "2026-01-31",
         "ParseRangeArgument should preserve ISO end_date.");

  const auto comma =
      insights_support::ParseRangeArgument(" 2026-02-01 , 2026-02-09 ");
  Expect(state, comma.start_date == "2026-02-01",
         "ParseRangeArgument should accept comma separators.");
  Expect(state, comma.end_date == "2026-02-09",
         "ParseRangeArgument should trim whitespace around comma-separated "
         "dates.");

  bool threw_descending = false;
  try {
    static_cast<void>(
        insights_support::ParseRangeArgument("2026-03-09|2026-03-01"));
  } catch (const std::invalid_argument&) {
    threw_descending = true;
  }
  Expect(state, threw_descending,
         "ParseRangeArgument should reject descending ranges.");

  bool threw_missing = false;
  try {
    static_cast<void>(insights_support::ParseRangeArgument("2026-03-01"));
  } catch (const std::invalid_argument&) {
    threw_missing = true;
  }
  Expect(state, threw_missing,
         "ParseRangeArgument should require explicit range separators.");
}

auto TestTemporalTextQueryPreservesWindowMetadata(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeInsightsHandler insights_handler;
  auto insights_data_query = std::make_shared<FakeInsightsDataQueryService>();
  auto runtime_api = BuildRuntimeApiForTest(pipeline_workflow, insights_handler,
                                            insights_data_query);

  const auto recent = runtime_api.insights().RunTemporalInsightsQuery(
      {.display_mode = InsightsDisplayMode::kRecent,
       .selection = BuildRecentSelection(7, "2026-03-07"),
       .format = InsightsFormat::kMarkdown});
  Expect(state, recent.ok,
         "RunTemporalInsightsQuery should succeed for recent period insights.");
  Expect(state, recent.content == "period:2026-03-01|2026-03-07",
         "RunTemporalInsightsQuery should delegate period formatting.");
  Expect(state, recent.insights_window_metadata.has_value(),
         "RunTemporalInsightsQuery should expose window metadata for recent "
         "insights.");
  if (recent.insights_window_metadata.has_value()) {
    const auto& metadata = *recent.insights_window_metadata;
    Expect(state, !metadata.has_records,
           "Recent insights metadata should preserve has_records=false.");
    Expect(state, metadata.matched_day_count == 0,
           "Recent insights metadata should preserve matched_day_count.");
    Expect(state, metadata.matched_record_count == 0,
           "Recent insights metadata should preserve matched_record_count.");
    Expect(state, metadata.start_date == "2026-03-01",
           "Recent insights metadata should preserve anchored start_date.");
    Expect(state, metadata.end_date == "2026-03-07",
           "Recent insights metadata should preserve anchored end_date.");
    Expect(state, metadata.requested_days == 7,
           "Recent insights metadata should preserve requested_days.");
  }

  const auto range = runtime_api.insights().RunTemporalInsightsQuery(
      {.display_mode = InsightsDisplayMode::kRange,
       .selection = BuildRangeSelection("2024-12-01", "2024-12-31"),
       .format = InsightsFormat::kMarkdown});
  Expect(state, range.ok,
         "RunTemporalInsightsQuery should succeed for range period insights.");
  Expect(state, range.insights_window_metadata.has_value(),
         "RunTemporalInsightsQuery should expose window metadata for range "
         "insights.");

  const auto month = runtime_api.insights().RunTemporalInsightsQuery(
      {.display_mode = InsightsDisplayMode::kMonth,
       .selection = BuildRangeSelection("2026-04-01", "2026-04-30"),
       .format = InsightsFormat::kMarkdown});
  Expect(state, month.ok,
         "RunTemporalInsightsQuery should succeed for monthly insights.");
  Expect(state, month.content == "month:2026-04",
         "RunTemporalInsightsQuery should delegate monthly formatting.");
  Expect(state, !month.insights_window_metadata.has_value(),
         "RunTemporalInsightsQuery should keep window metadata reserved for "
         "recent/range insights.");
}

auto TestStructuredInsightsDistinguishesEmptyWindowFromMissingTarget(
    TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeInsightsHandler insights_handler;
  auto insights_data_query = std::make_shared<FakeInsightsDataQueryService>();
  auto runtime_api = BuildRuntimeApiForTest(pipeline_workflow, insights_handler,
                                            insights_data_query);

  const auto empty_range =
      runtime_api.insights().RunTemporalStructuredInsightsQuery(
          {.display_mode = InsightsDisplayMode::kRange,
           .selection = BuildRangeSelection("2024-12-01", "2024-12-31")});
  Expect(
      state, empty_range.ok,
      "RunTemporalStructuredInsightsQuery should treat empty range windows as "
      "successful insights.");
  Expect(state, empty_range.error_contract.error_code.empty(),
         "RunTemporalStructuredInsightsQuery empty range should not expose "
         "target-not-found error code.");
  const auto* empty_range_insights =
      std::get_if<PeriodInsightsData>(&empty_range.insights);
  Expect(state, empty_range_insights != nullptr,
         "RunTemporalStructuredInsightsQuery empty range should still return "
         "period insights data.");
  if (empty_range_insights != nullptr) {
    Expect(state, !empty_range_insights->has_records,
           "RunTemporalStructuredInsightsQuery empty range should preserve "
           "has_records=false.");
    Expect(state, empty_range_insights->activity.occurrence_count == 0,
           "RunTemporalStructuredInsightsQuery empty range should preserve "
           "matched_record_count=0.");
  }

  insights_data_query->fail_target_not_found = true;
  const auto missing_day =
      runtime_api.insights().RunTemporalStructuredInsightsQuery(
          {.display_mode = InsightsDisplayMode::kDay,
           .selection = BuildDaySelection("2024-12-31")});
  Expect(
      state, !missing_day.ok,
      "RunTemporalStructuredInsightsQuery should fail when the named insights "
      "target is missing.");
  Expect(state,
         missing_day.error_contract.error_code == "insights.target.not_found",
         "RunTemporalStructuredInsightsQuery missing target should expose "
         "insights.target.not_found.");
  Expect(state, missing_day.error_contract.error_category == "insights",
         "RunTemporalStructuredInsightsQuery missing target should expose "
         "insights category.");
}

auto TestStructuredRangePreservesConfiguredStatuses(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeInsightsHandler insights_handler;
  auto insights_data_query = std::make_shared<FakeInsightsDataQueryService>();
  insights_data_query->period_statuses = {
      {.id = "study",
       .label = "Study",
       .occurrence_count = 3,
       .total_duration = 5400},
  };
  auto runtime_api = BuildRuntimeApiForTest(pipeline_workflow, insights_handler,
                                            insights_data_query);

  const auto result = runtime_api.insights().RunTemporalStructuredInsightsQuery(
      {.display_mode = InsightsDisplayMode::kRange,
       .selection = BuildRangeSelection("2026-01-01", "2026-01-07")});
  const auto* insights = std::get_if<PeriodInsightsData>(&result.insights);
  Expect(state, result.ok && insights != nullptr,
         "structured range insights should return period data.");
  if (insights != nullptr) {
    Expect(state,
           insights->statuses.size() == 1U &&
               insights->statuses[0].id == "study" &&
               insights->statuses[0].occurrence_count == 3 &&
               insights->statuses[0].total_duration == 5400,
           "structured range insights should preserve configured statuses.");
  }
}

}  // namespace

auto RunInsightsSemanticsTests(TestState& state) -> void {
  TestParseRecentDaysArgument(state);
  TestParseRangeArgument(state);
  TestTemporalTextQueryPreservesWindowMetadata(state);
  TestStructuredInsightsDistinguishesEmptyWindowFromMissingTarget(state);
  TestStructuredRangePreservesConfiguredStatuses(state);
}

}  // namespace tracer_core::application::tests
