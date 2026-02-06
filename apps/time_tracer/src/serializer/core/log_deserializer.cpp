// serializer/core/log_deserializer.cpp
#include "serializer/core/log_deserializer.hpp"

#include <iostream>

#include "common/utils/string_utils.hpp"

namespace serializer::core {

auto LogDeserializer::deserialize(const nlohmann::json& day_json) -> DailyLog {
  DailyLog day;
  try {
    const auto& headers = day_json.at("headers");
    const auto& generated_stats = day_json.at("generated_stats");

    // 1. Headers -> Basic Info
    day.date = headers.at("date");
    day.hasStudyActivity = (headers.value("status", 0) != 0);
    day.hasSleepActivity = (headers.value("sleep", 0) != 0);
    day.hasExerciseActivity = (headers.value("exercise", 0) != 0);

    std::string getup = headers.value("getup", "00:00");
    if (getup == "Null") {
      day.isContinuation = true;
      day.getupTime = "";
    } else {
      day.isContinuation = false;
      day.getupTime = getup;
    }

    std::string remark = headers.value("remark", "");
    if (!remark.empty()) {
      day.generalRemarks = SplitString(remark, '\n');
    }

    day.activityCount = headers.value("activity_count", 0);

    // 2. Generated Stats -> Stats Struct
    day.stats.sleep_night_time = generated_stats.value("sleep_night_time", 0);
    day.stats.sleep_day_time = generated_stats.value("sleep_day_time", 0);
    day.stats.sleep_total_time = generated_stats.value("sleep_total_time", 0);

    day.stats.total_exercise_time =
        generated_stats.value("total_exercise_time", 0);
    day.stats.cardio_time = generated_stats.value("cardio_time", 0);
    day.stats.anaerobic_time = generated_stats.value("anaerobic_time", 0);

    day.stats.grooming_time = generated_stats.value("grooming_time", 0);
    day.stats.toilet_time = generated_stats.value("toilet_time", 0);
    day.stats.gaming_time = generated_stats.value("gaming_time", 0);

    day.stats.recreation_time = generated_stats.value("recreation_time", 0);
    day.stats.recreation_zhihu_time =
        generated_stats.value("recreation_zhihu_time", 0);
    day.stats.recreation_bilibili_time =
        generated_stats.value("recreation_bilibili_time", 0);
    day.stats.recreation_douyin_time =
        generated_stats.value("recreation_douyin_time", 0);

    day.stats.study_time = generated_stats.value("total_study_time", 0);

    // 3. Activities
    const auto& activities_array = day_json.at("activities");
    if (activities_array.is_array()) {
      for (const auto& activity_json : activities_array) {
        BaseActivityRecord record;
        record.logical_id = activity_json.at("logical_id");
        record.start_timestamp = activity_json.at("start_timestamp");
        record.end_timestamp = activity_json.at("end_timestamp");
        record.start_time_str = activity_json.at("start_time");
        record.end_time_str = activity_json.at("end_time");
        record.duration_seconds = activity_json.at("duration_seconds");

        if (activity_json.contains("activity_remark") &&
            !activity_json["activity_remark"].is_null()) {
          record.remark = activity_json["activity_remark"].get<std::string>();
        }

        const auto& activity_details = activity_json.at("activity");
        record.project_path = activity_details.at("project_path");

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