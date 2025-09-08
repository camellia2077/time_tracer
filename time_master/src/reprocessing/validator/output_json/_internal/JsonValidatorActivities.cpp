// reprocessing/validator/output_json/JsonValidatorActivities.cpp
#include "JsonValidatorActivities.hpp"

using json = nlohmann::json;
// 因为程序依靠前一天的最后一项内容的时间,与当前天的起床时间来生成sleep_night
// 如果一天只有一项,那生成逻辑就会错乱
// 验证一天的活动数量是否至少为2
void validateActivityCount(const json& day_object, std::set<Error>& errors) {
    const auto& headers = day_object.value("Headers", json::object());
    std::string date_str = headers.value("Date", "[Unknown Date]");

    if (day_object.contains("Activities") && day_object["Activities"].is_array()) {
        if (day_object["Activities"].size() < 2) {
            errors.insert({0, "In file for date " + date_str + ": The day has less than 2 activities. This may cause issues with 'sleep_night' generation.", ErrorType::Json_TooFewActivities});
        }
    } else {
        errors.insert({0, "In file for date " + date_str + ": 'Activities' field is missing or not an array.", ErrorType::Json_TooFewActivities});
    }
}