// reprocessing/validator/output_json/pipelines/JsonValidatorActivities.cpp
#include "JsonValidatorActivities.hpp"

using json = nlohmann::json;
// 因为程序依靠前一天的最后一项内容的时间,与当前天的起床时间来生成sleep_night
// 如果一天只有一项,那生成逻辑就会错乱
// 验证一天的活动数量是否至少为2
void validateActivityCount(const json& day_object, std::set<Error>& errors) {
    const auto& headers = day_object.value("headers", json::object());
    std::string date_str = headers.value("date", "[Unknown Date]");

    if (day_object.contains("activities") && day_object["activities"].is_array()) {
        if (day_object["activities"].size() < 2) {
            errors.insert({0, "In file for date " + date_str + ": The day has less than 2 activities. This may cause issues with 'sleep' activity generation.", ErrorType::Json_TooFewActivities});
        }
    } else {
        errors.insert({0, "In file for date " + date_str + ": 'activities' field is missing or not an array.", ErrorType::Json_TooFewActivities});
    }
}