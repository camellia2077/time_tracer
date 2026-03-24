// infra/serialization/json_serializer.cpp
#include "infra/serialization/json_serializer.hpp"

#include <nlohmann/json.hpp>

#include <string>

#include "infra/reporting/data/utils/time_derived_stats.hpp"
#include "infra/schema/day_schema.hpp"

import tracer.core.domain.ports.diagnostics;
import tracer.core.shared.string_utils;

using tracer::core::shared::string_utils::SplitString;

namespace modports = tracer::core::domain::ports;

namespace serializer {
namespace {

using tracer::core::infrastructure::reports::data::stats::
    DerivedTimeStatsAggregator;

auto RebuildDerivedFlags(DailyLog& day) -> void {
  day.activityCount = static_cast<int>(day.processedActivities.size());
  day.hasStudyActivity = false;
  day.hasExerciseActivity = false;
  // headers.wake_anchor is a day-level semantic flag: valid getup anchor +
  // not a continuation day. It is intentionally independent from whether a
  // generated sleep_night activity exists in processedActivities.
  day.hasWakeAnchor = !day.isContinuation && !day.getupTime.empty() &&
                      day.getupTime != "00:00";

  for (const auto& activity : day.processedActivities) {
    if (activity.project_path.starts_with("study")) {
      day.hasStudyActivity = true;
    }
    if (activity.project_path.starts_with("exercise")) {
      day.hasExerciseActivity = true;
    }
  }
}

auto SerializeLogToJsonObject(const DailyLog& day) -> nlohmann::json {
  if (day.date.empty()) {
    return nlohmann::json{};
  }

  namespace json_keys = schema::day::json;

  nlohmann::json day_obj;
  DerivedTimeStatsAggregator stats_aggregator;
  for (const auto& activity_data : day.processedActivities) {
    stats_aggregator.AddPathDuration(activity_data.project_path,
                                     activity_data.duration_seconds);
  }

  nlohmann::json headers_obj;
  headers_obj[json_keys::kDate] = day.date;
  // Persist the public wake-anchor header from day semantics rather than from
  // sleep activity presence. Auto-generated sleep activities affect the fact
  // set and time stats, but not this header's meaning.
  headers_obj[json_keys::kWakeAnchor] =
      (!day.isContinuation && !day.getupTime.empty() && day.getupTime != "00:00")
          ? 1
          : 0;
  headers_obj[json_keys::kCardio] = stats_aggregator.HasCardioActivity() ? 1 : 0;
  headers_obj[json_keys::kAnaerobic] =
      stats_aggregator.HasAnaerobicActivity() ? 1 : 0;
  std::string getup_time = "Null";
  if (!day.isContinuation) {
    getup_time = day.getupTime.empty() ? "00:00" : day.getupTime;
  }
  headers_obj[json_keys::kGetup] = getup_time;
  headers_obj[json_keys::kActivityCount] =
      static_cast<int>(day.processedActivities.size());

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

  return day_obj;
}

auto DeserializeLogFromJsonObject(const nlohmann::json& day_json) -> DailyLog {
  DailyLog day;
  try {
    namespace json_keys = schema::day::json;

    const auto& headers = day_json.at(json_keys::kHeaders);

    day.date = headers.at(json_keys::kDate);

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

    RebuildDerivedFlags(day);
  } catch (const nlohmann::json::exception& e) {
    modports::EmitError("Deserialization error for day: " +
                        std::string(e.what()));
    throw;
  }

  return day;
}

auto BuildDaysJson(const std::vector<DailyLog>& days) -> nlohmann::json {
  nlohmann::json json_array = nlohmann::json::array();
  for (const auto& day : days) {
    if (!day.date.empty()) {
      json_array.push_back(SerializeLogToJsonObject(day));
    }
  }
  return json_array;
}

auto ParseJsonText(std::string_view json_text) -> nlohmann::json {
  return nlohmann::json::parse(json_text.begin(), json_text.end());
}

}  // namespace

auto JsonSerializer::SerializeDay(const DailyLog& day, int indent)
    -> std::string {
  return SerializeLogToJsonObject(day).dump(indent);
}

auto JsonSerializer::SerializeDays(const std::vector<DailyLog>& days)
    -> std::string {
  return SerializeDays(days, -1);
}

auto JsonSerializer::SerializeDays(const std::vector<DailyLog>& days, int indent)
    -> std::string {
  return BuildDaysJson(days).dump(indent);
}

auto JsonSerializer::DeserializeDay(std::string_view day_json) -> DailyLog {
  return DeserializeLogFromJsonObject(ParseJsonText(day_json));
}

auto JsonSerializer::DeserializeDays(std::string_view json_array)
    -> std::vector<DailyLog> {
  const nlohmann::json kParsed = ParseJsonText(json_array);
  std::vector<DailyLog> days;
  if (!kParsed.is_array()) {
    return days;
  }
  for (const auto& json_item : kParsed) {
    days.push_back(DeserializeLogFromJsonObject(json_item));
  }
  return days;
}

}  // namespace serializer
