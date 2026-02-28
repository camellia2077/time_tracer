// infrastructure/schema/day_schema.hpp
#ifndef INFRASTRUCTURE_SCHEMA_DAY_SCHEMA_H_
#define INFRASTRUCTURE_SCHEMA_DAY_SCHEMA_H_

#include <algorithm>
#include <array>
#include <span>
#include <string_view>

namespace schema::day {

namespace json {
inline constexpr std::string_view kHeaders = "headers";
inline constexpr std::string_view kActivities = "activities";
inline constexpr std::string_view kGeneratedStats = "generated_stats";

inline constexpr std::string_view kDate = "date";
inline constexpr std::string_view kStatus = "status";
inline constexpr std::string_view kExercise = "exercise";
inline constexpr std::string_view kSleep = "sleep";
inline constexpr std::string_view kCardio = "cardio";
inline constexpr std::string_view kAnaerobic = "anaerobic";
inline constexpr std::string_view kGetup = "getup";
inline constexpr std::string_view kActivityCount = "activity_count";
inline constexpr std::string_view kRemark = "remark";

inline constexpr std::string_view kLogicalId = "logical_id";
inline constexpr std::string_view kStartTimestamp = "start_timestamp";
inline constexpr std::string_view kEndTimestamp = "end_timestamp";
inline constexpr std::string_view kStartTime = "start_time";
inline constexpr std::string_view kEndTime = "end_time";
inline constexpr std::string_view kDurationSeconds = "duration_seconds";
inline constexpr std::string_view kActivityRemark = "activity_remark";
inline constexpr std::string_view kActivity = "activity";
inline constexpr std::string_view kProjectPath = "project_path";

inline constexpr std::string_view kSleepNightTime = "sleep_night_time";
inline constexpr std::string_view kSleepDayTime = "sleep_day_time";
inline constexpr std::string_view kSleepTotalTime = "sleep_total_time";
inline constexpr std::string_view kTotalExerciseTime = "total_exercise_time";
inline constexpr std::string_view kCardioTime = "cardio_time";
inline constexpr std::string_view kAnaerobicTime = "anaerobic_time";
inline constexpr std::string_view kGroomingTime = "grooming_time";
inline constexpr std::string_view kToiletTime = "toilet_time";
inline constexpr std::string_view kGamingTime = "gaming_time";
inline constexpr std::string_view kRecreationTime = "recreation_time";
inline constexpr std::string_view kRecreationZhihuTime =
    "recreation_zhihu_time";
inline constexpr std::string_view kRecreationBilibiliTime =
    "recreation_bilibili_time";
inline constexpr std::string_view kRecreationDouyinTime =
    "recreation_douyin_time";
inline constexpr std::string_view kTotalStudyTime = "total_study_time";
}  // namespace json

namespace db {
inline constexpr std::string_view kTable = "days";
inline constexpr std::string_view kIndexYearMonth = "idx_year_month";
inline constexpr std::string_view kDate = "date";
inline constexpr std::string_view kYear = "year";
inline constexpr std::string_view kMonth = "month";
inline constexpr std::string_view kStatus = "status";
inline constexpr std::string_view kSleep = "sleep";
inline constexpr std::string_view kRemark = "remark";
inline constexpr std::string_view kGetupTime = "getup_time";
inline constexpr std::string_view kExercise = "exercise";
inline constexpr std::string_view kTotalExerciseTime = "total_exercise_time";
inline constexpr std::string_view kCardioTime = "cardio_time";
inline constexpr std::string_view kAnaerobicTime = "anaerobic_time";
inline constexpr std::string_view kGamingTime = "gaming_time";
inline constexpr std::string_view kGroomingTime = "grooming_time";
inline constexpr std::string_view kToiletTime = "toilet_time";
inline constexpr std::string_view kStudyTime = "study_time";
inline constexpr std::string_view kSleepNightTime = "sleep_night_time";
inline constexpr std::string_view kSleepDayTime = "sleep_day_time";
inline constexpr std::string_view kSleepTotalTime = "sleep_total_time";
inline constexpr std::string_view kRecreationTime = "recreation_time";
inline constexpr std::string_view kRecreationZhihuTime =
    "recreation_zhihu_time";
inline constexpr std::string_view kRecreationBilibiliTime =
    "recreation_bilibili_time";
inline constexpr std::string_view kRecreationDouyinTime =
    "recreation_douyin_time";
}  // namespace db

struct FieldMapping {
  std::string_view json_key_;
  std::string_view db_column_;
};

struct DbColumnSet {
  std::span<const std::string_view> values;
};

struct JsonOnlyKeySet {
  std::span<const std::string_view> values;
};

inline constexpr std::array<std::string_view, 22> kDaysTableColumns = {
    db::kDate,
    db::kYear,
    db::kMonth,
    db::kStatus,
    db::kSleep,
    db::kRemark,
    db::kGetupTime,
    db::kExercise,
    db::kTotalExerciseTime,
    db::kCardioTime,
    db::kAnaerobicTime,
    db::kGamingTime,
    db::kGroomingTime,
    db::kToiletTime,
    db::kStudyTime,
    db::kSleepNightTime,
    db::kSleepDayTime,
    db::kSleepTotalTime,
    db::kRecreationTime,
    db::kRecreationZhihuTime,
    db::kRecreationBilibiliTime,
    db::kRecreationDouyinTime,
};

inline constexpr std::array<std::string_view, 9> kHeaderJsonKeys = {
    json::kDate,  json::kStatus,        json::kExercise,
    json::kSleep, json::kCardio,        json::kAnaerobic,
    json::kGetup, json::kActivityCount, json::kRemark};

inline constexpr std::array<std::string_view, 14> kGeneratedStatsJsonKeys = {
    json::kSleepNightTime,       json::kSleepDayTime,
    json::kSleepTotalTime,       json::kTotalExerciseTime,
    json::kCardioTime,           json::kAnaerobicTime,
    json::kGroomingTime,         json::kToiletTime,
    json::kGamingTime,           json::kRecreationTime,
    json::kRecreationZhihuTime,  json::kRecreationBilibiliTime,
    json::kRecreationDouyinTime, json::kTotalStudyTime};

inline constexpr std::array<std::string_view, 1> kHeaderJsonOnlyKeys = {
    json::kActivityCount};

inline constexpr std::array<FieldMapping, 9> kHeaderFieldMappings = {{
    {.json_key_ = json::kDate, .db_column_ = db::kDate},
    {.json_key_ = json::kStatus, .db_column_ = db::kStatus},
    {.json_key_ = json::kExercise, .db_column_ = db::kExercise},
    {.json_key_ = json::kSleep, .db_column_ = db::kSleep},
    {.json_key_ = json::kCardio, .db_column_ = db::kCardioTime},
    {.json_key_ = json::kAnaerobic, .db_column_ = db::kAnaerobicTime},
    {.json_key_ = json::kGetup, .db_column_ = db::kGetupTime},
    {.json_key_ = json::kRemark, .db_column_ = db::kRemark},
    {.json_key_ = json::kActivityCount, .db_column_ = ""},
}};

inline constexpr std::array<FieldMapping, 14> kGeneratedStatsMappings = {{
    {.json_key_ = json::kSleepNightTime, .db_column_ = db::kSleepNightTime},
    {.json_key_ = json::kSleepDayTime, .db_column_ = db::kSleepDayTime},
    {.json_key_ = json::kSleepTotalTime, .db_column_ = db::kSleepTotalTime},
    {.json_key_ = json::kTotalExerciseTime,
     .db_column_ = db::kTotalExerciseTime},
    {.json_key_ = json::kCardioTime, .db_column_ = db::kCardioTime},
    {.json_key_ = json::kAnaerobicTime, .db_column_ = db::kAnaerobicTime},
    {.json_key_ = json::kGroomingTime, .db_column_ = db::kGroomingTime},
    {.json_key_ = json::kToiletTime, .db_column_ = db::kToiletTime},
    {.json_key_ = json::kGamingTime, .db_column_ = db::kGamingTime},
    {.json_key_ = json::kRecreationTime, .db_column_ = db::kRecreationTime},
    {.json_key_ = json::kRecreationZhihuTime,
     .db_column_ = db::kRecreationZhihuTime},
    {.json_key_ = json::kRecreationBilibiliTime,
     .db_column_ = db::kRecreationBilibiliTime},
    {.json_key_ = json::kRecreationDouyinTime,
     .db_column_ = db::kRecreationDouyinTime},
    {.json_key_ = json::kTotalStudyTime, .db_column_ = db::kStudyTime},
}};

constexpr auto ContainsValue(std::span<const std::string_view> items,
                             std::string_view value) -> bool {
  return std::ranges::any_of(
      items, [value](std::string_view item) -> bool { return item == value; });
}

constexpr auto HasDuplicateKeys(std::span<const std::string_view> items)
    -> bool {
  for (size_t idx = 0; idx < items.size(); ++idx) {
    for (size_t jdx = idx + 1; jdx < items.size(); ++jdx) {
      if (items[idx] == items[jdx]) {
        return true;
      }
    }
  }
  return false;
}

constexpr auto HasDuplicateMappingJsonKeys(
    std::span<const FieldMapping> mappings) -> bool {
  for (size_t idx = 0; idx < mappings.size(); ++idx) {
    for (size_t jdx = idx + 1; jdx < mappings.size(); ++jdx) {
      if (mappings[idx].json_key_ == mappings[jdx].json_key_) {
        return true;
      }
    }
  }
  return false;
}

constexpr auto HasDuplicateMappingDbColumns(
    std::span<const FieldMapping> mappings) -> bool {
  for (size_t idx = 0; idx < mappings.size(); ++idx) {
    for (size_t jdx = idx + 1; jdx < mappings.size(); ++jdx) {
      if (!mappings[idx].db_column_.empty() &&
          mappings[idx].db_column_ == mappings[jdx].db_column_) {
        return true;
      }
    }
  }
  return false;
}

constexpr auto AllKeysCovered(std::span<const std::string_view> keys,
                              std::span<const FieldMapping> mappings) -> bool {
  for (const auto kKey : keys) {
    bool found = false;
    for (const auto& mapping : mappings) {
      if (mapping.json_key_ == kKey) {
        found = true;
        break;
      }
    }
    if (!found) {
      return false;
    }
  }
  return true;
}

constexpr auto AllMappingsValid(std::span<const FieldMapping> mappings,
                                DbColumnSet db_columns,
                                JsonOnlyKeySet json_only_keys) -> bool {
  return std::ranges::all_of(
      mappings, [&](const FieldMapping& mapping) -> bool {
        if (mapping.json_key_.empty()) {
          return false;
        }
        if (mapping.db_column_.empty()) {
          return ContainsValue(json_only_keys.values, mapping.json_key_);
        }
        return ContainsValue(db_columns.values, mapping.db_column_);
      });
}

static_assert(!HasDuplicateKeys(kHeaderJsonKeys),
              "Duplicate header JSON keys detected.");
static_assert(!HasDuplicateKeys(kGeneratedStatsJsonKeys),
              "Duplicate stats JSON keys detected.");
static_assert(!HasDuplicateMappingJsonKeys(kHeaderFieldMappings),
              "Duplicate header mapping JSON keys detected.");
static_assert(!HasDuplicateMappingDbColumns(kHeaderFieldMappings),
              "Duplicate header mapping DB columns detected.");
static_assert(!HasDuplicateMappingJsonKeys(kGeneratedStatsMappings),
              "Duplicate stats mapping JSON keys detected.");
static_assert(!HasDuplicateMappingDbColumns(kGeneratedStatsMappings),
              "Duplicate stats mapping DB columns detected.");
static_assert(AllKeysCovered(kHeaderJsonKeys, kHeaderFieldMappings),
              "Header JSON keys missing in header mappings.");
static_assert(AllKeysCovered(kGeneratedStatsJsonKeys, kGeneratedStatsMappings),
              "Stats JSON keys missing in stats mappings.");
static_assert(AllMappingsValid(kHeaderFieldMappings,
                               DbColumnSet{.values = kDaysTableColumns},
                               JsonOnlyKeySet{.values = kHeaderJsonOnlyKeys}),
              "Invalid header JSON->DB mappings detected.");
static_assert(
    AllMappingsValid(kGeneratedStatsMappings,
                     DbColumnSet{.values = kDaysTableColumns},
                     JsonOnlyKeySet{
                         .values = std::span<const std::string_view>{}}),
    "Invalid stats JSON->DB mappings detected.");

}  // namespace schema::day

#endif  // INFRASTRUCTURE_SCHEMA_DAY_SCHEMA_H_
