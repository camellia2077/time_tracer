// infrastructure/io/processed_json_validation.cpp
#include "infrastructure/io/processed_json_validation.hpp"

#include <nlohmann/json.hpp>

#include <string>
#include <utility>

#include "infrastructure/schema/day_schema.hpp"

namespace infrastructure::io {

namespace {

auto ResolveDisplayDate(const ProcessedJsonDayValidationInput& day)
    -> std::string {
  return day.has_date_string ? day.date : "[Unknown Date]";
}

}  // namespace

auto BuildProcessedJsonValidationInput(const nlohmann::json& payload)
    -> ProcessedJsonValidationInput {
  ProcessedJsonValidationInput input;
  input.has_root_array = payload.is_array();

  if (!input.has_root_array) {
    return input;
  }

  namespace json_keys = schema::day::json;

  for (const auto& day_json : payload) {
    ProcessedJsonDayValidationInput day_input;
    day_input.has_day_object = day_json.is_object();

    if (!day_input.has_day_object) {
      input.days.push_back(std::move(day_input));
      continue;
    }

    const auto headers_it = day_json.find(json_keys::kHeaders);
    if (headers_it != day_json.end() && headers_it->is_object()) {
      day_input.has_headers_object = true;

      const auto date_it = headers_it->find(json_keys::kDate);
      if (date_it != headers_it->end() && date_it->is_string()) {
        day_input.has_date_string = true;
        day_input.date = date_it->get<std::string>();
      }
    }

    const auto activities_it = day_json.find(json_keys::kActivities);
    if (activities_it != day_json.end() && activities_it->is_array()) {
      day_input.has_activities_array = true;
      day_input.activity_count = activities_it->size();
    }

    input.days.push_back(std::move(day_input));
  }

  return input;
}

auto CollectProcessedJsonValidationErrors(
    std::string_view source,
    const ProcessedJsonValidationInput& input)
    -> std::vector<tracer_core::application::ports::ProcessedDataLoadError> {
  std::vector<tracer_core::application::ports::ProcessedDataLoadError> errors;

  if (!input.has_root_array) {
    errors.push_back(
        {.source = std::string(source),
         .message = "Processed data JSON root must be an array."});
    return errors;
  }

  for (const auto& day : input.days) {
    const std::string display_date = ResolveDisplayDate(day);

    if (!day.has_day_object) {
      errors.push_back(
          {.source = std::string(source),
           .message = "In file for date " + display_date +
                      ": Each day entry must be a JSON object."});
      continue;
    }

    if (!day.has_headers_object || !day.has_date_string) {
      errors.push_back(
          {.source = std::string(source),
           .message = "In file for date " + display_date +
                      ": 'headers.date' field is missing or not a string."});
    }

    if (!day.has_activities_array) {
      errors.push_back(
          {.source = std::string(source),
           .message = "In file for date " + display_date +
                      ": 'activities' field is missing or not an array."});
      continue;
    }

    if (day.activity_count < 2) {
      errors.push_back(
          {.source = std::string(source),
           .message =
               "In file for date " + display_date +
               ": The day has less than 2 activities. This may cause issues "
               "with 'sleep' activity generation."});
    }
  }

  return errors;
}

}  // namespace infrastructure::io
