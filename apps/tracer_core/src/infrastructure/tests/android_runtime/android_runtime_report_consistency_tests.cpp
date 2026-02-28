// infrastructure/tests/android_runtime/android_runtime_report_consistency_tests.cpp
#include <sodium.h>

#include <algorithm>
#include <array>
#include <exception>
#include <iostream>
#include <numeric>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_smoke_internal.hpp"

#ifndef TT_ENABLE_HEAVY_DIAGNOSTICS
#define TT_ENABLE_HEAVY_DIAGNOSTICS 0
#endif

namespace android_runtime_tests {
namespace {

// NOLINTBEGIN(readability-magic-numbers)

auto ToHexLower(const unsigned char* bytes, size_t size) -> std::string {
  constexpr char kHex[] = "0123456789abcdef";
  std::string text(size * 2U, '\0');
  for (size_t index = 0; index < size; ++index) {
    const unsigned char value = bytes[index];
    text[index * 2U] = kHex[(value >> 4U) & 0x0FU];
    text[index * 2U + 1U] = kHex[value & 0x0FU];
  }
  return text;
}

auto ComputeSha256Hex(std::string_view text) -> std::string {
  std::array<unsigned char, crypto_hash_sha256_BYTES> digest{};
  const auto* input = reinterpret_cast<const unsigned char*>(text.data());
  if (crypto_hash_sha256(digest.data(), input,
                         static_cast<unsigned long long>(text.size())) != 0) {
    return "sha256_error";
  }
  return ToHexLower(digest.data(), digest.size());
}

#if TT_ENABLE_HEAVY_DIAGNOSTICS
auto BuildDiffContext(std::string_view text, size_t offset) -> std::string {
  constexpr size_t kRadius = 24;
  const size_t begin = (offset > kRadius) ? (offset - kRadius) : 0U;
  const size_t end = std::min(text.size(), offset + kRadius);
  std::string snippet(text.substr(begin, end - begin));
  for (char& value : snippet) {
    if (value == '\n' || value == '\r' || value == '\t') {
      value = ' ';
    }
  }
  return snippet;
}
#endif

auto BuildDiffDiagnostics(std::string_view left, std::string_view right)
    -> std::string {
  const size_t max_len = std::min(left.size(), right.size());
  for (size_t index = 0; index < max_len; ++index) {
    if (left[index] == right[index]) {
      continue;
    }
    int line = 1;
    int column = 1;
    for (size_t pos = 0; pos < index; ++pos) {
      if (left[pos] == '\n') {
        ++line;
        column = 1;
      } else {
        ++column;
      }
    }

    std::ostringstream output;
    output << "first mismatch at line " << line << ", column " << column
           << ", byte offset " << index
           << "; left_sha256=" << ComputeSha256Hex(left)
           << ", right_sha256=" << ComputeSha256Hex(right);
#if TT_ENABLE_HEAVY_DIAGNOSTICS
    output << "; left_context=\"" << BuildDiffContext(left, index)
           << "\", right_context=\"" << BuildDiffContext(right, index) << "\"";
#endif
    return output.str();
  }

  if (left.size() != right.size()) {
    std::ostringstream output;
    output << "content length differs (left=" << left.size()
           << ", right=" << right.size()
           << "); left_sha256=" << ComputeSha256Hex(left)
           << ", right_sha256=" << ComputeSha256Hex(right);
    return output.str();
  }
  return "unknown mismatch.";
}

// ---------------------------------------------------------------------------
// Test 1: Data Layer — Structured Report Field Verification
// ---------------------------------------------------------------------------
auto TestDataLayerStructuredFieldVerification(
    const std::shared_ptr<ITracerCoreApi>& core_api, int& failures) -> void {
  // Query a day known to have records (2025-01-03 from test/data/2025/)
  tracer_core::core::dto::StructuredReportQueryRequest structured_request;
  structured_request.type = tracer_core::core::dto::ReportQueryType::kDay;
  structured_request.argument = "2025-01-03";

  const auto structured_result =
      core_api->RunStructuredReportQuery(structured_request);

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

  // 1a. total_duration > 0
  if (daily->total_duration <= 0) {
    ++failures;
    std::cerr << "[FAIL] DataLayer/FieldVerify: total_duration should be > 0, "
                 "actual: "
              << daily->total_duration << '\n';
  }

  // 1b. detailed_records non-empty
  if (daily->detailed_records.empty()) {
    ++failures;
    std::cerr << "[FAIL] DataLayer/FieldVerify: detailed_records should not be "
                 "empty.\n";
  }

  // 1c. project_tree has nodes
  if (daily->project_tree.empty()) {
    ++failures;
    std::cerr
        << "[FAIL] DataLayer/FieldVerify: project_tree should not be empty.\n";
  }

  // 1d. Sum of record durations equals total_duration
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

  // 1e. Every record has non-empty project_path, start_time, end_time
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

// ---------------------------------------------------------------------------
// Test 2: Data Layer — Cross-Ingest Consistency (Same Runtime)
// ---------------------------------------------------------------------------
auto TestDataLayerCrossIngestConsistency(
    const std::shared_ptr<ITracerCoreApi>& core_api,
    const std::filesystem::path& input_path, int& failures) -> void {
  const std::string target_date = "2025-01-03";

  // --- Before snapshot ---
  tracer_core::core::dto::ReportQueryRequest md_request;
  md_request.type = tracer_core::core::dto::ReportQueryType::kDay;
  md_request.argument = target_date;
  md_request.format = ReportFormat::kMarkdown;

  const auto md_before = core_api->RunReportQuery(md_request);
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
  const auto data_before = core_api->RunStructuredReportQuery(struct_request);
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

  // --- Re-ingest the same month via kSingleTxtReplaceMonth ---
  const std::filesystem::path month_txt = input_path / "2025" / "2025-01.txt";
  tracer_core::core::dto::IngestRequest reingest_request;
  reingest_request.input_path = month_txt.string();
  reingest_request.date_check_mode = DateCheckMode::kNone;
  reingest_request.ingest_mode = IngestMode::kSingleTxtReplaceMonth;

  const auto reingest_result = core_api->RunIngest(reingest_request);
  if (!reingest_result.ok) {
    ++failures;
    std::cerr << "[FAIL] CrossIngest: re-ingest (replace_month) should "
                 "succeed: "
              << reingest_result.error_message << '\n';
    return;
  }

  // --- After snapshot ---
  const auto md_after = core_api->RunReportQuery(md_request);
  if (!md_after.ok) {
    ++failures;
    std::cerr << "[FAIL] CrossIngest: RunReportQuery(day, after) should "
                 "succeed: "
              << md_after.error_message << '\n';
    return;
  }

  const auto data_after = core_api->RunStructuredReportQuery(struct_request);
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

  // --- Assert data consistency ---
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

  // --- Assert MD raw bytes consistency (no newline normalization) ---
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

// ---------------------------------------------------------------------------
// Test 3: Structure Layer — MD Section Integrity
// ---------------------------------------------------------------------------
auto TestStructureLayerMdSectionIntegrity(
    const std::shared_ptr<ITracerCoreApi>& core_api, int& failures) -> void {
  // --- Daily report structural checks ---
  tracer_core::core::dto::ReportQueryRequest day_request;
  day_request.type = tracer_core::core::dto::ReportQueryType::kDay;
  day_request.argument = "2025-01-03";
  day_request.format = ReportFormat::kMarkdown;

  const auto day_result = core_api->RunReportQuery(day_request);
  if (!day_result.ok) {
    ++failures;
    std::cerr << "[FAIL] StructureLayer/Day: RunReportQuery should succeed: "
              << day_result.error_message << '\n';
    return;
  }

  const std::string& day_md = day_result.content;

  struct SectionCheck {
    const char* label;
    const char* marker;
  };

  const SectionCheck day_checks[] = {
      {"title (Daily Report for)", "# Daily Report for"},
      {"All Activities section", "## All Activities"},
      {"Project Breakdown section", "## Project Breakdown"},
      {"Date label", "- **Date**:"},
      {"Total Time Recorded label", "- **Total Time Recorded**:"},
      {"Statistics section", "## Statistics"},
  };

  for (const auto& check : day_checks) {
    if (!Contains(day_md, check.marker)) {
      ++failures;
      std::cerr << "[FAIL] StructureLayer/Day: MD should contain "
                << check.label << " ('" << check.marker << "').\n";
    }
  }

  // --- Monthly report structural checks ---
  tracer_core::core::dto::ReportQueryRequest month_request;
  month_request.type = tracer_core::core::dto::ReportQueryType::kMonth;
  month_request.argument = "2025-01";
  month_request.format = ReportFormat::kMarkdown;

  const auto month_result = core_api->RunReportQuery(month_request);
  if (!month_result.ok) {
    ++failures;
    std::cerr << "[FAIL] StructureLayer/Month: RunReportQuery should succeed: "
              << month_result.error_message << '\n';
    return;
  }

  const std::string& month_md = month_result.content;

  const SectionCheck month_checks[] = {
      {"title (Monthly Summary)", "# Monthly Summary for"},
      {"Project Breakdown section", "## Project Breakdown"},
      {"Total Time Recorded label", "- **Total Time Recorded**:"},
  };

  for (const auto& check : month_checks) {
    if (!Contains(month_md, check.marker)) {
      ++failures;
      std::cerr << "[FAIL] StructureLayer/Month: MD should contain "
                << check.label << " ('" << check.marker << "').\n";
    }
  }
}

// NOLINTEND(readability-magic-numbers)

}  // namespace

auto RunReportConsistencyTests(int& failures) -> void {
  auto fixture_opt = smoke::BuildRuntimeFixture(
      "time_tracer_report_consistency_test", failures);
  if (!fixture_opt.has_value()) {
    return;
  }

  auto fixture = std::move(*fixture_opt);
  try {
    // Ingest all test data
    tracer_core::core::dto::IngestRequest ingest_request;
    ingest_request.input_path = (fixture.input_path / "2025").string();
    ingest_request.date_check_mode = DateCheckMode::kNone;
    const auto ingest_result =
        fixture.runtime.core_api->RunIngest(ingest_request);
    if (!ingest_result.ok) {
      ++failures;
      std::cerr << "[FAIL] ReportConsistency: initial ingest should succeed: "
                << ingest_result.error_message << '\n';
      smoke::CleanupRuntimeFixture(fixture);
      return;
    }

    // Test 1: Data layer field verification
    TestDataLayerStructuredFieldVerification(fixture.runtime.core_api,
                                             failures);

    // Test 3: Structure layer MD section integrity (run before re-ingest)
    TestStructureLayerMdSectionIntegrity(fixture.runtime.core_api, failures);

    // Test 2: Cross-ingest consistency (does re-ingest internally)
    TestDataLayerCrossIngestConsistency(fixture.runtime.core_api,
                                        fixture.input_path, failures);
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] ReportConsistency tests threw exception: "
              << exception.what() << '\n';
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] ReportConsistency tests threw non-standard "
                 "exception.\n";
  }

  smoke::CleanupRuntimeFixture(fixture);
}

}  // namespace android_runtime_tests
