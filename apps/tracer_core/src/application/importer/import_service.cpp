// application/importer/import_service.cpp
#include "application/importer/import_service.hpp"

#include <chrono>
#include <exception>
#include <format>

#include "application/parser/memory_parser.hpp"

ImportService::ImportService(
    tracer_core::application::ports::ITimeSheetRepository& repository)
    : repository_(repository) {}

auto ImportService::ImportFromMemory(
    const std::map<std::string, std::vector<DailyLog>>& data_map,
    const std::optional<ReplaceMonthTarget>& replace_month_target)
    -> ImportStats {
  ImportStats stats;
  for (const auto& [source_key, days] : data_map) {
    static_cast<void>(source_key);
    for (const auto& day : days) {
      ++stats.total_days;
      stats.total_records += day.processedActivities.size();
    }
  }

  stats.total_files = stats.total_days;
  stats.successful_files = stats.total_files;

  if (data_map.empty() && !replace_month_target.has_value()) {
    return stats;
  }

  ParsedData all_data;
  const auto kParsingStart = std::chrono::high_resolution_clock::now();
  auto parsing_end = kParsingStart;

  if (!data_map.empty()) {
    // 1. 转换 (MemoryParser::Parse is a static method)
    all_data = MemoryParser::Parse(data_map);
    stats.successful_days = all_data.days.size();
    stats.successful_records = all_data.records.size();

    if (stats.total_days > stats.successful_days) {
      stats.skipped_days = stats.total_days - stats.successful_days;
      stats.reason_buckets["day_filtered_in_parse"] = stats.skipped_days;
    }
    if (stats.total_records > stats.successful_records) {
      stats.skipped_records = stats.total_records - stats.successful_records;
      stats.reason_buckets["record_filtered_in_parse"] = stats.skipped_records;
    }

    // Compatibility counters (historically file-based)
    stats.successful_files = stats.successful_days;

    parsing_end = std::chrono::high_resolution_clock::now();
    stats.parsing_duration_s =
        std::chrono::duration<double>(parsing_end - kParsingStart).count();
  }

  // 2. 入库
  if (repository_.IsDbOpen()) {
    stats.db_open_success = true;
    try {
      if (replace_month_target.has_value()) {
        repository_.ReplaceMonthData(replace_month_target->year,
                                     replace_month_target->month, all_data.days,
                                     all_data.records);
        stats.replaced_month =
            std::format("{:04d}-{:02d}", replace_month_target->year,
                        replace_month_target->month);
      } else {
        repository_.ImportData(all_data.days, all_data.records);
      }
      stats.transaction_success = true;
    } catch (const std::exception& e) {
      stats.transaction_success = false;
      stats.error_message = e.what();
      stats.reason_buckets["db_transaction_failed"] += 1;
    }
  } else {
    stats.db_open_success = false;
    stats.error_message = "Cannot open database.";
    stats.reason_buckets["db_not_open"] += 1;
  }

  auto end_total = std::chrono::high_resolution_clock::now();
  stats.db_insertion_duration_s =
      std::chrono::duration<double>(end_total - parsing_end).count();

  return stats;
}
