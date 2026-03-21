// infrastructure/tests/android_runtime/android_runtime_report_consistency_data_layer_tests.cpp
#include <iostream>
#include <variant>

#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"
#include "application/use_cases/i_tracer_core_runtime.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_report_consistency_internal.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_smoke_internal.hpp"

using tracer::core::application::use_cases::ITracerCoreRuntime;

namespace android_runtime_tests::report_consistency_internal {
namespace {

auto TestDataLayerStructuredFieldVerification(
    const std::shared_ptr<ITracerCoreRuntime>& runtime_api, int& failures) -> void {
  tracer_core::core::dto::StructuredReportQueryRequest structured_request;
  structured_request.type = tracer_core::core::dto::ReportQueryType::kDay;
  structured_request.argument = "2025-01-03";

  const auto structured_result =
      runtime_api->report().RunStructuredReportQuery(structured_request);

  if (!structured_result.ok) {
    ++failures;
    std::cerr << "[FAIL] DataLayer/FieldVerify: RunStructuredReportQuery(day, "
                 "2026-01-03) should succeed: "
              << structured_result.error_message << '\n';
    return;
  }

  if (structured_result.kind !=
      tracer_core::core::dto::StructuredReportKind::kDay) {
    ++failures;
    std::cerr << "[FAIL] DataLayer/FieldVerify: Report kind should be kDay.\n";
    return;
  }

  const auto* daily = std::get_if<DailyReportData>(&structured_result.report);
  if (daily == nullptr) {
    ++failures;
    std::cerr << "[FAIL] DataLayer/FieldVerify: Report variant should hold "
                 "DailyReportData.\n";
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

auto TestDataLayerCrossIngestConsistency(
    const std::shared_ptr<ITracerCoreRuntime>& runtime_api,
    const std::filesystem::path& input_path, int& failures) -> void {
  const std::string target_date = "2025-01-03";

  tracer_core::core::dto::ReportQueryRequest md_request;
  md_request.type = tracer_core::core::dto::ReportQueryType::kDay;
  md_request.argument = target_date;
  md_request.format = ReportFormat::kMarkdown;

  const auto md_before = runtime_api->report().RunReportQuery(md_request);
  if (!md_before.ok) {
    ++failures;
    std::cerr << "[FAIL] CrossIngest: RunReportQuery(day, before) should "
                 "succeed: "
              << md_before.error_message << '\n';
    return;
  }

  tracer_core::core::dto::StructuredReportQueryRequest struct_request;
  struct_request.type = tracer_core::core::dto::ReportQueryType::kDay;
  struct_request.argument = target_date;
  const auto data_before =
      runtime_api->report().RunStructuredReportQuery(struct_request);
  if (!data_before.ok) {
    ++failures;
    std::cerr << "[FAIL] CrossIngest: RunStructuredReportQuery(before) should "
                 "succeed: "
              << data_before.error_message << '\n';
    return;
  }

  const auto* daily_before = std::get_if<DailyReportData>(&data_before.report);
  if (daily_before == nullptr) {
    ++failures;
    std::cerr << "[FAIL] CrossIngest: before report should hold "
                 "DailyReportData.\n";
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

  const auto reingest_result = runtime_api->pipeline().RunIngest(reingest_request);
  if (!reingest_result.ok) {
    ++failures;
    std::cerr << "[FAIL] CrossIngest: re-ingest (replace_month) should "
                 "succeed: "
              << reingest_result.error_message << '\n';
    return;
  }

  const auto md_after = runtime_api->report().RunReportQuery(md_request);
  if (!md_after.ok) {
    ++failures;
    std::cerr << "[FAIL] CrossIngest: RunReportQuery(day, after) should "
                 "succeed: "
              << md_after.error_message << '\n';
    return;
  }

  const auto data_after =
      runtime_api->report().RunStructuredReportQuery(struct_request);
  if (!data_after.ok) {
    ++failures;
    std::cerr << "[FAIL] CrossIngest: RunStructuredReportQuery(after) should "
                 "succeed: "
              << data_after.error_message << '\n';
    return;
  }

  const auto* daily_after = std::get_if<DailyReportData>(&data_after.report);
  if (daily_after == nullptr) {
    ++failures;
    std::cerr << "[FAIL] CrossIngest: after report should hold "
                 "DailyReportData.\n";
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

auto RunReportConsistencyFieldVerificationTests(
    const std::shared_ptr<ITracerCoreRuntime>& runtime_api, int& failures) -> void {
  TestDataLayerStructuredFieldVerification(runtime_api, failures);
}

auto RunReportConsistencyCrossIngestTests(
    const std::shared_ptr<ITracerCoreRuntime>& runtime_api,
    const std::filesystem::path& input_path, int& failures) -> void {
  TestDataLayerCrossIngestConsistency(runtime_api, input_path, failures);
}

}  // namespace android_runtime_tests::report_consistency_internal
