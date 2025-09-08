// reprocessing/validator/output_json/JsonValidatorRules.cpp
#include "JsonValidatorRules.hpp"

using json = nlohmann::json;

void validateHighLevelRules(const json& day_object, std::set<Error>& errors) {
    const auto& headers = day_object.value("Headers", json::object());
    bool sleep_status = headers.value("Sleep", false);

    if (sleep_status) {
        if (!day_object.contains("Activities") || !day_object["Activities"].is_array() || day_object["Activities"].empty()) {
             std::string date_str = headers.value("Date", "[Unknown Date]");
             errors.insert({0, "In file for date " + date_str + ": Last activity must be 'sleep' when Sleep is True, but no activities found.", ErrorType::MissingSleepNight});
             return;
        }
        const auto& last_activity = day_object["Activities"].back();
        
        std::string top_parent_val = last_activity.value("activity", json::object()).value("top_parent", "");
        
        if (top_parent_val != "sleep") {
            std::string date_str = headers.value("Date", "[Unknown Date]");
            errors.insert({0, "In file for date " + date_str + ": The last activity must be 'sleep' when Sleep is True.", ErrorType::MissingSleepNight});
        }
    }
}