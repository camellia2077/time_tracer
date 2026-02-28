// application/importer/model/import_models.hpp
#ifndef APPLICATION_IMPORTER_MODEL_IMPORT_MODELS_H_
#define APPLICATION_IMPORTER_MODEL_IMPORT_MODELS_H_

#include <cstddef>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "domain/model/time_data_models.hpp"

struct DayData {
  std::string date;
  std::string remark;
  std::optional<std::string> getup_time;

  int year;
  int month;
  int status;
  int sleep;
  int exercise;

  ActivityStats stats;
};

struct TimeRecordInternal {
  long long logical_id = 0;
  long long start_timestamp = 0;
  long long end_timestamp = 0;

  std::string start_time_str;
  std::string end_time_str;
  std::string project_path;
  int duration_seconds = 0;
  std::optional<std::string> remark;

  std::string date;
};

struct PairHash {
  template <class T1, class T2>
  auto operator()(const std::pair<T1, T2>& pair) const -> std::size_t {
    auto hash1 = std::hash<T1>{}(pair.first);
    auto hash2 = std::hash<T2>{}(pair.second);
    return hash1 ^ (hash2 << 1);
  }
};

struct ParsedData {
  std::vector<DayData> days;
  std::vector<TimeRecordInternal> records;
};

struct ReplaceMonthTarget {
  int year = 0;
  int month = 0;
};

struct ImportStats {
  size_t total_files = 0;
  size_t successful_files = 0;
  std::vector<std::string> failed_files;

  size_t total_days = 0;
  size_t successful_days = 0;
  size_t skipped_days = 0;

  size_t total_records = 0;
  size_t successful_records = 0;
  size_t skipped_records = 0;

  std::map<std::string, size_t> reason_buckets;

  double parsing_duration_s = 0.0;
  double db_insertion_duration_s = 0.0;

  bool db_open_success = false;
  bool transaction_success = false;
  std::optional<std::string> replaced_month;
  std::string error_message;
};

#endif  // APPLICATION_IMPORTER_MODEL_IMPORT_MODELS_H_
