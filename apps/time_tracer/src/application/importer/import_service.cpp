// importer/import_service.cpp
#include "application/importer/import_service.hpp"

#include <chrono>
#include <utility>

#include "application/importer/storage/repository.hpp"
#include "application/parser/memory_parser.hpp"

ImportService::ImportService(std::string db_path)
    : db_path_(std::move(db_path)) {}

auto ImportService::ImportFromMemory(
    const std::map<std::string, std::vector<DailyLog>>& data_map)
    -> ImportStats {
  ImportStats stats;
  for (const auto& data_pair : data_map) {
    stats.total_files += data_pair.second.size();
  }
  stats.successful_files = stats.total_files;

  if (data_map.empty()) {
    return stats;
  }

  auto start = std::chrono::high_resolution_clock::now();

  // 1. 转换 (MemoryParser::Parse is a static method)
  ParsedData all_data = MemoryParser::Parse(data_map);

  auto end_parsing = std::chrono::high_resolution_clock::now();
  stats.parsing_duration_s =
      std::chrono::duration<double>(end_parsing - start).count();

  // 2. 入库
  Repository inserter(db_path_);
  if (inserter.IsDbOpen()) {
    stats.db_open_success = true;
    try {
      inserter.ImportData(all_data.days, all_data.records);
      stats.transaction_success = true;
    } catch (const std::exception& e) {
      stats.transaction_success = false;
      stats.error_message = e.what();
    }
  } else {
    stats.db_open_success = false;
    stats.error_message = "Cannot open database.";
  }

  auto end_total = std::chrono::high_resolution_clock::now();
  stats.db_insertion_duration_s =
      std::chrono::duration<double>(end_total - end_parsing).count();

  return stats;
}
