// reprocessing/validator/output_json/pipelines/JsonValidatorTime.cpp
#include "JsonValidatorTime.hpp"

using json = nlohmann::json;

void validateTimeContinuity(const json& day_object, std::set<Error>& errors) {
    if (!day_object.contains("activities") || !day_object["activities"].is_array() || day_object["activities"].empty()) {
        return;
    }

    const auto& headers = day_object.value("headers", json::object());
    const auto& activities = day_object["activities"];
    
    std::string last_end_time = headers.value("getup", "00:00");
    if (last_end_time == "Null") {
        if (!activities.empty()) {
            last_end_time = activities[0].value("startTime", "");
        }
    }

    for (const auto& activity : activities) {
        std::string start_time = activity.value("startTime", "");
        if (start_time != last_end_time) {
            std::string date_str = headers.value("date", "[Unknown Date]");
            std::string error_msg = "Time discontinuity in file for date " + date_str + ". Expected start time " + last_end_time + ", but got " + start_time + ".";
            errors.insert({0, error_msg, ErrorType::TimeDiscontinuity});
        }
        last_end_time = activity.value("endTime", "");
    }
}