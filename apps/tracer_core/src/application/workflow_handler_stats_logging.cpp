// application/workflow_handler_stats_logging.cpp
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "application/importer/import_service.hpp"
#include "application/ports/logger.hpp"
#include "application/workflow_handler.hpp"
#include "shared/types/ansi_colors.hpp"

namespace app_ports = tracer_core::application::ports;

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

  namespace colors = tracer_core::common::colors;
  if (!stats.db_open_success) {
    app_ports::LogError(
        std::string(colors::kRed) + "[Fatal] DB Error: " +
        (stats.error_message.empty() ? "Unknown" : stats.error_message) +
        colors::kReset.data());
    return;
  }

  if (!stats.transaction_success) {
    app_ports::LogError(std::string(colors::kRed) +
                        "[Fatal] Transaction Failed: " + stats.error_message +
                        colors::kReset.data());
    return;
  }

  const bool has_skipped =
      (stats.skipped_days > 0U) || (stats.skipped_records > 0U);
  const bool has_legacy_failed = !stats.failed_files.empty();

  if (!has_legacy_failed && !has_skipped) {
    app_ports::LogInfo(std::string(colors::kGreen) + "[Success] Processed " +
                       std::to_string(stats.successful_files) + " items." +
                       colors::kReset.data());
  } else {
    std::ostringstream summary;
    summary << std::string(colors::kYellow)
            << "[Partial] Days=" << stats.successful_days << "/"
            << stats.total_days << ", Records=" << stats.successful_records
            << "/" << stats.total_records;
    if (has_legacy_failed) {
      summary << ", LegacyFailedFiles=" << stats.failed_files.size();
    }
    summary << colors::kReset.data();
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

  const double total_time =
      stats.parsing_duration_s + stats.db_insertion_duration_s;

  std::ostringstream timing;
  timing << std::fixed << std::setprecision(3)
         << "Timing: Parse=" << stats.parsing_duration_s
         << "s, Insert=" << stats.db_insertion_duration_s
         << "s, Total=" << total_time << "s";
  app_ports::LogInfo(timing.str());

  if (stats.replaced_month.has_value()) {
    app_ports::LogInfo("Replace scope: month=" + *stats.replaced_month);
  }
}

}  // namespace

void WorkflowHandler::RunDatabaseImport(const std::string& processed_path_str) {
  app_ports::LogInfo("正在解析 JSON 数据...");

  auto load_result = processed_data_loader_->LoadDailyLogs(processed_path_str);

  namespace colors = tracer_core::common::colors;
  for (const auto& error : load_result.errors) {
    app_ports::LogError(std::string(colors::kRed) + "解析文件失败 " +
                        error.source + ": " + error.message +
                        colors::kReset.data());
  }

  if (load_result.data_by_source.empty()) {
    app_ports::LogWarn(std::string(colors::kYellow) +
                       "没有有效的 JSON 数据可供导入。" +
                       colors::kReset.data());
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
      data_map, ReplaceMonthTarget{.year = year, .month = month});

  PrintImportStats(stats, "Memory Import (Replace Month)");
  workflow_handler_internal::ThrowIfImportTaskFailed(
      stats, "Memory import (replace month) failed.");
}
