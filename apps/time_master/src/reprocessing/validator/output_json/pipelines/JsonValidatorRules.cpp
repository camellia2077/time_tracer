// reprocessing/validator/output_json/pipelines/JsonValidatorRules.cpp
#include "JsonValidatorRules.hpp"

using json = nlohmann::json;

void validateHighLevelRules(const json& day_object, std::set<Error>& errors) {
    const auto& headers = day_object.value("headers", json::object());
    // [修改] 更改默认值类型，以匹配JSON中的整数
    int sleep_status = headers.value("sleep", 0);

    // [修改] 检查 sleep_status 是否大于0，以匹配布尔逻辑
    if (sleep_status > 0) {
        if (!day_object.contains("activities") || !day_object["activities"].is_array() || day_object["activities"].empty()) {
             std::string date_str = headers.value("date", "[Unknown Date]");
             errors.insert({0, "In file for date " + date_str + ": Last activity must be 'sleep' when Sleep is True, but no activities found.", ErrorType::MissingSleepNight});
             return;
        }
        const auto& last_activity = day_object["activities"].back();
        
        std::string top_parent_val = last_activity.value("activity", json::object()).value("parent", "");
        
        if (top_parent_val != "sleep") {
            std::string date_str = headers.value("date", "[Unknown Date]");
            errors.insert({0, "In file for date " + date_str + ": The last activity must be 'sleep' when Sleep is True.", ErrorType::MissingSleepNight});
        }
    }
}