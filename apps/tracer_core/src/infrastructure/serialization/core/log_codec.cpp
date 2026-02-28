// infrastructure/serialization/core/log_codec.cpp
#include "infrastructure/serialization/core/log_codec.hpp"

#include <string>

#include "domain/ports/diagnostics.hpp"
#include "infrastructure/schema/day_schema.hpp"
#include "shared/utils/string_utils.hpp"

namespace serializer::core {

auto LogSerializer::Serialize(const DailyLog& day) -> nlohmann::json {
  if (day.date.empty()) {
    return nlohmann::json{};
  }

  namespace json_keys = schema::day::json;

  nlohmann::json day_obj;

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

auto LogDeserializer::Deserialize(const nlohmann::json& day_json) -> DailyLog {
  DailyLog day;
  try {
    namespace json_keys = schema::day::json;

    const auto& headers = day_json.at(json_keys::kHeaders);
    const auto& generated_stats = day_json.at(json_keys::kGeneratedStats);

    day.date = headers.at(json_keys::kDate);
    day.hasStudyActivity = (headers.value(json_keys::kStatus, 0) != 0);
    day.hasSleepActivity = (headers.value(json_keys::kSleep, 0) != 0);
    day.hasExerciseActivity = (headers.value(json_keys::kExercise, 0) != 0);

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

    const auto& activities_array = day_json.at(json_keys::kActivities);
    if (activities_array.is_array()) {
      for (const auto& activity_json : activities_array) {
        BaseActivityRecord record;
        record.logical_id = activity_json.at(json_keys::kLogicalId);
        record.start_timestamp = activity_json.at(json_keys::kStartTimestamp);
        record.end_timestamp = activity_json.at(json_keys::kEndTimestamp);
        record.start_time_str = activity_json.at(json_keys::kStartTime);
        record.end_time_str = activity_json.at(json_keys::kEndTime);
        record.duration_seconds = activity_json.at(json_keys::kDurationSeconds);

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
    tracer_core::domain::ports::EmitError("Deserialization error for day: " +
                                          std::string(e.what()));
    throw;
  }

  return day;
}

}  // namespace serializer::core
