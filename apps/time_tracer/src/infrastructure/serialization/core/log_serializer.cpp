// infrastructure/serialization/serializer/core/log_serializer.cpp
#include "infrastructure/serialization/core/log_serializer.hpp"

#include "infrastructure/schema/day_schema.hpp"

namespace serializer::core {

auto LogSerializer::Serialize(const DailyLog& day) -> nlohmann::json {
  if (day.date.empty()) {
    return nlohmann::json{};
  }

  namespace json_keys = schema::day::json;

  nlohmann::json day_obj;

  // 1. Headers
  nlohmann::json headers_obj;
  headers_obj[json_keys::kDate] = day.date;
  headers_obj[json_keys::kStatus] = static_cast<int>(day.hasStudyActivity);
  headers_obj[json_keys::kExercise] = static_cast<int>(day.hasExerciseActivity);
  headers_obj[json_keys::kSleep] = static_cast<int>(day.hasSleepActivity);
  headers_obj[json_keys::kCardio] = (day.stats.cardio_time > 0) ? 1 : 0;
  headers_obj[json_keys::kAnaerobic] = (day.stats.anaerobic_time > 0) ? 1 : 0;
  std::string getup_time = "Null";
  if (!day.isContinuation) {
    getup_time = day.getupTime.empty() ? "00:00" : day.getupTime;
  }
  headers_obj[json_keys::kGetup] = getup_time;
  headers_obj[json_keys::kActivityCount] = day.activityCount;

  if (!day.generalRemarks.empty()) {
    std::string full_remark;
    for (size_t i = 0; i < day.generalRemarks.size(); ++i) {
      full_remark += day.generalRemarks[i];
      if (i < day.generalRemarks.size() - 1) {
        full_remark += "\n";
      }
    }
    headers_obj[json_keys::kRemark] = full_remark;
  } else {
    headers_obj[json_keys::kRemark] = "";
  }
  day_obj[json_keys::kHeaders] = headers_obj;

  // 2. Activities
  nlohmann::json activities = nlohmann::json::array();
  for (const auto& activity_data : day.processedActivities) {
    nlohmann::json activity_obj;
    activity_obj[json_keys::kLogicalId] = activity_data.logical_id;
    activity_obj[json_keys::kStartTimestamp] = activity_data.start_timestamp;
    activity_obj[json_keys::kEndTimestamp] = activity_data.end_timestamp;
    activity_obj[json_keys::kStartTime] = activity_data.start_time_str;
    activity_obj[json_keys::kEndTime] = activity_data.end_time_str;
    activity_obj[json_keys::kDurationSeconds] = activity_data.duration_seconds;

    if (activity_data.remark.has_value()) {
      activity_obj[json_keys::kActivityRemark] = activity_data.remark.value();
    } else {
      activity_obj[json_keys::kActivityRemark] = nullptr;
    }

    nlohmann::json activity_details;
    activity_details[json_keys::kProjectPath] = activity_data.project_path;
    activity_obj[json_keys::kActivity] = activity_details;
    activities.push_back(activity_obj);
  }
  day_obj[json_keys::kActivities] = activities;

  // 3. Generated Stats
  nlohmann::json generated_stats_obj;
  generated_stats_obj[json_keys::kSleepNightTime] = day.stats.sleep_night_time;
  generated_stats_obj[json_keys::kSleepDayTime] = day.stats.sleep_day_time;
  generated_stats_obj[json_keys::kSleepTotalTime] = day.stats.sleep_total_time;
  generated_stats_obj[json_keys::kTotalExerciseTime] =
      day.stats.total_exercise_time;
  generated_stats_obj[json_keys::kCardioTime] = day.stats.cardio_time;
  generated_stats_obj[json_keys::kAnaerobicTime] = day.stats.anaerobic_time;
  generated_stats_obj[json_keys::kGroomingTime] = day.stats.grooming_time;
  generated_stats_obj[json_keys::kToiletTime] = day.stats.toilet_time;
  generated_stats_obj[json_keys::kGamingTime] = day.stats.gaming_time;
  generated_stats_obj[json_keys::kRecreationTime] = day.stats.recreation_time;
  generated_stats_obj[json_keys::kRecreationZhihuTime] =
      day.stats.recreation_zhihu_time;
  generated_stats_obj[json_keys::kRecreationBilibiliTime] =
      day.stats.recreation_bilibili_time;
  generated_stats_obj[json_keys::kRecreationDouyinTime] =
      day.stats.recreation_douyin_time;
  generated_stats_obj[json_keys::kTotalStudyTime] = day.stats.study_time;

  day_obj[json_keys::kGeneratedStats] = generated_stats_obj;

  return day_obj;
}

}  // namespace serializer::core
