namespace {

auto PrintImportStats(const ImportStats& stats, std::string_view title)
    -> void {
  std::ostringstream stream;
  stream << "\n--- " << title << " Report ---";
  runtime_bridge::LogInfo(stream.str());

  if (!stats.db_open_success) {
    runtime_bridge::LogError(
        "[Fatal] DB Error: " +
        (stats.error_message.empty() ? "Unknown" : stats.error_message));
    return;
  }

  if (!stats.transaction_success) {
    runtime_bridge::LogError("[Fatal] Transaction Failed: " + stats.error_message);
    return;
  }

  const bool kHasSkipped =
      (stats.skipped_days > 0U) || (stats.skipped_records > 0U);
  const bool kHasLegacyFailed = !stats.failed_files.empty();

  if (!kHasLegacyFailed && !kHasSkipped) {
    runtime_bridge::LogInfo("[Success] Processed " +
                       std::to_string(stats.successful_files) + " items.");
  } else {
    std::ostringstream summary;
    summary << "[Partial] Days=" << stats.successful_days << "/"
            << stats.total_days << ", Records=" << stats.successful_records
            << "/" << stats.total_records;
    if (kHasLegacyFailed) {
      summary << ", LegacyFailedFiles=" << stats.failed_files.size();
    }
    runtime_bridge::LogWarn(summary.str());

    for (const auto& failed_file : stats.failed_files) {
      runtime_bridge::LogError("  Failed: " + failed_file);
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
    runtime_bridge::LogInfo(day_record_breakdown.str());
  }

  if (!stats.reason_buckets.empty()) {
    runtime_bridge::LogInfo("Reason Buckets:");
    for (const auto& [reason, count] : stats.reason_buckets) {
      runtime_bridge::LogInfo("  - " + reason + ": " + std::to_string(count));
    }
  }

  const double kTotalTime =
      stats.parsing_duration_s + stats.db_insertion_duration_s;

  std::ostringstream timing;
  timing << std::fixed << std::setprecision(3)
         << "Timing: Parse=" << stats.parsing_duration_s
         << "s, Insert=" << stats.db_insertion_duration_s
         << "s, Total=" << kTotalTime << "s";
  runtime_bridge::LogInfo(timing.str());

  if (stats.replaced_month.has_value()) {
    runtime_bridge::LogInfo("Replace scope: month=" + *stats.replaced_month);
  }
}

}  // namespace

namespace tracer::core::application::workflow {

void WorkflowHandler::RunDatabaseImport(const std::string& processed_path_str) {
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

auto WorkflowHandler::RunDatabaseImportFromMemory(
    const std::map<std::string, std::vector<DailyLog>>& data_map) -> void {
  runtime_bridge::LogInfo("Task: Memory Import...");

  ImportService service(*time_sheet_repository_);
  ImportStats stats = service.ImportFromMemory(data_map);

  PrintImportStats(stats, "Memory Import");
  workflow_handler_internal::ThrowIfImportTaskFailed(stats,
                                                     "Memory import failed.");
}

auto WorkflowHandler::RunDatabaseImportFromMemoryReplacingAll(
    const std::map<std::string, std::vector<DailyLog>>& data_map) -> void {
  runtime_bridge::LogInfo("Task: Memory Import (Replace All)...");

  ImportService service(*time_sheet_repository_);
  ImportStats stats =
      service.ImportFromMemory(data_map, std::nullopt, ReplaceAllTarget{});

  PrintImportStats(stats, "Memory Import (replace all)");
  workflow_handler_internal::ThrowIfImportTaskFailed(
      stats, "Memory import (replace all) failed.");
}

auto WorkflowHandler::RunDatabaseImportFromMemoryReplacingMonth(
    const std::map<std::string, std::vector<DailyLog>>& data_map, int year,
    int month) -> void {
  runtime_bridge::LogInfo("Task: Memory Import (Replace Month)...");

  ImportService service(*time_sheet_repository_);
  ImportStats stats = service.ImportFromMemory(
      data_map, ReplaceMonthTarget{.kYear = year, .kMonth = month});

  PrintImportStats(stats, "Memory Import (Replace Month)");
  workflow_handler_internal::ThrowIfImportTaskFailed(
      stats, "Memory import (replace month) failed.");
}

}  // namespace tracer::core::application::workflow
