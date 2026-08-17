// application/tests/modules/insights_tests.cpp
#include "application/tests/modules/insights_tests.hpp"
#include "application/tests/support/fakes.hpp"
#include "application/tests/support/test_support.hpp"

namespace tracer_core::application::tests {

using tracer_core::core::dto::InsightsDisplayMode;
using tracer_core::core::dto::PeriodBatchQueryRequest;
using tracer_core::core::dto::TemporalInsightsQueryRequest;
using tracer_core::core::dto::TemporalInsightsTargetsRequest;
using tracer_core::core::dto::TemporalSelectionKind;
using tracer_core::core::dto::TemporalSelectionPayload;

namespace {

auto BuildDaySelection(std::string date) -> TemporalSelectionPayload {
  return {.kind = TemporalSelectionKind::kSingleDay, .date = std::move(date)};
}

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

auto TestTemporalInsightsQueryResponses(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeInsightsHandler insights_handler;
  auto insights_data_query = std::make_shared<FakeInsightsDataQueryService>();
  auto runtime_api = BuildRuntimeApiForTest(pipeline_workflow, insights_handler,
                                            insights_data_query);

  const auto kDaySuccess = runtime_api.insights().RunTemporalInsightsQuery(
      {.display_mode = InsightsDisplayMode::kDay,
       .selection = BuildDaySelection("2026-01-03"),
       .format = InsightsFormat::kMarkdown});
  Expect(state, kDaySuccess.ok,
         "RunTemporalInsightsQuery should return ok on success.");
  Expect(state, kDaySuccess.content == "daily:2026-01-03",
         "RunTemporalInsightsQuery should return formatted day content.");

  const auto kRecentSuccess = runtime_api.insights().RunTemporalInsightsQuery(
      {.display_mode = InsightsDisplayMode::kRecent,
       .selection = BuildRecentSelection(7, "2026-03-07"),
       .format = InsightsFormat::kMarkdown});
  Expect(state, kRecentSuccess.ok,
         "RunTemporalInsightsQuery recent should succeed with anchor_date.");
  Expect(
      state, kRecentSuccess.content == "period:2026-03-01|2026-03-07",
      "RunTemporalInsightsQuery recent should format anchored fixed window.");
  Expect(
      state,
      kRecentSuccess.insights_window_metadata.has_value() &&
          kRecentSuccess.insights_window_metadata->requested_days == 7,
      "RunTemporalInsightsQuery recent should expose requested_days metadata.");

  const auto kBadRecentArg = runtime_api.insights().RunTemporalInsightsQuery(
      {.display_mode = InsightsDisplayMode::kRecent,
       .selection = BuildRecentSelection(0),
       .format = InsightsFormat::kMarkdown});
  Expect(state, !kBadRecentArg.ok,
         "RunTemporalInsightsQuery recent should fail DTO on invalid days.");
  Expect(
      state,
      Contains(kBadRecentArg.error_message, "RunTemporalInsightsQuery failed"),
      "RunTemporalInsightsQuery invalid argument should include operation "
      "name.");

  insights_data_query->fail_target_not_found = true;
  const auto kMissingDay = runtime_api.insights().RunTemporalInsightsQuery(
      {.display_mode = InsightsDisplayMode::kDay,
       .selection = BuildDaySelection("2024-12-31"),
       .format = InsightsFormat::kMarkdown});
  Expect(state, !kMissingDay.ok,
         "RunTemporalInsightsQuery should fail when named insights target is "
         "missing.");
  Expect(state,
         kMissingDay.error_contract.error_code == "insights.target.not_found",
         "RunTemporalInsightsQuery missing-target failure should expose stable "
         "error code.");
  Expect(state, kMissingDay.error_contract.error_category == "insights",
         "RunTemporalInsightsQuery missing-target failure should expose "
         "insights category.");
  insights_data_query->fail_target_not_found = false;

  insights_handler.period_batch_result = "period-batch-insights";
  const auto kBatchSuccess = runtime_api.insights().RunPeriodBatchQuery(
      {.days_list = {7, 14}, .format = InsightsFormat::kMarkdown});
  Expect(state, kBatchSuccess.ok,
         "RunPeriodBatchQuery should return ok on success.");
  Expect(state, Contains(kBatchSuccess.content, "period:|"),
         "RunPeriodBatchQuery should use structured formatter when available.");
}

auto TestTemporalInsightsTargetsResponses(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeInsightsHandler insights_handler;
  auto insights_data_query = std::make_shared<FakeInsightsDataQueryService>();
  auto runtime_api = BuildRuntimeApiForTest(pipeline_workflow, insights_handler,
                                            insights_data_query);

  const auto kSuccess = runtime_api.insights().RunTemporalInsightsTargetsQuery(
      {.display_mode = InsightsDisplayMode::kMonth});
  Expect(state, kSuccess.ok,
         "RunTemporalInsightsTargetsQuery should return ok on success.");
  Expect(state, kSuccess.items == insights_data_query->monthly_targets,
         "RunTemporalInsightsTargetsQuery should return monthly canonical "
         "targets.");

  insights_data_query->fail_list_targets = true;
  const auto kFailure = runtime_api.insights().RunTemporalInsightsTargetsQuery(
      {.display_mode = InsightsDisplayMode::kDay});
  Expect(
      state, !kFailure.ok,
      "RunTemporalInsightsTargetsQuery should return failed DTO when listing "
      "throws.");
  Expect(
      state,
      Contains(kFailure.error_message, "RunTemporalInsightsTargetsQuery"),
      "RunTemporalInsightsTargetsQuery failure should include operation name.");

  auto runtime_without_targets =
      BuildRuntimeApiForTest(pipeline_workflow, insights_handler);
  const auto kMissingService =
      runtime_without_targets.insights().RunTemporalInsightsTargetsQuery(
          {.display_mode = InsightsDisplayMode::kYear});
  Expect(state, !kMissingService.ok,
         "RunTemporalInsightsTargetsQuery should fail when insights data query "
         "service is missing.");
  Expect(
      state,
      Contains(kMissingService.error_message,
               "RunTemporalInsightsTargetsQuery"),
      "RunTemporalInsightsTargetsQuery missing-service failure should include "
      "operation name.");
}

auto TestStructuredWindowInsightsSemantics(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeInsightsHandler insights_handler;
  auto insights_data_query = std::make_shared<FakeInsightsDataQueryService>();
  auto runtime_api = BuildRuntimeApiForTest(pipeline_workflow, insights_handler,
                                            insights_data_query);

  const auto kEmptyRecent =
      runtime_api.insights().RunTemporalStructuredInsightsQuery(
          {.display_mode = InsightsDisplayMode::kRecent,
           .selection = BuildRecentSelection(7)});
  Expect(state, kEmptyRecent.ok,
         "RunTemporalStructuredInsightsQuery recent should succeed for empty "
         "window.");
  Expect(state, kEmptyRecent.error_contract.error_code.empty(),
         "RunTemporalStructuredInsightsQuery recent empty window should not "
         "expose error code.");
  const auto* kRecentInsights =
      std::get_if<PeriodInsightsData>(&kEmptyRecent.insights);
  Expect(
      state, kRecentInsights != nullptr,
      "RunTemporalStructuredInsightsQuery recent should return period insights "
      "data.");
  if (kRecentInsights != nullptr) {
    Expect(state, !kRecentInsights->has_records,
           "RunTemporalStructuredInsightsQuery recent empty window should set "
           "has_records=false.");
    Expect(state, kRecentInsights->matched_day_count == 0,
           "RunTemporalStructuredInsightsQuery recent empty window should set "
           "matched_day_count=0.");
    Expect(state, kRecentInsights->activity.occurrence_count == 0,
           "RunTemporalStructuredInsightsQuery recent empty window should set "
           "matched_record_count=0.");
    Expect(state, kRecentInsights->requested_days == 7,
           "RunTemporalStructuredInsightsQuery recent should preserve "
           "requested_days.");
  }

  const auto kAnchoredRecent =
      runtime_api.insights().RunTemporalStructuredInsightsQuery(
          {.display_mode = InsightsDisplayMode::kRecent,
           .selection = BuildRecentSelection(7, "2026-03-07")});
  Expect(state, kAnchoredRecent.ok,
         "RunTemporalStructuredInsightsQuery anchored recent should succeed.");
  const auto* kAnchoredRecentInsights =
      std::get_if<PeriodInsightsData>(&kAnchoredRecent.insights);
  Expect(state, kAnchoredRecentInsights != nullptr,
         "RunTemporalStructuredInsightsQuery anchored recent should return "
         "period data.");
  if (kAnchoredRecentInsights != nullptr) {
    Expect(state, kAnchoredRecentInsights->start_date == "2026-03-01",
           "Anchored recent should resolve fixed inclusive window start_date.");
    Expect(state, kAnchoredRecentInsights->end_date == "2026-03-07",
           "Anchored recent should resolve fixed inclusive window end_date.");
    Expect(state, kAnchoredRecentInsights->requested_days == 7,
           "Anchored recent should preserve requested_days.");
  }

  const auto kEmptyRange =
      runtime_api.insights().RunTemporalStructuredInsightsQuery(
          {.display_mode = InsightsDisplayMode::kRange,
           .selection = BuildRangeSelection("2024-12-01", "2024-12-31")});
  Expect(state, kEmptyRange.ok,
         "RunTemporalStructuredInsightsQuery range should succeed for empty "
         "window.");
  Expect(state, kEmptyRange.error_contract.error_code.empty(),
         "RunTemporalStructuredInsightsQuery range empty window should not "
         "expose error code.");
  const auto* kRangeInsights =
      std::get_if<PeriodInsightsData>(&kEmptyRange.insights);
  Expect(
      state, kRangeInsights != nullptr,
      "RunTemporalStructuredInsightsQuery range should return period insights "
      "data.");
  if (kRangeInsights != nullptr) {
    Expect(state, !kRangeInsights->has_records,
           "RunTemporalStructuredInsightsQuery range empty window should set "
           "has_records=false.");
    Expect(state, kRangeInsights->matched_day_count == 0,
           "RunTemporalStructuredInsightsQuery range empty window should set "
           "matched_day_count=0.");
    Expect(state, kRangeInsights->activity.occurrence_count == 0,
           "RunTemporalStructuredInsightsQuery range empty window should set "
           "matched_record_count=0.");
  }

  const auto kInvalidRecent =
      runtime_api.insights().RunTemporalStructuredInsightsQuery(
          {.display_mode = InsightsDisplayMode::kRecent,
           .selection = BuildRecentSelection(0)});
  Expect(
      state, !kInvalidRecent.ok,
      "RunTemporalStructuredInsightsQuery recent should fail on non-positive "
      "days.");
  Expect(state,
         Contains(kInvalidRecent.error_message,
                  "RunTemporalStructuredInsightsQuery failed"),
         "RunTemporalStructuredInsightsQuery recent invalid argument should "
         "include operation name.");

  const auto kInvalidRange =
      runtime_api.insights().RunTemporalStructuredInsightsQuery(
          {.display_mode = InsightsDisplayMode::kRange,
           .selection = BuildRangeSelection("2026-01-31", "2026-01-01")});
  Expect(state, !kInvalidRange.ok,
         "RunTemporalStructuredInsightsQuery range should fail on descending "
         "range.");
  Expect(state,
         Contains(kInvalidRange.error_message,
                  "RunTemporalStructuredInsightsQuery failed"),
         "RunTemporalStructuredInsightsQuery range invalid argument should "
         "include operation name.");
}

}  // namespace

auto RunInsightsTests(TestState& state) -> void {
  TestTemporalInsightsQueryResponses(state);
  TestTemporalInsightsTargetsResponses(state);
  TestStructuredWindowInsightsSemantics(state);
}

}  // namespace tracer_core::application::tests
