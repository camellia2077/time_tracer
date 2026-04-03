#include "application/pipeline/pipeline_workflow.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iomanip>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <set>
#include <sstream>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "application/pipeline/importer/import_service.hpp"
#include "application/pipeline/detail/pipeline_record_time_order_support.hpp"
#include "application/runtime_bridge/logger.hpp"
#include "domain/logic/converter/convert/core/converter_core.hpp"
#include "domain/logic/converter/log_processor.hpp"
#include "domain/model/daily_log.hpp"
#include "domain/ports/diagnostics.hpp"
#include "shared/utils/canonical_text.hpp"
#include "shared/utils/string_utils.hpp"

import tracer.core.application.pipeline.orchestrator;
import tracer.core.application.pipeline.stages;
import tracer.core.application.pipeline.types;
import tracer.core.domain.types.app_options;

namespace tracer::core::application::pipeline {

namespace fs = std::filesystem;
namespace app_ports = tracer_core::application::ports;
namespace runtime_bridge = tracer_core::application::runtime_bridge;
namespace modtext = tracer::core::shared::canonical_text;
namespace modports = tracer_core::domain::ports;
using tracer::core::domain::types::AppOptions;
using tracer::core::shared::string_utils::Trim;
using tracer_core::application::dto::IngestInputModel;
using tracer_core::core::dto::IngestSyncStatusEntry;
using tracer_core::core::dto::IngestSyncStatusOutput;
using tracer_core::core::dto::IngestSyncStatusRequest;
using tracer_core::core::dto::RecordActivityAtomicallyRequest;
using tracer_core::core::dto::RecordActivityAtomicallyResponse;

namespace {

#include "application/pipeline/detail/pipeline_workflow_support_impl.inc"
#include "application/pipeline/detail/pipeline_replace_month_support_impl.inc"
#include "application/pipeline/detail/pipeline_record_alias_text_support_impl.inc"
#include "application/pipeline/detail/pipeline_record_atomic_support_impl.inc"

struct ConverterConfigPathSet {
  fs::path main_config_path;
  fs::path alias_mapping_path;
  fs::path duration_rules_path;
};

[[nodiscard]] auto ResolveConverterConfigPathSet(
    const fs::path& main_config_path) -> ConverterConfigPathSet {
  if (main_config_path.empty()) {
    throw std::invalid_argument(
        "Converter main config path must not be empty.");
  }

  const fs::path resolved_main_config_path = fs::absolute(main_config_path);
  const fs::path config_dir = resolved_main_config_path.parent_path();
  return {
      .main_config_path = resolved_main_config_path,
      .alias_mapping_path = config_dir / "alias_mapping.toml",
      .duration_rules_path = config_dir / "duration_rules.toml",
  };
}

auto EnsureConverterConfigSourceExists(const fs::path& path,
                                       std::string_view label) -> void {
  if (!fs::exists(path) || !fs::is_regular_file(path)) {
    throw std::runtime_error(std::string(label) +
                             " must be an existing file: " + path.string());
  }
}

auto CopyConverterConfigFile(const fs::path& source_path,
                             const fs::path& target_path,
                             std::string_view label) -> void {
  std::error_code io_error;
  fs::create_directories(target_path.parent_path(), io_error);
  if (io_error) {
    throw std::runtime_error("Failed to prepare " + std::string(label) +
                             " target directory: " + target_path.string() +
                             " | " + io_error.message());
  }

  fs::copy_file(source_path, target_path, fs::copy_options::overwrite_existing,
                io_error);
  if (io_error) {
    throw std::runtime_error("Failed to install " + std::string(label) +
                             ": " + source_path.string() + " -> " +
                             target_path.string() + " | " +
                             io_error.message());
  }
}

[[nodiscard]] auto BuildCanonicalMonthRelativePath(
    const SingleTxtTargetMonth& month) -> std::string {
  return std::format("{0:04d}/{0:04d}-{1:02d}.txt", month.year, month.month);
}


[[nodiscard]] constexpr auto RotateRight(const std::uint32_t value,
                                         const std::uint32_t amount)
    -> std::uint32_t {
  return (value >> amount) | (value << (32U - amount));
}

[[nodiscard]] constexpr auto BigSigma0(const std::uint32_t value)
    -> std::uint32_t {
  return RotateRight(value, 2U) ^ RotateRight(value, 13U) ^
         RotateRight(value, 22U);
}

[[nodiscard]] constexpr auto BigSigma1(const std::uint32_t value)
    -> std::uint32_t {
  return RotateRight(value, 6U) ^ RotateRight(value, 11U) ^
         RotateRight(value, 25U);
}

[[nodiscard]] constexpr auto SmallSigma0(const std::uint32_t value)
    -> std::uint32_t {
  return RotateRight(value, 7U) ^ RotateRight(value, 18U) ^ (value >> 3U);
}

[[nodiscard]] constexpr auto SmallSigma1(const std::uint32_t value)
    -> std::uint32_t {
  return RotateRight(value, 17U) ^ RotateRight(value, 19U) ^ (value >> 10U);
}

[[nodiscard]] constexpr auto Choose(const std::uint32_t x,
                                    const std::uint32_t y,
                                    const std::uint32_t z) -> std::uint32_t {
  return (x & y) ^ (~x & z);
}

[[nodiscard]] constexpr auto Majority(const std::uint32_t x,
                                      const std::uint32_t y,
                                      const std::uint32_t z)
    -> std::uint32_t {
  return (x & y) ^ (x & z) ^ (y & z);
}

[[nodiscard]] constexpr auto kSha256RoundConstants()
    -> std::array<std::uint32_t, 64> {
  // SHA-256 round constants are fixed by the algorithm specification.
  // NOLINTBEGIN(readability-magic-numbers)
  return {
      0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U, 0x3956c25bU,
      0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U, 0xd807aa98U, 0x12835b01U,
      0x243185beU, 0x550c7dc3U, 0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U,
      0xc19bf174U, 0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU,
      0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU, 0x983e5152U,
      0xa831c66dU, 0xb00327c8U, 0xbf597fc7U, 0xc6e00bf3U, 0xd5a79147U,
      0x06ca6351U, 0x14292967U, 0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU,
      0x53380d13U, 0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
      0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U, 0xd192e819U,
      0xd6990624U, 0xf40e3585U, 0x106aa070U, 0x19a4c116U, 0x1e376c08U,
      0x2748774cU, 0x34b0bcb5U, 0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU,
      0x682e6ff3U, 0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,
      0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U,
  };
  // NOLINTEND(readability-magic-numbers)
}

[[nodiscard]] auto ComputeSha256Bytes(std::span<const std::uint8_t> data)
    -> std::array<std::uint8_t, 32> {
  static constexpr auto kRoundConstants = kSha256RoundConstants();
  std::array<std::uint32_t, 8> state = {
      0x6a09e667U, 0xbb67ae85U, 0x3c6ef372U, 0xa54ff53aU,
      0x510e527fU, 0x9b05688cU, 0x1f83d9abU, 0x5be0cd19U,
  };

  std::vector<std::uint8_t> padded(data.begin(), data.end());
  const std::uint64_t bit_length =
      static_cast<std::uint64_t>(data.size()) * 8ULL;
  padded.push_back(0x80U);
  while ((padded.size() % 64U) != 56U) {
    padded.push_back(0x00U);
  }
  for (int shift = 56; shift >= 0; shift -= 8) {
    padded.push_back(
        static_cast<std::uint8_t>((bit_length >> shift) & 0xFFULL));
  }

  std::array<std::uint32_t, 64> schedule{};
  for (std::size_t offset = 0; offset < padded.size(); offset += 64U) {
    for (std::size_t index = 0; index < 16U; ++index) {
      const std::size_t base = offset + index * 4U;
      schedule[index] =
          (static_cast<std::uint32_t>(padded[base]) << 24U) |
          (static_cast<std::uint32_t>(padded[base + 1U]) << 16U) |
          (static_cast<std::uint32_t>(padded[base + 2U]) << 8U) |
          static_cast<std::uint32_t>(padded[base + 3U]);
    }
    for (std::size_t index = 16U; index < 64U; ++index) {
      schedule[index] = SmallSigma1(schedule[index - 2U]) +
                        schedule[index - 7U] +
                        SmallSigma0(schedule[index - 15U]) +
                        schedule[index - 16U];
    }

    std::uint32_t a = state[0];
    std::uint32_t b = state[1];
    std::uint32_t c = state[2];
    std::uint32_t d = state[3];
    std::uint32_t e = state[4];
    std::uint32_t f = state[5];
    std::uint32_t g = state[6];
    std::uint32_t h = state[7];

    for (std::size_t index = 0; index < 64U; ++index) {
      const std::uint32_t temp1 =
          h + BigSigma1(e) + Choose(e, f, g) + kRoundConstants[index] +
          schedule[index];
      const std::uint32_t temp2 = BigSigma0(a) + Majority(a, b, c);
      h = g;
      g = f;
      f = e;
      e = d + temp1;
      d = c;
      c = b;
      b = a;
      a = temp1 + temp2;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    state[5] += f;
    state[6] += g;
    state[7] += h;
  }

  std::array<std::uint8_t, 32> digest{};
  for (std::size_t index = 0; index < state.size(); ++index) {
    const std::uint32_t word = state[index];
    digest[index * 4U] = static_cast<std::uint8_t>((word >> 24U) & 0xFFU);
    digest[index * 4U + 1U] =
        static_cast<std::uint8_t>((word >> 16U) & 0xFFU);
    digest[index * 4U + 2U] =
        static_cast<std::uint8_t>((word >> 8U) & 0xFFU);
    digest[index * 4U + 3U] = static_cast<std::uint8_t>(word & 0xFFU);
  }
  return digest;
}

[[nodiscard]] auto ComputeSha256Hex(std::string_view canonical_text)
    -> std::string {
  const auto* input =
      reinterpret_cast<const std::uint8_t*>(canonical_text.data());
  const auto digest = ComputeSha256Bytes(
      std::span<const std::uint8_t>(input, canonical_text.size()));

  std::ostringstream stream;
  stream << std::hex << std::setfill('0');
  for (const std::uint8_t byte : digest) {
    stream << std::setw(2) << static_cast<unsigned int>(byte);
  }
  return stream.str();
}

[[nodiscard]] auto CurrentUnixMillis() -> std::int64_t {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch())
      .count();
}

[[nodiscard]] auto TryBuildIngestSyncEntry(const IngestInputModel& input,
                                           const std::int64_t ingested_at_ms)
    -> std::optional<IngestSyncStatusEntry> {
  const auto canonical = modtext::Canonicalize(
      input.content, input.source_label.empty() ? input.source_id
                                                : input.source_label);
  if (!canonical.ok) {
    runtime_bridge::LogWarn("Skipping ingest sync snapshot due to invalid TXT: " +
                            canonical.error_message);
    return std::nullopt;
  }

  const auto target_month =
      TryParseSingleTxtTargetMonthFromContent(canonical.text);
  if (!target_month.has_value()) {
    runtime_bridge::LogWarn(
        "Skipping ingest sync snapshot because TXT month header is missing: " +
        (input.source_label.empty() ? input.source_id : input.source_label));
    return std::nullopt;
  }

  return IngestSyncStatusEntry{
      .month_key = target_month->month_key,
      .txt_relative_path = BuildCanonicalMonthRelativePath(*target_month),
      .txt_content_hash_sha256 = ComputeSha256Hex(canonical.text),
      .ingested_at_unix_ms = ingested_at_ms,
  };
}

[[nodiscard]] auto BuildIngestSyncSnapshot(const PipelineSession& context)
    -> std::vector<IngestSyncStatusEntry> {
  std::map<std::string, IngestSyncStatusEntry> unique_entries;
  std::set<std::string> duplicate_months;
  const std::int64_t ingested_at_ms = CurrentUnixMillis();

  for (const auto& input : context.state.ingest_inputs) {
    const auto entry = TryBuildIngestSyncEntry(input, ingested_at_ms);
    if (!entry.has_value()) {
      continue;
    }

    if (duplicate_months.contains(entry->month_key)) {
      continue;
    }

    const auto insert_result =
        unique_entries.emplace(entry->month_key, *entry);
    if (!insert_result.second) {
      duplicate_months.insert(entry->month_key);
      unique_entries.erase(entry->month_key);
      runtime_bridge::LogWarn(
          "Duplicate TXT month detected during ingest sync snapshot: " +
          entry->month_key +
          ". Sync row will be omitted for this month.");
    }
  }

  std::vector<IngestSyncStatusEntry> snapshot;
  snapshot.reserve(unique_entries.size());
  for (auto& [month_key, entry] : unique_entries) {
    (void)month_key;
    snapshot.push_back(std::move(entry));
  }
  return snapshot;
}

auto PersistIngestSyncSnapshot(const PipelineSession& context,
                               app_ports::ITimeSheetRepository& repository)
    -> void {
  repository.ReplaceIngestSyncStatuses(BuildIngestSyncSnapshot(context));
}

auto PersistSingleIngestSyncEntry(const PipelineSession& context,
                                  app_ports::ITimeSheetRepository& repository)
    -> void {
  if (context.state.ingest_inputs.size() != 1U) {
    throw std::runtime_error(
        "Single TXT ingest sync snapshot requires exactly one input.");
  }

  const auto entry =
      TryBuildIngestSyncEntry(context.state.ingest_inputs.front(),
                              CurrentUnixMillis());
  if (!entry.has_value()) {
    throw std::runtime_error(
        "Single TXT ingest sync snapshot requires valid yYYYY + mMM headers.");
  }

  repository.UpsertIngestSyncStatus(*entry);
}

}  // namespace

PipelineWorkflow::PipelineWorkflow(
    fs::path output_root_path, ProcessedDataLoaderPtr processed_data_loader,
    TimeSheetRepositoryPtr time_sheet_repository,
    DatabaseHealthCheckerPtr database_health_checker,
    ConverterConfigProviderPtr converter_config_provider,
    IngestInputProviderPtr ingest_input_provider,
    ProcessedDataStoragePtr processed_data_storage,
    ValidationIssueReporterPtr validation_issue_reporter)
    : output_root_path_(std::move(output_root_path)),
      processed_data_loader_(std::move(processed_data_loader)),
      time_sheet_repository_(std::move(time_sheet_repository)),
      database_health_checker_(std::move(database_health_checker)),
      converter_config_provider_(std::move(converter_config_provider)),
      ingest_input_provider_(std::move(ingest_input_provider)),
      processed_data_storage_(std::move(processed_data_storage)),
      validation_issue_reporter_(std::move(validation_issue_reporter)) {
  if (!processed_data_loader_ || !time_sheet_repository_ ||
      !database_health_checker_ || !converter_config_provider_ ||
      !ingest_input_provider_ || !processed_data_storage_ ||
      !validation_issue_reporter_) {
    throw std::invalid_argument(
        "PipelineWorkflow dependencies must not be null.");
  }
}

PipelineWorkflow::~PipelineWorkflow() = default;

auto PipelineWorkflow::RunConverter(const std::string& input_path,
                                    const AppOptions& options) -> void {
  (void)input_path;
  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_, processed_data_storage_,
                                validation_issue_reporter_);
  RunPipelineOrThrow(pipeline, options, "Converter Pipeline Failed.");
}

auto PipelineWorkflow::RunValidateStructure(const std::string& source_path)
    -> void {
  modports::ClearBufferedDiagnostics();
  const AppOptions options = BuildStructureValidationOptions(source_path);
  const ScopedErrorReportWriterOverride disable_error_reports(nullptr);

  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_, processed_data_storage_,
                                validation_issue_reporter_);
  RunPipelineOrThrow(pipeline, options, "Validate structure pipeline failed.");
}

auto PipelineWorkflow::RunValidateLogic(const std::string& source_path,
                                        DateCheckMode date_check_mode) -> void {
  modports::ClearBufferedDiagnostics();
  const AppOptions options =
      BuildLogicValidationOptions(source_path, date_check_mode);
  const ScopedErrorReportWriterOverride disable_error_reports(nullptr);

  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_, processed_data_storage_,
                                validation_issue_reporter_);
  RunPipelineOrThrow(pipeline, options, "Validate logic pipeline failed.");
}

auto PipelineWorkflow::RunRecordActivityAtomically(
    const RecordActivityAtomicallyRequest& request)
    -> RecordActivityAtomicallyResponse {
  // This is orchestration only: delegate atomic TXT candidate build/validate/ingest+rollback
  // to dedicated record helpers, while keeping workflow-owned RunIngest invocation here.
  return RunRecordActivityAtomicallySupport(
      request, output_root_path_, *converter_config_provider_,
      validation_issue_reporter_,
      [this](const std::string& source_path,
             const DateCheckMode date_check_mode) -> void {
        RunIngest(source_path, date_check_mode, false,
                  IngestMode::kSingleTxtReplaceMonth);
      });
}

auto PipelineWorkflow::InstallActiveConverterConfig(
    const std::string& source_main_config_path,
    const std::string& target_main_config_path) -> void {
  const auto source_paths =
      ResolveConverterConfigPathSet(source_main_config_path);
  const auto target_paths =
      ResolveConverterConfigPathSet(target_main_config_path);

  EnsureConverterConfigSourceExists(source_paths.main_config_path,
                                    "Converter main config");
  EnsureConverterConfigSourceExists(source_paths.alias_mapping_path,
                                    "Alias mapping config");
  EnsureConverterConfigSourceExists(source_paths.duration_rules_path,
                                    "Duration rules config");

  CopyConverterConfigFile(source_paths.main_config_path,
                          target_paths.main_config_path,
                          "converter main config");
  CopyConverterConfigFile(source_paths.alias_mapping_path,
                          target_paths.alias_mapping_path,
                          "alias mapping config");
  CopyConverterConfigFile(source_paths.duration_rules_path,
                          target_paths.duration_rules_path,
                          "duration rules config");
  converter_config_provider_->InvalidateCache();
}

auto PipelineWorkflow::RunDatabaseImport(const std::string& processed_path_str)
    -> void {
  runtime_bridge::LogInfo("正在解析 JSON 数据...");

  auto load_result = processed_data_loader_->LoadDailyLogs(processed_path_str);
  for (const auto& error : load_result.errors) {
    runtime_bridge::LogError("解析文件失败 " + error.source + ": " +
                             error.message);
  }
  if (load_result.data_by_source.empty()) {
    runtime_bridge::LogWarn("没有有效的 JSON 数据可供导入。");
    return;
  }

  RunDatabaseImportFromMemory(load_result.data_by_source);
}

auto PipelineWorkflow::RunDatabaseImportFromMemory(
    const std::map<std::string, std::vector<DailyLog>>& data_map) -> void {
  runtime_bridge::LogInfo("Task: Memory Import...");
  ImportService service(*time_sheet_repository_);
  ImportStats stats = service.ImportFromMemory(data_map);
  PrintImportStats(stats, "Memory Import");
  ThrowIfImportTaskFailed(stats, "Memory import failed.");
}

auto PipelineWorkflow::RunDatabaseImportFromMemoryReplacingAll(
    const std::map<std::string, std::vector<DailyLog>>& data_map) -> void {
  runtime_bridge::LogInfo("Task: Memory Import (Replace All)...");
  ImportService service(*time_sheet_repository_);
  ImportStats stats =
      service.ImportFromMemory(data_map, std::nullopt, ReplaceAllTarget{});
  PrintImportStats(stats, "Memory Import (replace all)");
  ThrowIfImportTaskFailed(stats, "Memory import (replace all) failed.");
}

auto PipelineWorkflow::RunDatabaseImportFromMemoryReplacingMonth(
    const std::map<std::string, std::vector<DailyLog>>& data_map, int year,
    int month) -> void {
  runtime_bridge::LogInfo("Task: Memory Import (Replace Month)...");
  ImportService service(*time_sheet_repository_);
  ImportStats stats = service.ImportFromMemory(
      data_map, ReplaceMonthTarget{.kYear = year, .kMonth = month});
  PrintImportStats(stats, "Memory Import (Replace Month)");
  ThrowIfImportTaskFailed(stats, "Memory import (replace month) failed.");
}

auto PipelineWorkflow::RunIngest(const std::string& source_path,
                                 DateCheckMode date_check_mode,
                                 bool save_processed, IngestMode ingest_mode)
    -> void {
  runtime_bridge::LogInfo("\n--- 启动数据摄入 (Ingest) ---");
  modports::ClearBufferedDiagnostics();

  const auto db_check = database_health_checker_->CheckReady();
  if (!db_check.ok) {
    throw std::runtime_error(db_check.message.empty()
                                 ? "Database readiness check failed."
                                 : db_check.message);
  }

  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_, processed_data_storage_,
                                validation_issue_reporter_);
  const AppOptions full_options =
      BuildIngestOptions(source_path, date_check_mode, save_processed);
  auto result_context_opt = pipeline.Run(full_options);

  if (!result_context_opt) {
    runtime_bridge::LogError("\n=== Ingest 执行失败 ===");
    throw std::runtime_error(
        BuildPipelineFailureMessage("Ingestion process failed."));
  }

  auto& context = *result_context_opt;
  runtime_bridge::LogInfo("\n--- 流水线验证通过，准备入库 ---");

  if (ingest_mode == IngestMode::kSingleTxtReplaceMonth) {
    const auto target_month = TryResolveSingleTxtTargetMonth(context);
    if (!target_month.has_value()) {
      throw std::runtime_error(
          "Single TXT replace-month ingest requires exactly one TXT input "
          "with valid headers: yYYYY + mMM.");
    }

    if (!IsSingleMonthConsistent(context.result.processed_data,
                                 target_month->month_key)) {
      throw std::runtime_error(
          "Single TXT replace-month ingest failed: parsed days are not "
          "consistent with header month " +
          target_month->month_key + ".");
    }

    const auto previous_tail = ResolvePreviousTailForReplaceMonth(
        context, *target_month, context.result.processed_data,
        *time_sheet_repository_);
    if (previous_tail.has_value()) {
      LogLinker linker(context.state.converter_config);
      linker.LinkFirstDayWithExternalPreviousEvent(
          context.result.processed_data,
          LogLinker::ExternalPreviousEvent{
              .date = previous_tail->date,
              .end_time = previous_tail->end_time,
          });
    } else if (!context.result.processed_data.empty()) {
      runtime_bridge::LogWarn(
          "[LogLinker] No previous-month tail context found (DB/sibling "
          "TXT). Ingest will proceed without cross-month backfill.");
    }

    RunDatabaseImportFromMemoryReplacingMonth(
        context.result.processed_data, target_month->year, target_month->month);
    PersistSingleIngestSyncEntry(context, *time_sheet_repository_);
    runtime_bridge::LogInfo("\n=== Ingest 执行成功（单月替换）===");
    return;
  }

  if (!context.result.processed_data.empty()) {
    RunDatabaseImportFromMemory(context.result.processed_data);
    PersistIngestSyncSnapshot(context, *time_sheet_repository_);
    runtime_bridge::LogInfo("\n=== Ingest 执行成功 ===");
  } else {
    PersistIngestSyncSnapshot(context, *time_sheet_repository_);
    runtime_bridge::LogWarn("\n=== Ingest 完成但无数据产生 ===");
  }
}

auto PipelineWorkflow::RunIngestSyncStatusQuery(
    const IngestSyncStatusRequest& request) -> IngestSyncStatusOutput {
  return time_sheet_repository_->ListIngestSyncStatuses(request);
}

auto PipelineWorkflow::ClearIngestSyncStatus() -> void {
  time_sheet_repository_->ClearIngestSyncStatus();
}

auto PipelineWorkflow::RunIngestReplacingAll(const std::string& source_path,
                                             DateCheckMode date_check_mode,
                                             bool save_processed) -> void {
  runtime_bridge::LogInfo("\n--- 启动数据摄入 (Replace All) ---");
  modports::ClearBufferedDiagnostics();

  const auto db_check = database_health_checker_->CheckReady();
  if (!db_check.ok) {
    throw std::runtime_error(db_check.message.empty()
                                 ? "Database readiness check failed."
                                 : db_check.message);
  }

  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_, processed_data_storage_,
                                validation_issue_reporter_);
  const AppOptions kFullOptions =
      BuildIngestOptions(source_path, date_check_mode, save_processed);
  auto result_context_opt = pipeline.Run(kFullOptions);

  if (!result_context_opt) {
    runtime_bridge::LogError("\n=== Replace-all ingest 执行失败 ===");
    throw std::runtime_error(
        BuildPipelineFailureMessage("Replace-all ingestion process failed."));
  }

  auto& context = *result_context_opt;
  runtime_bridge::LogInfo("\n--- 流水线验证通过，准备全量替换入库 ---");
  RunDatabaseImportFromMemoryReplacingAll(context.result.processed_data);
  PersistIngestSyncSnapshot(context, *time_sheet_repository_);
  runtime_bridge::LogInfo("\n=== Ingest 执行成功（全量替换）===");
}

}  // namespace tracer::core::application::pipeline

