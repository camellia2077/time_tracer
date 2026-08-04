#include "application/pipeline/detail/pipeline_ingest_sync_support.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <map>
#include <optional>
#include <ranges>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "application/ports/pipeline/i_time_sheet_write_repository.hpp"
#include "application/ports/pipeline/i_ingest_runtime_repository.hpp"
#include "application/pipeline/detail/pipeline_sha256.hpp"
#include "application/runtime_bridge/logger.hpp"
#include "application/activity_name_converter.hpp"
#include "application/dto/ingest_input_model.hpp"
#include "domain/logic/converter/convert/core/converter_core.hpp"
#include "domain/logic/converter/log_processor.hpp"
#include "domain/model/daily_log.hpp"
#include "shared/utils/canonical_text.hpp"
#include "shared/utils/string_utils.hpp"

import tracer.core.application.pipeline.types;
import tracer.core.application.pipeline.orchestrator;

namespace tracer::core::application::pipeline::detail {

namespace app_ports = tracer_core::application::ports;
namespace fs = std::filesystem;
namespace modtext = tracer::core::shared::canonical_text;
namespace pipeline_detail = tracer::core::application::pipeline::detail;
namespace runtime_bridge = tracer_core::application::runtime_bridge;
using tracer::core::shared::string_utils::Trim;
using tracer_core::application::dto::IngestInputModel;
using tracer_core::core::dto::IngestSyncStatusEntry;

namespace {

#include "application/pipeline/detail/pipeline_replace_month_support_impl.inc"

[[nodiscard]] auto BuildCanonicalMonthRelativePath(
    const SingleTxtTargetMonth& month) -> std::string {
  return std::format("{0:04d}/{0:04d}-{1:02d}.txt", month.year, month.month);
}

[[nodiscard]] auto TryBuildIngestSyncEntry(
    const IngestInputModel& input, const ConverterConfig& converter_config,
    const std::int64_t kIngestedAtMs) -> std::optional<IngestSyncStatusEntry> {
  const auto kCanonical = modtext::Canonicalize(
      input.content,
      input.source_label.empty() ? input.source_id : input.source_label);
  if (!kCanonical.ok) {
    runtime_bridge::LogWarn(
        "Skipping ingest sync snapshot due to invalid TXT: " +
        kCanonical.error_message);
    return std::nullopt;
  }

  const auto kTargetMonth =
      TryParseSingleTxtTargetMonthFromContent(kCanonical.text);
  if (!kTargetMonth.has_value()) {
    runtime_bridge::LogWarn(
        "Skipping ingest sync snapshot because TXT month header is missing: " +
        (input.source_label.empty() ? input.source_id : input.source_label));
    return std::nullopt;
  }

  const ActivityNameTextConverter kActivityNameConverter(converter_config);
  const std::string kSemanticText = kActivityNameConverter.ConvertText(
      kCanonical.text, ActivityNameMappingDirection::kAliasToCanonical);

  return IngestSyncStatusEntry{
      .month_key = kTargetMonth->month_key,
      .txt_relative_path = BuildCanonicalMonthRelativePath(*kTargetMonth),
      .txt_content_hash_sha256 =
          pipeline_detail::ComputeSha256Hex(kSemanticText),
      .ingested_at_unix_ms = kIngestedAtMs,
  };
}

[[nodiscard]] auto BuildIngestSyncSnapshot(const PipelineSession& context)
    -> std::vector<IngestSyncStatusEntry> {
  std::map<std::string, IngestSyncStatusEntry> unique_entries;
  std::set<std::string> duplicate_months;
  const std::int64_t kIngestedAtMs = pipeline_detail::CurrentUnixMillis();

  for (const auto& input : context.state.ingest_inputs) {
    const auto kEntry = TryBuildIngestSyncEntry(
        input, context.state.converter_config, kIngestedAtMs);
    if (!kEntry.has_value() || duplicate_months.contains(kEntry->month_key)) {
      continue;
    }

    const auto kInsertResult = unique_entries.emplace(kEntry->month_key, *kEntry);
    if (!kInsertResult.second) {
      duplicate_months.insert(kEntry->month_key);
      unique_entries.erase(kEntry->month_key);
      runtime_bridge::LogWarn(
          "Duplicate TXT month detected during ingest sync snapshot: " +
          kEntry->month_key + ". Sync row will be omitted for this month.");
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

}  // namespace

auto PersistIngestSyncSnapshot(const PipelineSession& context,
                               app_ports::ITimeSheetWriteRepository& repository)
    -> void {
  repository.ReplaceIngestSyncStatuses(BuildIngestSyncSnapshot(context));
}

auto PersistSingleIngestSyncEntry(
    const PipelineSession& context,
    app_ports::ITimeSheetWriteRepository& repository) -> void {
  if (context.state.ingest_inputs.size() != 1U) {
    throw std::runtime_error(
        "Single TXT ingest sync snapshot requires exactly one input.");
  }

  const auto kEntry = TryBuildIngestSyncEntry(
      context.state.ingest_inputs.front(), context.state.converter_config,
      pipeline_detail::CurrentUnixMillis());
  if (!kEntry.has_value()) {
    throw std::runtime_error(
        "Single TXT ingest sync snapshot requires valid yYYYY + mMM headers.");
  }
  repository.UpsertIngestSyncStatus(*kEntry);
}

}  // namespace tracer::core::application::pipeline::detail
