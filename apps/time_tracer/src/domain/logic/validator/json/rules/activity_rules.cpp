// domain/logic/validator/json/rules/activity_rules.cpp
#include "domain/logic/validator/json/rules/activity_rules.hpp"

namespace validator::json {

void validateActivityCount(const nlohmann::json& day_object,
                           std::set<Error>& errors) {
  const auto& headers = day_object.value("headers", nlohmann::json::object());
  std::string date_str = headers.value("date", "[Unknown Date]");

  if (day_object.contains("activities") &&
      day_object["activities"].is_array()) {
    if (day_object["activities"].size() < 2) {
      errors.insert({0,
                     "In file for date " + date_str +
                         ": The day has less than 2 activities. This may cause "
                         "issues with 'sleep' activity generation.",
                     ErrorType::kJsonTooFewActivities});
    }
  } else {
    errors.insert({0,
                   "In file for date " + date_str +
                       ": 'activities' field is missing or not an array.",
                   ErrorType::kJsonTooFewActivities});
  }
}

}  // namespace validator::json
