// infrastructure/config/validator/reports/strategies/report_query_strategies.cpp
#include "infrastructure/config/validator/reports/strategies/report_query_strategies.hpp"

#include <algorithm>
#include <set>
#include <string>

#include "domain/ports/diagnostics.hpp"

namespace {

[[nodiscard]] auto IsHexDigit(char value) -> bool {
  return (value >= '0' && value <= '9') || (value >= 'a' && value <= 'f') ||
         (value >= 'A' && value <= 'F');
}

void EmitValidationError(const std::string& message) {
  time_tracer::domain::ports::EmitError(message);
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
auto ValidateStatsRecursive(const toml::array& items_array,
                            const std::string& config_file_name,
                            const std::string& context = "root") -> bool {
  for (size_t i = 0; i < items_array.size(); ++i) {
    const auto& item_node = items_array[i];
    std::string current_context = context + "[" + std::to_string(i) + "]";

    if (!item_node.is_table()) {
      EmitValidationError("[Validator] Error in " + config_file_name +
                          ": Item " + current_context + " must be a table.");
      return false;
    }

    const auto& item = *item_node.as_table();

    if (!item.contains("label") || !item["label"].is_string()) {
      EmitValidationError("[Validator] Error in " + config_file_name +
                          ": Item " + current_context +
                          " must contain a string 'label'.");
      return false;
    }
    if (item.contains("show") && !item["show"].is_boolean()) {
      EmitValidationError("[Validator] Error in " + config_file_name +
                          ": Item " + current_context +
                          " 'show' must be a boolean.");
      return false;
    }
    if (item.contains("db_column") && !item["db_column"].is_string()) {
      EmitValidationError("[Validator] Error in " + config_file_name +
                          ": 'db_column' in " + current_context +
                          " must be a string.");
      return false;
    }

    if (item.contains("sub_items")) {
      if (const toml::array* sub_arr = item["sub_items"].as_array()) {
        if (!ValidateStatsRecursive(*sub_arr, config_file_name,
                                    current_context + ".sub_items")) {
          return false;
        }
      } else {
        EmitValidationError("[Validator] Error in " + config_file_name +
                            ": 'sub_items' in " + current_context +
                            " must be an array.");
        return false;
      }
    }
  }
  return true;
}
// NOLINTEND(bugprone-easily-swappable-parameters)

auto ValidateRequiredKeys(const toml::table& query_config,
                          const std::string& file_name,
                          const std::set<std::string>& required_keys,
                          const std::string& report_type) -> bool {
  return std::ranges::all_of(required_keys, [&](const auto& key) {
    if (query_config.contains(key)) {
      return true;
    }
    time_tracer::domain::ports::EmitError("[Validator] Error in " + file_name +
                                          ": Missing " + report_type +
                                          " report key '" + key + "'.");
    return false;
  });
}

auto ValidatePeriodKeys(const toml::table& query_config,
                        const std::string& file_name,
                        const std::string& report_type) -> bool {
  const std::set<std::string> kPeriodKeys = {
      "title_template",        "actual_days_label",      "status_days_label",
      "sleep_days_label",      "exercise_days_label",    "cardio_days_label",
      "anaerobic_days_label",  "total_time_label",       "no_records_message",
      "invalid_range_message", "project_breakdown_label"};

  return std::ranges::all_of(kPeriodKeys, [&](const auto& key) {
    if (query_config.contains(key)) {
      return true;
    }
    time_tracer::domain::ports::EmitError("[Validator] Error in " + file_name +
                                          ": Missing " + report_type +
                                          " report key '" + key + "'.");
    return false;
  });
}

}  // namespace

auto BaseStrategy::Validate(const toml::table& query_config,
                            const std::string& file_name) const -> bool {
  if (!ValidateSpecificKeys(query_config, file_name)) {
    return false;
  }
  if (!ValidateCommonRules(query_config, file_name)) {
    return false;
  }
  if (file_name.find("Day") != std::string::npos) {
    if (!ValidateStatisticsItems(query_config, file_name)) {
      return false;
    }
  }
  return true;
}

auto BaseStrategy::ValidateCommonRules(const toml::table& query_config,
                                       const std::string& file_name) -> bool {
  const std::set<std::string> kNumericKeys = {
      "base_font_size",   "report_title_font_size", "category_title_font_size",
      "margin_in",        "margin_top_cm",          "margin_bottom_cm",
      "margin_left_cm",   "margin_right_cm",        "list_top_sep_pt",
      "list_item_sep_ex", "line_spacing_em"};

  for (const auto& [key, node] : query_config) {
    std::string key_str(key.str());
    if (key_str == "statistics_items") {
      continue;
    }

    if (kNumericKeys.contains(key_str) && !node.is_number()) {
      EmitValidationError("[Validator] Error in " + file_name + ": Key '" +
                          key_str + "' must have a numeric value.");
      return false;
    }

    if (key_str == "keyword_colors") {
      if (!node.is_table()) {
        EmitValidationError("[Validator] Error in " + file_name +
                            ": 'keyword_colors' must be a table.");
        return false;
      }
      const auto& colors = *node.as_table();
      for (const auto& [color_key, color_val] : colors) {
        if (!color_val.is_string() ||
            !IsValidHexColor(color_val.value<std::string>().value_or(""))) {
          EmitValidationError("[Validator] Error in " + file_name +
                              ": Color for '" + std::string(color_key.str()) +
                              "' is not a valid hex string (e.g., #RRGGBB).");
          return false;
        }
      }
    }

    if (node.is_string()) {
      std::string val_str = node.value<std::string>().value_or("");
      if (val_str.empty() && key_str != "remark" && key_str != "db_column") {
      }
    }
  }
  return true;
}

auto BaseStrategy::ValidateStatisticsItems(const toml::table& query_config,
                                           const std::string& file_name)
    -> bool {
  if (!query_config.contains("statistics_items")) {
    return true;
  }

  const auto& items_node = query_config["statistics_items"];
  if (!items_node.is_array()) {
    EmitValidationError("[Validator] Error in " + file_name +
                        ": 'statistics_items' must be an array.");
    return false;
  }

  return ValidateStatsRecursive(*items_node.as_array(), file_name);
}

auto BaseStrategy::IsValidHexColor(const std::string& color_string) -> bool {
  constexpr size_t kHexColorLength = 7;
  if (color_string.size() != kHexColorLength || color_string[0] != '#') {
    return false;
  }
  return std::all_of(color_string.begin() + 1, color_string.end(),
                     [](char value) -> bool { return IsHexDigit(value); });
}

auto DailyMd::ValidateSpecificKeys(const toml::table& query_config,
                                   const std::string& file_name) const -> bool {
  const std::set<std::string> kDailyMdKeys = {
      "title_prefix",          "date_label",
      "total_time_label",      "status_label",
      "sleep_label",           "exercise_label",
      "getup_time_label",      "remark_label",
      "statistics_label",      "all_activities_label",
      "activity_remark_label", "activity_connector",
      "statistics_items",      "no_records_message"};

  return ValidateRequiredKeys(query_config, file_name, kDailyMdKeys, "daily");
}

auto DailyTex::ValidateSpecificKeys(const toml::table& query_config,
                                    const std::string& file_name) const
    -> bool {
  const std::set<std::string> kDailyTexKeys = {
      "report_title",       "date_label",           "total_time_label",
      "status_label",       "sleep_label",          "exercise_label",
      "getup_time_label",   "remark_label",         "no_records_message",
      "statistics_label",   "all_activities_label", "activity_remark_label",
      "activity_connector", "keyword_colors",       "statistics_items"};

  return ValidateRequiredKeys(query_config, file_name, kDailyTexKeys, "daily");
}

auto DailyTyp::ValidateSpecificKeys(const toml::table& query_config,
                                    const std::string& file_name) const
    -> bool {
  const std::set<std::string> kDailyTypKeys = {
      "title_prefix",       "date_label",           "total_time_label",
      "status_label",       "sleep_label",          "getup_time_label",
      "remark_label",       "exercise_label",       "no_records_message",
      "statistics_label",   "all_activities_label", "activity_remark_label",
      "activity_connector", "keyword_colors",       "statistics_items"};

  return ValidateRequiredKeys(query_config, file_name, kDailyTypKeys, "daily");
}

auto Monthly::ValidateSpecificKeys(const toml::table& query_config,
                                   const std::string& file_name) const -> bool {
  return ValidatePeriodKeys(query_config, file_name, "monthly");
}

auto Weekly::ValidateSpecificKeys(const toml::table& query_config,
                                  const std::string& file_name) const -> bool {
  return ValidatePeriodKeys(query_config, file_name, "weekly");
}

auto Yearly::ValidateSpecificKeys(const toml::table& query_config,
                                  const std::string& file_name) const -> bool {
  return ValidatePeriodKeys(query_config, file_name, "yearly");
}

auto StrategyFactory::CreateStrategy(const std::string& file_name)
    -> std::unique_ptr<IQueryStrategy> {
  if (file_name.find("DayMd") != std::string::npos) {
    return std::make_unique<DailyMd>();
  }
  if (file_name.find("DayTex") != std::string::npos) {
    return std::make_unique<DailyTex>();
  }
  if (file_name.find("DayTyp") != std::string::npos) {
    return std::make_unique<DailyTyp>();
  }
  if (file_name.find("Month") != std::string::npos) {
    return std::make_unique<Monthly>();
  }
  if (file_name.find("Week") != std::string::npos) {
    return std::make_unique<Weekly>();
  }
  if (file_name.find("Year") != std::string::npos) {
    return std::make_unique<Yearly>();
  }
  return nullptr;
}
