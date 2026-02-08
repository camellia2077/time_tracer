// infrastructure/serialization/serializer/core/log_deserializer.cpp
#include "infrastructure/serialization/core/log_deserializer.hpp"

#include <iostream>

#include "infrastructure/schema/day_schema.hpp"
#include "shared/utils/string_utils.hpp"

namespace serializer::core {

auto LogDeserializer::deserialize(const nlohmann::json& day_json) -> DailyLog {
  DailyLog day;
  try {
    namespace json_keys = schema::day::json;

    const auto& headers = day_json.at(json_keys::kHeaders);
    const auto& generated_stats = day_json.at(json_keys::kGeneratedStats);

    // 1. Headers -> Basic Info
    day.date = headers.at(json_keys::kDate);
    day.hasStudyActivity = (headers.value(json_keys::kStatus, 0) != 0);
    day.hasSleepActivity = (headers.value(json_keys::kSleep, 0) != 0);
    day.hasExerciseActivity =
        (headers.value(json_keys::kExercise, 0) != 0);

    std::string getup = headers.value(json_keys::kGetup, "00:00");
    if (getup == "Null") {
      day.isContinuation = true;
      day.getupTime = "";
    } else {
      day.isContinuation = false;
      day.getupTime = getup;
    }

    std::string remark = headers.value(json_keys::kRemark, "");
    if (!remark.empty()) {
      day.generalRemarks = SplitString(remark, '\n');
    }

    day.activityCount = headers.value(json_keys::kActivityCount, 0);

    // 2. Generated Stats -> Stats Struct
    day.stats.sleep_night_time =
        generated_stats.value(json_keys::kSleepNightTime, 0);
    day.stats.sleep_day_time =
        generated_stats.value(json_keys::kSleepDayTime, 0);
    day.stats.sleep_total_time =
        generated_stats.value(json_keys::kSleepTotalTime, 0);

    day.stats.total_exercise_time =
        generated_stats.value(json_keys::kTotalExerciseTime, 0);
    day.stats.cardio_time = generated_stats.value(json_keys::kCardioTime, 0);
    day.stats.anaerobic_time =
        generated_stats.value(json_keys::kAnaerobicTime, 0);

    day.stats.grooming_time =
        generated_stats.value(json_keys::kGroomingTime, 0);
    day.stats.toilet_time = generated_stats.value(json_keys::kToiletTime, 0);
    day.stats.gaming_time = generated_stats.value(json_keys::kGamingTime, 0);

    day.stats.recreation_time =
        generated_stats.value(json_keys::kRecreationTime, 0);
    day.stats.recreation_zhihu_time =
        generated_stats.value(json_keys::kRecreationZhihuTime, 0);
    day.stats.recreation_bilibili_time =
        generated_stats.value(json_keys::kRecreationBilibiliTime, 0);
    day.stats.recreation_douyin_time =
        generated_stats.value(json_keys::kRecreationDouyinTime, 0);

    day.stats.study_time = generated_stats.value(json_keys::kTotalStudyTime, 0);

    // 3. Activities
    const auto& activities_array = day_json.at(json_keys::kActivities);
    if (activities_array.is_array()) {
      for (const auto& activity_json : activities_array) {
        BaseActivityRecord record;
        record.logical_id = activity_json.at(json_keys::kLogicalId);
        record.start_timestamp = activity_json.at(json_keys::kStartTimestamp);
        record.end_timestamp = activity_json.at(json_keys::kEndTimestamp);
        record.start_time_str = activity_json.at(json_keys::kStartTime);
        record.end_time_str = activity_json.at(json_keys::kEndTime);
        record.duration_seconds =
            activity_json.at(json_keys::kDurationSeconds);

        if (activity_json.contains(json_keys::kActivityRemark) &&
            !activity_json[json_keys::kActivityRemark].is_null()) {
          record.remark =
              activity_json[json_keys::kActivityRemark].get<std::string>();
        }

        const auto& activity_details = activity_json.at(json_keys::kActivity);
        record.project_path = activity_details.at(json_keys::kProjectPath);

        day.processedActivities.push_back(record);
      }
    }

  } catch (const nlohmann::json::exception& e) {
    std::cerr << "Deserialization error for day: " << e.what() << std::endl;
    throw;
  }

  return day;
}

}  // namespace serializer::core
