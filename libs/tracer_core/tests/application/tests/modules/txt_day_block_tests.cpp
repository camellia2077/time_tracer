#include "application/tests/modules/pipeline_tests.hpp"

#include "application/pipeline/txt_day_block_support.hpp"
#include "application/tests/support/fakes.hpp"
#include "application/tests/support/test_support.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>

namespace tracer_core::application::tests {

using tracer::core::application::pipeline::txt_day_block::DefaultDayMarker;
using tracer::core::application::pipeline::txt_day_block::ReplaceDayBlock;
using tracer::core::application::pipeline::txt_day_block::ResolveDayBlock;

namespace {

auto BuildRepoRoot() -> std::filesystem::path {
  return std::filesystem::path(__FILE__)
      .parent_path()   // modules
      .parent_path()   // tests
      .parent_path()   // application
      .parent_path()   // tests
      .parent_path()   // tracer_core
      .parent_path()   // libs
      .parent_path();  // repo root
}

auto ReadFixtureText(const std::filesystem::path& relative_path) -> std::string {
  const std::filesystem::path fixture_path = BuildRepoRoot() / relative_path;
  std::ifstream input(fixture_path, std::ios::binary);
  if (!input.is_open()) {
    return {};
  }
  return std::string(std::istreambuf_iterator<char>(input),
                     std::istreambuf_iterator<char>());
}

constexpr std::string_view kMonthContent =
    "y2025\n"
    "m01\n"
    "\n"
    "0101\n"
    "0900study\n"
    "\n"
    "0102\n"
    "0656w\n"
    "0904无氧训练 #cherry\n"
    "2207minecraft\n"
    "\n"
    "0103\n"
    "0900sleep\n";

auto TestTxtDayBlockSemantics(TestState& state) -> void {
  const auto resolved = ResolveDayBlock({
      .content = std::string(kMonthContent),
      .day_marker = "0102",
      .selected_month = "2025-01",
  });
  Expect(state, resolved.ok, "ResolveDayBlock should return ok=true.");
  Expect(state, resolved.found,
         "ResolveDayBlock should find an existing day marker.");
  Expect(state, resolved.is_marker_valid,
         "ResolveDayBlock should accept valid MMDD markers.");
  Expect(state, resolved.can_save,
         "ResolveDayBlock should allow save when target day exists.");
  Expect(state,
         !Contains(resolved.day_body, "0102") &&
             Contains(resolved.day_body, "0904无氧训练"),
         "ResolveDayBlock should return body lines without marker line.");
  Expect(state,
         resolved.day_content_iso_date == std::optional<std::string>("2025-01-02"),
         "ResolveDayBlock should derive ISO date when selected month matches marker.");

  const auto missing = ResolveDayBlock({
      .content = std::string(kMonthContent),
      .day_marker = "0109",
      .selected_month = "2025-01",
  });
  Expect(state, missing.ok, "missing day should still return ok=true.");
  Expect(state, !missing.found, "missing day should set found=false.");
  Expect(state, missing.is_marker_valid,
         "missing day should preserve valid marker status.");
  Expect(state, !missing.can_save,
         "missing day should disable save semantics.");

  const auto invalid = ResolveDayBlock({
      .content = std::string(kMonthContent),
      .day_marker = "01ab99",
      .selected_month = "2025-01",
  });
  Expect(state, invalid.ok, "invalid marker should still return ok=true.");
  Expect(state, !invalid.is_marker_valid,
         "invalid marker should be rejected.");
  Expect(state, !invalid.found, "invalid marker should not report found.");

  const auto replaced = ReplaceDayBlock({
      .content = std::string(kMonthContent),
      .day_marker = "0102",
      .edited_day_body = "0102\n1111new_line\n2222tail\n",
  });
  Expect(state, replaced.ok, "ReplaceDayBlock should return ok=true.");
  Expect(state, replaced.found,
         "ReplaceDayBlock should find existing day block.");
  Expect(state,
         Contains(replaced.updated_content, "0102\n1111new_line\n2222tail\n") &&
             Contains(replaced.updated_content, "0101\n0900study\n") &&
             Contains(replaced.updated_content, "0103\n0900sleep\n"),
         "ReplaceDayBlock should only rewrite the target day block.");
  const auto replaced_resolved = ResolveDayBlock({
      .content = replaced.updated_content,
      .day_marker = "0102",
      .selected_month = "2025-01",
  });
  Expect(state, replaced_resolved.day_body == "1111new_line\n2222tail\n",
         "ReplaceDayBlock should preserve trailing blank line after resolve.");

  const auto default_marker = DefaultDayMarker({
      .selected_month = "2025-02",
      .target_date_iso = "2025-01-31",
  });
  Expect(state, default_marker.ok,
         "DefaultDayMarker should succeed for valid target_date_iso.");
  Expect(state, default_marker.normalized_day_marker == "0228",
         "DefaultDayMarker should clamp to selected month end.");

  const auto fallback_marker = DefaultDayMarker({
      .selected_month = "bad-month",
      .target_date_iso = "2025-01-31",
  });
  Expect(state, fallback_marker.normalized_day_marker == "0131",
         "DefaultDayMarker should fall back to target_date_iso month/day when selected month is invalid.");
}

auto TestTxtDayBlockFixtureFiles(TestState& state) -> void {
  const std::string empty_month = ReadFixtureText(
      "test/fixtures/text/minimal_month/2026-01.empty.txt");
  Expect(state, !empty_month.empty(),
         "empty month fixture should be readable from test/fixtures.");
  const auto empty_resolved = ResolveDayBlock({
      .content = empty_month,
      .day_marker = "0101",
      .selected_month = "2026-01",
  });
  Expect(state, empty_resolved.ok,
         "empty month fixture should still resolve with ok=true.");
  Expect(state, !empty_resolved.found,
         "empty month fixture should not find a missing day block.");
  Expect(state, !empty_resolved.can_save,
         "empty month fixture should keep save disabled for missing day.");

  const std::string single_day = ReadFixtureText(
      "test/fixtures/text/minimal_month/2026-01.single_day.txt");
  Expect(state, !single_day.empty(),
         "single-day fixture should be readable from test/fixtures.");
  const auto single_resolved = ResolveDayBlock({
      .content = single_day,
      .day_marker = "0101",
      .selected_month = "2026-01",
  });
  Expect(state, single_resolved.ok && single_resolved.found,
         "single-day fixture should resolve the authored day block.");
  Expect(state,
         Contains(single_resolved.day_body, "0700wake") &&
             Contains(single_resolved.day_body, "0830study"),
         "single-day fixture should preserve authored event lines.");

  const std::string cross_midnight = ReadFixtureText(
      "test/fixtures/text/minimal_month/2026-01.cross_midnight.txt");
  Expect(state, !cross_midnight.empty(),
         "cross-midnight fixture should be readable from test/fixtures.");
  const auto overnight_resolved = ResolveDayBlock({
      .content = cross_midnight,
      .day_marker = "0101",
      .selected_month = "2026-01",
  });
  Expect(state, overnight_resolved.ok && overnight_resolved.found,
         "cross-midnight fixture should resolve the target day block.");
  Expect(state,
         Contains(overnight_resolved.day_body, "2350睡觉") &&
             Contains(overnight_resolved.day_body, "0700起床") &&
             Contains(overnight_resolved.day_body, "0830meal"),
         "cross-midnight fixture should preserve overnight boundary lines.");
}

auto TestTxtDayBlockPipelineApiForwarding(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeReportHandler report_handler;
  auto runtime_api = BuildRuntimeApiForTest(pipeline_workflow, report_handler);

  const auto default_response = runtime_api.pipeline().RunDefaultTxtDayMarker({
      .selected_month = "2025-01",
      .target_date_iso = "2025-01-02",
  });
  Expect(state, default_response.ok,
         "RunDefaultTxtDayMarker should return workflow response.");
  Expect(state, pipeline_workflow.default_txt_day_marker_call_count == 1,
         "RunDefaultTxtDayMarker should call workflow once.");
  Expect(state,
         pipeline_workflow.last_default_txt_day_marker_request.target_date_iso ==
             "2025-01-02",
         "RunDefaultTxtDayMarker should forward target_date_iso.");

  const auto resolve_response = runtime_api.pipeline().RunResolveTxtDayBlock({
      .content = "full-content",
      .day_marker = "0102",
      .selected_month = "2025-01",
  });
  Expect(state, resolve_response.ok,
         "RunResolveTxtDayBlock should return workflow response.");
  Expect(state, pipeline_workflow.resolve_txt_day_block_call_count == 1,
         "RunResolveTxtDayBlock should call workflow once.");
  Expect(state,
         pipeline_workflow.last_resolve_txt_day_block_request.day_marker == "0102",
         "RunResolveTxtDayBlock should forward day_marker.");

  const auto replace_response = runtime_api.pipeline().RunReplaceTxtDayBlock({
      .content = "full-content",
      .day_marker = "0102",
      .edited_day_body = "body",
  });
  Expect(state, replace_response.ok,
         "RunReplaceTxtDayBlock should return workflow response.");
  Expect(state, pipeline_workflow.replace_txt_day_block_call_count == 1,
         "RunReplaceTxtDayBlock should call workflow once.");
  Expect(state,
         pipeline_workflow.last_replace_txt_day_block_request.edited_day_body ==
             "body",
         "RunReplaceTxtDayBlock should forward edited body.");

  pipeline_workflow.fail_resolve_txt_day_block = true;
  const auto failure = runtime_api.pipeline().RunResolveTxtDayBlock({
      .content = "full-content",
      .day_marker = "0102",
      .selected_month = "2025-01",
  });
  Expect(state, !failure.ok,
         "RunResolveTxtDayBlock should convert thrown exceptions into failed DTOs.");
  Expect(state, Contains(failure.error_message, "RunResolveTxtDayBlock failed"),
         "RunResolveTxtDayBlock failure should include operation name.");
}

}  // namespace

auto RunTxtDayBlockTests(TestState& state) -> void {
  TestTxtDayBlockSemantics(state);
  TestTxtDayBlockFixtureFiles(state);
  TestTxtDayBlockPipelineApiForwarding(state);
}

}  // namespace tracer_core::application::tests
