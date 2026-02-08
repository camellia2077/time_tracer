// infrastructure/schema/day_schema.hpp
#ifndef INFRASTRUCTURE_SCHEMA_DAY_SCHEMA_H_
#define INFRASTRUCTURE_SCHEMA_DAY_SCHEMA_H_

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
  std::string_view json_key;
  std::string_view db_column;
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
    json::kDate,
    json::kStatus,
    json::kExercise,
    json::kSleep,
    json::kCardio,
    json::kAnaerobic,
    json::kGetup,
    json::kActivityCount,
    json::kRemark};

inline constexpr std::array<std::string_view, 14> kGeneratedStatsJsonKeys = {
    json::kSleepNightTime,      json::kSleepDayTime,
    json::kSleepTotalTime,      json::kTotalExerciseTime,
    json::kCardioTime,          json::kAnaerobicTime,
    json::kGroomingTime,        json::kToiletTime,
    json::kGamingTime,          json::kRecreationTime,
    json::kRecreationZhihuTime, json::kRecreationBilibiliTime,
    json::kRecreationDouyinTime, json::kTotalStudyTime};

inline constexpr std::array<std::string_view, 1> kHeaderJsonOnlyKeys = {
    json::kActivityCount};

inline constexpr std::array<FieldMapping, 9> kHeaderFieldMappings = {{
    {json::kDate, db::kDate},
    {json::kStatus, db::kStatus},
    {json::kExercise, db::kExercise},
    {json::kSleep, db::kSleep},
    {json::kCardio, db::kCardioTime},
    {json::kAnaerobic, db::kAnaerobicTime},
    {json::kGetup, db::kGetupTime},
    {json::kRemark, db::kRemark},
    {json::kActivityCount, ""},
}};

inline constexpr std::array<FieldMapping, 14> kGeneratedStatsMappings = {{
    {json::kSleepNightTime, db::kSleepNightTime},
    {json::kSleepDayTime, db::kSleepDayTime},
    {json::kSleepTotalTime, db::kSleepTotalTime},
    {json::kTotalExerciseTime, db::kTotalExerciseTime},
    {json::kCardioTime, db::kCardioTime},
    {json::kAnaerobicTime, db::kAnaerobicTime},
    {json::kGroomingTime, db::kGroomingTime},
    {json::kToiletTime, db::kToiletTime},
    {json::kGamingTime, db::kGamingTime},
    {json::kRecreationTime, db::kRecreationTime},
    {json::kRecreationZhihuTime, db::kRecreationZhihuTime},
    {json::kRecreationBilibiliTime, db::kRecreationBilibiliTime},
    {json::kRecreationDouyinTime, db::kRecreationDouyinTime},
    {json::kTotalStudyTime, db::kStudyTime},
}};

constexpr auto ContainsValue(std::span<const std::string_view> items,
                             std::string_view value) -> bool {
  for (size_t idx = 0; idx < items.size(); ++idx) {
    if (items[idx] == value) {
      return true;
    }
  }
  return false;
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
      if (mappings[idx].json_key == mappings[jdx].json_key) {
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
      if (!mappings[idx].db_column.empty() &&
          mappings[idx].db_column == mappings[jdx].db_column) {
        return true;
      }
    }
  }
  return false;
}

constexpr auto AllKeysCovered(std::span<const std::string_view> keys,
                              std::span<const FieldMapping> mappings) -> bool {
  for (size_t idx = 0; idx < keys.size(); ++idx) {
    bool found = false;
    for (size_t jdx = 0; jdx < mappings.size(); ++jdx) {
      if (mappings[jdx].json_key == keys[idx]) {
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
                                std::span<const std::string_view> db_columns,
                                std::span<const std::string_view> json_only)
    -> bool {
  for (size_t idx = 0; idx < mappings.size(); ++idx) {
    if (mappings[idx].json_key.empty()) {
      return false;
    }
    if (mappings[idx].db_column.empty()) {
      if (!ContainsValue(json_only, mappings[idx].json_key)) {
        return false;
      }
      continue;
    }
    if (!ContainsValue(db_columns, mappings[idx].db_column)) {
      return false;
    }
  }
  return true;
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
static_assert(AllMappingsValid(kHeaderFieldMappings, kDaysTableColumns,
                               kHeaderJsonOnlyKeys),
              "Invalid header JSON->DB mappings detected.");
static_assert(AllMappingsValid(kGeneratedStatsMappings, kDaysTableColumns,
                               std::span<const std::string_view>{}),
              "Invalid stats JSON->DB mappings detected.");

}  // namespace schema::day

#endif  // INFRASTRUCTURE_SCHEMA_DAY_SCHEMA_H_
