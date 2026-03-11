module;

#include <filesystem>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "application/ports/i_processed_data_loader.hpp"
#include "application/ports/logger.hpp"
#include "domain/types/date_check_mode.hpp"
#include "domain/types/ingest_mode.hpp"

namespace fs = std::filesystem;

struct AppOptions;
struct DailyLog;

namespace tracer_core::application::ports {
class IConverterConfigProvider;
class IDatabaseHealthChecker;
class IIngestInputProvider;
class IProcessedDataLoader;
class IProcessedDataStorage;
class ITimeSheetRepository;
class IValidationIssueReporter;
}  // namespace tracer_core::application::ports

module tracer.core.application.workflow.handler;

import tracer.core.application.workflow.interface;
import tracer.core.application.importer.service;
import tracer.core.domain.model.daily_log;
import tracer.core.shared.ansi_colors;

namespace app_ports = tracer_core::application::ports;
namespace modcolors = tracer::core::shared::ansi_colors;
using tracer::core::application::modimporter::ImportService;
using tracer::core::application::modimporter::ImportStats;
using tracer::core::application::modimporter::ReplaceMonthTarget;
using tracer::core::domain::model::DailyLog;

namespace workflow_handler_internal {
auto ThrowIfImportTaskFailed(const ImportStats& stats,
                             std::string_view default_message) -> void;
}  // namespace workflow_handler_internal

namespace {

auto PrintImportStats(const ImportStats& stats, std::string_view title)
    -> void {
  std::ostringstream stream;
  stream << "\n--- " << title << " Report ---";
  app_ports::LogInfo(stream.str());

  if (!stats.db_open_success) {
    app_ports::LogError(
        std::string(modcolors::kRed) + "[Fatal] DB Error: " +
        (stats.error_message.empty() ? "Unknown" : stats.error_message) +
        modcolors::kReset.data());
    return;
  }

  if (!stats.transaction_success) {
    app_ports::LogError(std::string(modcolors::kRed) +
                        "[Fatal] Transaction Failed: " + stats.error_message +
                        modcolors::kReset.data());
    return;
  }

  const bool kHasSkipped =
      (stats.skipped_days > 0U) || (stats.skipped_records > 0U);
  const bool kHasLegacyFailed = !stats.failed_files.empty();

  if (!kHasLegacyFailed && !kHasSkipped) {
    app_ports::LogInfo(std::string(modcolors::kGreen) +
                       "[Success] Processed " +
                       std::to_string(stats.successful_files) + " items." +
                       modcolors::kReset.data());
  } else {
    std::ostringstream summary;
    summary << std::string(modcolors::kYellow)
            << "[Partial] Days=" << stats.successful_days << "/"
            << stats.total_days << ", Records=" << stats.successful_records
            << "/" << stats.total_records;
    if (kHasLegacyFailed) {
      summary << ", LegacyFailedFiles=" << stats.failed_files.size();
    }
    summary << modcolors::kReset.data();
    app_ports::LogWarn(summary.str());

    for (const auto& failed_file : stats.failed_files) {
      app_ports::LogError("  Failed: " + failed_file);
    }
  }

  {
    std::ostringstream day_record_breakdown;
    day_record_breakdown << "Breakdown: days(total=" << stats.total_days
                         << ", success=" << stats.successful_days
                         << ", skipped=" << stats.skipped_days
                         << "), records(total=" << stats.total_records
                         << ", success=" << stats.successful_records
                         << ", skipped=" << stats.skipped_records << ")";
    app_ports::LogInfo(day_record_breakdown.str());
  }

  if (!stats.reason_buckets.empty()) {
    app_ports::LogInfo("Reason Buckets:");
    for (const auto& [reason, count] : stats.reason_buckets) {
      app_ports::LogInfo("  - " + reason + ": " + std::to_string(count));
    }
  }

  const double kTotalTime =
      stats.parsing_duration_s + stats.db_insertion_duration_s;

  std::ostringstream timing;
  timing << std::fixed << std::setprecision(3)
         << "Timing: Parse=" << stats.parsing_duration_s
         << "s, Insert=" << stats.db_insertion_duration_s
         << "s, Total=" << kTotalTime << "s";
  app_ports::LogInfo(timing.str());

  if (stats.replaced_month.has_value()) {
    app_ports::LogInfo("Replace scope: month=" + *stats.replaced_month);
  }
}

}  // namespace

namespace tracer::core::application::workflow {

void WorkflowHandler::RunDatabaseImport(const std::string& processed_path_str) {
  app_ports::LogInfo("正在解析 JSON 数据...");

  auto load_result = processed_data_loader_->LoadDailyLogs(processed_path_str);

  for (const auto& error : load_result.errors) {
    app_ports::LogError(std::string(modcolors::kRed) + "解析文件失败 " +
                        error.source + ": " + error.message +
                        modcolors::kReset.data());
  }

  if (load_result.data_by_source.empty()) {
    app_ports::LogWarn(std::string(modcolors::kYellow) +
                       "没有有效的 JSON 数据可供导入。" +
                       modcolors::kReset.data());
    return;
  }

  RunDatabaseImportFromMemory(load_result.data_by_source);
}

auto WorkflowHandler::RunDatabaseImportFromMemory(
    const std::map<std::string, std::vector<DailyLog>>& data_map) -> void {
  app_ports::LogInfo("Task: Memory Import...");

  ImportService service(*time_sheet_repository_);
  ImportStats stats = service.ImportFromMemory(data_map);

  PrintImportStats(stats, "Memory Import");
  workflow_handler_internal::ThrowIfImportTaskFailed(stats,
                                                     "Memory import failed.");
}

auto WorkflowHandler::RunDatabaseImportFromMemoryReplacingMonth(
    const std::map<std::string, std::vector<DailyLog>>& data_map, int year,
    int month) -> void {
  app_ports::LogInfo("Task: Memory Import (Replace Month)...");

  ImportService service(*time_sheet_repository_);
  ImportStats stats = service.ImportFromMemory(
      data_map, ReplaceMonthTarget{.kYear = year, .kMonth = month});

  PrintImportStats(stats, "Memory Import (Replace Month)");
  workflow_handler_internal::ThrowIfImportTaskFailed(
      stats, "Memory import (replace month) failed.");
}

}  // namespace tracer::core::application::workflow
