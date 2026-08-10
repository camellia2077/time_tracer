// infrastructure/tests/android_runtime/android_runtime_insights_consistency_data_layer_tests.cpp
#include <iostream>
#include <algorithm>
#include <fstream>
#include <variant>

#include "application/aggregate_runtime/i_tracer_core_runtime.hpp"
#include "application/dto/query_requests.hpp"
#include "application/dto/insights_requests.hpp"
#include "application/dto/insights_responses.hpp"
#include "application/dto/shared_envelopes.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_insights_consistency_internal.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_smoke_internal.hpp"

using tracer::core::application::use_cases::ITracerCoreRuntime;

namespace android_runtime_tests::insights_consistency_internal {
namespace {

using tracer_core::core::dto::InsightsDisplayMode;
using tracer_core::core::dto::TemporalSelectionKind;

auto TestDataLayerStructuredFieldVerification(
    const std::shared_ptr<ITracerCoreRuntime>& runtime_api, int& failures)
    -> void {
  const auto structured_result =
      runtime_api->insights().RunTemporalStructuredInsightsQuery(
          {.display_mode = InsightsDisplayMode::kDay,
           .selection = {.kind = TemporalSelectionKind::kSingleDay,
                         .date = "2025-01-03"}});

  if (!structured_result.ok) {
    ++failures;
    std::cerr
        << "[FAIL] DataLayer/FieldVerify: RunTemporalStructuredInsightsQuery"
           "(day, 2026-01-03) should succeed: "
        << structured_result.error_message << '\n';
    return;
  }

  if (structured_result.display_mode != InsightsDisplayMode::kDay) {
    ++failures;
    std::cerr
        << "[FAIL] DataLayer/FieldVerify: Insights display_mode should be day.\n";
    return;
  }

  const auto* daily = std::get_if<DailyInsightsData>(&structured_result.insights);
  if (daily == nullptr) {
    ++failures;
    std::cerr << "[FAIL] DataLayer/FieldVerify: Insights variant should hold "
                 "DailyInsightsData.\n";
    return;
  }

  if (daily->total_duration <= 0) {
    ++failures;
    std::cerr << "[FAIL] DataLayer/FieldVerify: total_duration should be > 0, "
                 "actual: "
              << daily->total_duration << '\n';
  }

  if (daily->detailed_records.empty()) {
    ++failures;
    std::cerr << "[FAIL] DataLayer/FieldVerify: detailed_records should not be "
                 "empty.\n";
  }

  if (daily->project_tree.empty()) {
    ++failures;
    std::cerr
        << "[FAIL] DataLayer/FieldVerify: project_tree should not be empty.\n";
  }

  if (daily->metadata.statuses.size() != 2U) {
    ++failures;
    std::cerr << "[FAIL] DataLayer/FieldVerify: configured daily statuses "
                 "should be present as structured fields.\n";
  } else {
    bool study_counted = false;
    bool exercise_empty = false;
    for (const auto& status : daily->metadata.statuses) {
      if (status.id == "status") {
        study_counted = status.label == "Study" &&
                        status.occurrence_count == 5 &&
                        status.total_duration == 41008;
      } else if (status.id == "exercise") {
        exercise_empty = status.label == "Exercise" &&
                         status.occurrence_count == 1 &&
                         status.total_duration == 48;
      }
    }
    if (!study_counted || !exercise_empty) {
      ++failures;
      std::cerr << "[FAIL] DataLayer/FieldVerify: configured status totals for "
                   "2025-01-03 should match the expected parent records.\n";
    }
  }

  if (!daily->detailed_records.empty()) {
    long long records_sum = 0;
    for (const auto& record : daily->detailed_records) {
      records_sum += record.duration_seconds;
    }
    if (records_sum != daily->total_duration) {
      ++failures;
      std::cerr << "[FAIL] DataLayer/FieldVerify: sum of record durations ("
                << records_sum << ") should equal total_duration ("
                << daily->total_duration << ").\n";
    }
  }

  for (size_t index = 0; index < daily->detailed_records.size(); ++index) {
    const auto& record = daily->detailed_records[index];
    if (record.project_path.empty()) {
      ++failures;
      std::cerr << "[FAIL] DataLayer/FieldVerify: record[" << index
                << "].project_path should not be empty.\n";
      break;
    }
    if (record.kind == ActivityRecordKind::kEndOnly) {
      if (!record.start_time.empty() || record.duration_seconds != 0) {
        ++failures;
        std::cerr << "[FAIL] DataLayer/FieldVerify: end-only activity["
                  << index
                  << "] should have empty start_time and zero duration.\n";
      }
      continue;
    }
    if (record.start_time.empty()) {
      ++failures;
      std::cerr << "[FAIL] DataLayer/FieldVerify: record[" << index
                << "].start_time should not be empty.\n";
      break;
    }
    if (record.end_time.empty()) {
      ++failures;
      std::cerr << "[FAIL] DataLayer/FieldVerify: record[" << index
                << "].end_time should not be empty.\n";
      break;
    }
  }
}

auto TestDataLayerEndOnlyConsistency(
    const std::shared_ptr<ITracerCoreRuntime>& runtime_api, int& failures)
    -> void {
  const auto synthetic_root =
      std::filesystem::temp_directory_path() /
      "time_tracer_insights_consistency_end_only_fixture";
  const auto synthetic_input_path = synthetic_root / "2026" / "2026-03.txt";
  std::error_code cleanup_error;
  std::filesystem::remove_all(synthetic_root, cleanup_error);
  const auto cleanup = [&synthetic_root]() {
    std::error_code error;
    std::filesystem::remove_all(synthetic_root, error);
  };
  std::error_code io_error;
  std::filesystem::create_directories(synthetic_input_path.parent_path(),
                                      io_error);
  if (io_error) {
    ++failures;
    std::cerr << "[FAIL] DataLayer/EndOnly: could not create synthetic input "
                 "directory.\n";
    cleanup();
    return;
  }
  std::ofstream synthetic_input(synthetic_input_path);
  synthetic_input
      << "y2026\nm03\n\nd0302\n"
         "090000study_math_probability-theory_probability-distribution\n"
         "100000study_math_probability-theory_probability-basics_conditional-probability\n";
  synthetic_input.close();
  if (!synthetic_input) {
    ++failures;
    std::cerr << "[FAIL] DataLayer/EndOnly: could not write synthetic input.\n";
    cleanup();
    return;
  }

  tracer_core::core::dto::IngestRequest ingest_request;
  ingest_request.input_path = synthetic_input_path.string();
  ingest_request.date_check_mode = DateCheckMode::kNone;
  ingest_request.ingest_mode = IngestMode::kSingleTxtReplaceMonth;
  const auto ingest_result = runtime_api->pipeline().RunIngest(ingest_request);
  if (!ingest_result.ok) {
    ++failures;
    std::cerr << "[FAIL] DataLayer/EndOnly: synthetic ingest should succeed: "
              << ingest_result.error_message << '\n';
    cleanup();
    return;
  }

  const auto structured_result =
      runtime_api->insights().RunTemporalStructuredInsightsQuery(
          {.display_mode = InsightsDisplayMode::kDay,
           .selection = {.kind = TemporalSelectionKind::kSingleDay,
                         .date = "2026-03-02"}});

  if (!structured_result.ok) {
    ++failures;
    std::cerr << "[FAIL] DataLayer/EndOnly: structured day insights should "
                 "succeed: "
              << structured_result.error_message << '\n';
    cleanup();
    return;
  }

  const auto* daily = std::get_if<DailyInsightsData>(&structured_result.insights);
  if (daily == nullptr) {
    ++failures;
    std::cerr << "[FAIL] DataLayer/EndOnly: insights should hold "
                 "DailyInsightsData.\n";
    cleanup();
    return;
  }

  const auto end_only_it = std::find_if(
      daily->detailed_records.begin(), daily->detailed_records.end(),
      [](const auto& record) { return record.kind == ActivityRecordKind::kEndOnly; });
  if (end_only_it == daily->detailed_records.end()) {
    ++failures;
    std::cerr << "[FAIL] DataLayer/EndOnly: first day should retain an "
                 "end-only timeline activity.\n";
    cleanup();
    return;
  }

  if (!end_only_it->start_time.empty() || end_only_it->end_time.empty() ||
      end_only_it->duration_seconds != 0) {
    ++failures;
    std::cerr << "[FAIL] DataLayer/EndOnly: end-only activity should preserve "
                 "end_time only and zero duration.\n";
  }
  if (daily->activity_count <
      static_cast<int>(daily->detailed_records.size())) {
    ++failures;
    std::cerr << "[FAIL] DataLayer/EndOnly: activity_count should include "
                 "end-only activities.\n";
  }
  cleanup();
}

auto TestDataLayerCrossIngestConsistency(
    const std::shared_ptr<ITracerCoreRuntime>& runtime_api,
    const std::filesystem::path& input_path, int& failures) -> void {
  const std::string target_date = "2025-01-03";

  const tracer_core::core::dto::TemporalInsightsQueryRequest md_request{
      .display_mode = InsightsDisplayMode::kDay,
      .selection =
          {.kind = TemporalSelectionKind::kSingleDay, .date = target_date},
      .format = InsightsFormat::kMarkdown,
  };

  const auto md_before = runtime_api->insights().RunTemporalInsightsQuery(md_request);
  if (!md_before.ok) {
    ++failures;
    std::cerr << "[FAIL] CrossIngest: RunTemporalInsightsQuery(day, before) "
                 "should succeed: "
              << md_before.error_message << '\n';
    return;
  }

  const tracer_core::core::dto::TemporalStructuredInsightsQueryRequest
      struct_request{
          .display_mode = InsightsDisplayMode::kDay,
          .selection = {.kind = TemporalSelectionKind::kSingleDay,
                        .date = target_date},
      };
  const auto data_before =
      runtime_api->insights().RunTemporalStructuredInsightsQuery(struct_request);
  if (!data_before.ok) {
    ++failures;
    std::cerr
        << "[FAIL] CrossIngest: RunTemporalStructuredInsightsQuery(before) "
           "should succeed: "
        << data_before.error_message << '\n';
    return;
  }

  const auto* daily_before = std::get_if<DailyInsightsData>(&data_before.insights);
  if (daily_before == nullptr) {
    ++failures;
    std::cerr << "[FAIL] CrossIngest: before insights should hold "
                 "DailyInsightsData.\n";
    return;
  }

  const long long duration_before = daily_before->total_duration;
  const size_t records_before = daily_before->detailed_records.size();
  const size_t tree_size_before = daily_before->project_tree.size();

  const std::filesystem::path month_txt = input_path / "2025" / "2025-01.txt";
  tracer_core::core::dto::IngestRequest reingest_request;
  reingest_request.input_path = month_txt.string();
  reingest_request.date_check_mode = DateCheckMode::kNone;
  reingest_request.ingest_mode = IngestMode::kSingleTxtReplaceMonth;

  const auto reingest_result =
      runtime_api->pipeline().RunIngest(reingest_request);
  if (!reingest_result.ok) {
    ++failures;
    std::cerr << "[FAIL] CrossIngest: re-ingest (replace_month) should "
                 "succeed: "
              << reingest_result.error_message << '\n';
    return;
  }

  const auto md_after = runtime_api->insights().RunTemporalInsightsQuery(md_request);
  if (!md_after.ok) {
    ++failures;
    std::cerr << "[FAIL] CrossIngest: RunTemporalInsightsQuery(day, after) "
                 "should succeed: "
              << md_after.error_message << '\n';
    return;
  }

  const auto data_after =
      runtime_api->insights().RunTemporalStructuredInsightsQuery(struct_request);
  if (!data_after.ok) {
    ++failures;
    std::cerr
        << "[FAIL] CrossIngest: RunTemporalStructuredInsightsQuery(after) "
           "should succeed: "
        << data_after.error_message << '\n';
    return;
  }

  const auto* daily_after = std::get_if<DailyInsightsData>(&data_after.insights);
  if (daily_after == nullptr) {
    ++failures;
    std::cerr << "[FAIL] CrossIngest: after insights should hold "
                 "DailyInsightsData.\n";
    return;
  }

  if (daily_after->total_duration != duration_before) {
    ++failures;
    std::cerr << "[FAIL] CrossIngest: total_duration changed after re-ingest ("
              << duration_before << " -> " << daily_after->total_duration
              << ").\n";
  }

  if (daily_after->detailed_records.size() != records_before) {
    ++failures;
    std::cerr << "[FAIL] CrossIngest: record count changed after re-ingest ("
              << records_before << " -> "
              << daily_after->detailed_records.size() << ").\n";
  }

  if (daily_after->project_tree.size() != tree_size_before) {
    ++failures;
    std::cerr << "[FAIL] CrossIngest: project_tree root count changed after "
                 "re-ingest ("
              << tree_size_before << " -> " << daily_after->project_tree.size()
              << ").\n";
  }

  if (md_before.content != md_after.content) {
    ++failures;
    std::cerr << "[FAIL] CrossIngest: MD content differs after re-ingest. "
              << BuildDiffDiagnostics(md_before.content, md_after.content)
              << ".\n";
  }

  const std::string hash_before = ComputeSha256Hex(md_before.content);
  const std::string hash_after = ComputeSha256Hex(md_after.content);
  if (hash_before != hash_after) {
    ++failures;
    std::cerr << "[FAIL] CrossIngest: MD hash differs after re-ingest. "
              << "before_sha256=" << hash_before
              << ", after_sha256=" << hash_after << ".\n";
  }
}

}  // namespace

auto RunInsightsConsistencyFieldVerificationTests(
    const std::shared_ptr<ITracerCoreRuntime>& runtime_api, int& failures)
    -> void {
  TestDataLayerStructuredFieldVerification(runtime_api, failures);
  TestDataLayerEndOnlyConsistency(runtime_api, failures);
}

auto RunInsightsConsistencyCrossIngestTests(
    const std::shared_ptr<ITracerCoreRuntime>& runtime_api,
    const std::filesystem::path& input_path, int& failures) -> void {
  TestDataLayerCrossIngestConsistency(runtime_api, input_path, failures);
}

}  // namespace android_runtime_tests::insights_consistency_internal
