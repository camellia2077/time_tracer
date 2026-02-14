// application/importer/model/import_models.hpp
#ifndef APPLICATION_IMPORTER_MODEL_IMPORT_MODELS_H_
#define APPLICATION_IMPORTER_MODEL_IMPORT_MODELS_H_

#include <cstddef>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "domain/model/time_data_models.hpp"

struct DayData {
  std::string date;
  std::string remark;
  std::string getup_time;

  int year;
  int month;
  int status;
  int sleep;
  int exercise;

  ActivityStats stats;
};

struct TimeRecordInternal : public BaseActivityRecord {
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

struct ImportStats {
  size_t total_files = 0;
  size_t successful_files = 0;
  std::vector<std::string> failed_files;

  double parsing_duration_s = 0.0;
  double db_insertion_duration_s = 0.0;

  bool db_open_success = false;
  bool transaction_success = false;
  std::string error_message;
};

#endif  // APPLICATION_IMPORTER_MODEL_IMPORT_MODELS_H_
