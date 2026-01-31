// importer/import_service.cpp
#include "importer/import_service.hpp"

#include <chrono>
#include <utility>

#include "importer/parser/memory_parser.hpp"
#include "importer/storage/repository.hpp"

ImportService::ImportService(std::string db_path)
    : db_path_(std::move(db_path)) {}

auto ImportService::import_from_memory(
    const std::map<std::string, std::vector<DailyLog>>& data_map)
    -> ImportStats {
  ImportStats stats;
  for (const auto& p : data_map) {
    stats.total_files += p.second.size();
  }
  stats.successful_files = stats.total_files;

  if (data_map.empty()) {
    return stats;
  }

  auto start = std::chrono::high_resolution_clock::now();

  // 1. 转换
  MemoryParser parser;
  ParsedData all_data = parser.parse(data_map);

  auto end_parsing = std::chrono::high_resolution_clock::now();
  stats.parsing_duration_s =
      std::chrono::duration<double>(end_parsing - start).count();

  // 2. 入库
  Repository inserter(db_path_);
  if (inserter.is_db_open()) {
    stats.db_open_success = true;
    try {
      inserter.import_data(all_data.days, all_data.records);
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