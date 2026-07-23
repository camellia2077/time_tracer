// infrastructure/config/config.cpp
#include "infrastructure/config/config.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

#include "infra/config/loader/alias_mapping_index_utils.hpp"

namespace {
constexpr double kDefaultGenerationChance = 0.5;
constexpr double kMinProbability = 0.0;
constexpr double kMaxProbability = 1.0;
constexpr double kDefaultNoSleepProbability = 1.0;

namespace modalias = tracer::core::infrastructure::config::loader::detail;

auto ReadTomlFile(const std::filesystem::path& path) -> toml::table {
  std::ifstream in_file(path);
  if (!in_file.is_open()) {
    throw std::runtime_error("Could not open TOML file: " + path.string());
  }

  std::stringstream buffer;
  buffer << in_file.rdbuf();
  return toml::parse(buffer.str());
}
}  // namespace

// --- Public Method Implementation ---

auto ConfigLoader::load_from_sources(
    const std::string& settings_content,
    const std::filesystem::path& alias_directory_path)
    -> std::optional<TomlConfigData> {
  TomlConfigData config_data;

  // 解析顺序：映射 -> 设置
  if (!_parse_mapping_keys(alias_directory_path, config_data) ||
      !_parse_settings(settings_content, config_data)) {
    return std::nullopt;
  }

  return config_data;
}

auto ConfigLoader::_parse_mapping_keys(
    const std::filesystem::path& alias_directory_path,
                                       TomlConfigData& config_data) -> bool {
  try {
    const modalias::AliasMappingDefinition definition =
        modalias::LoadAliasMappingDefinition(alias_directory_path, ReadTomlFile);

    for (const auto& entry : definition.expanded_entries) {
      config_data.mapped_activities.push_back(ActivityTokenVariant{
          .alias_token = entry.alias_key,
          .canonical_token = entry.canonical_value,
      });
    }
    return true;

  } catch (const toml::parse_error& e) {
    std::cerr << "TOML Parse Error in alias config: "
              << e.description() << "\n";
    return false;
  } catch (const std::runtime_error& e) {
    std::cerr << "Alias mapping load error: " << e.what() << "\n";
    return false;
  }
}

auto ConfigLoader::_parse_settings(const std::string& content,
                                   TomlConfigData& config_data) -> bool {
  try {
    toml::table tbl = toml::parse(content);
    _load_daily_remarks(tbl, config_data);
    _load_activity_remarks(tbl, config_data);
    _load_wake_keywords(tbl, config_data);
    _load_nosleep_settings(tbl, config_data);
    return true;
  } catch (const toml::parse_error& e) {
    std::cerr << "TOML Parse Error in settings config: " << e.description()
              << "\n";
    return false;
  }
}

void ConfigLoader::_load_daily_remarks(const toml::table& data,
                                       TomlConfigData& config) {
  if (const auto* node = data.get_as<toml::table>("daily_remarks")) {
    DailyRemarkConfig remarks;
    remarks.generation_chance =
        (*node)["generation_chance"].value_or(kDefaultGenerationChance);

    int64_t lines_value = (*node)["max_lines"].value_or(1);
    remarks.max_lines = static_cast<int>(lines_value < 1 ? 1 : lines_value);

    if (const auto* arr = (*node).get_as<toml::array>("contents")) {
      for (auto&& value_node : *arr) {
        remarks.contents.emplace_back(value_node.value_or(""));
      }
      config.remarks.emplace(remarks);
    }
  }
}

void ConfigLoader::_load_activity_remarks(const toml::table& data,
                                          TomlConfigData& config) {
  if (const auto* node = data.get_as<toml::table>("activity_remarks")) {
    ActivityRemarkConfig activity_remarks;
    int64_t lines_value = (*node)["max_lines"].value_or(4);
    activity_remarks.max_lines =
        static_cast<int>(lines_value < 1 ? 1 : lines_value);
    if (const auto* arr = (*node).get_as<toml::array>("contents")) {
      for (auto&& value_node : *arr) {
        activity_remarks.contents.emplace_back(value_node.value_or(""));
      }
      config.activity_remarks.emplace(activity_remarks);
    }
  }
}

void ConfigLoader::_load_wake_keywords(const toml::table& data,
                                       TomlConfigData& config) {
  if (const auto* arr = data.get_as<toml::array>("wake_keywords")) {
    for (auto&& value_node : *arr) {
      config.wake_keywords.emplace_back(value_node.value_or(""));
    }
  } else {
    config.wake_keywords.emplace_back("起床");
  }
}

void ConfigLoader::_load_nosleep_settings(const toml::table& data,
                                          TomlConfigData& config) {
  if (const auto* node = data.get_as<toml::table>("nosleep")) {
    const double raw_probability =
        (*node)["probability"].value_or(kDefaultNoSleepProbability);
    config.nosleep_probability = std::clamp(raw_probability, kMinProbability,
                                            kMaxProbability);
  }
}
