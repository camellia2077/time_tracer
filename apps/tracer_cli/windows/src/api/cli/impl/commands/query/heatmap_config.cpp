// api/cli/impl/commands/query/heatmap_config.cpp
#include "api/cli/impl/commands/query/heatmap_config.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace tracer_core::cli::impl::commands::query::heatmap {

namespace {

namespace fs = std::filesystem;

enum class ParseSection {
  kNone,
  kThresholds,
  kDefaults,
  kPalettes,
};

auto TrimAscii(std::string_view input) -> std::string {
  std::size_t begin = 0;
  while (begin < input.size() &&
         std::isspace(static_cast<unsigned char>(input[begin])) != 0) {
    ++begin;
  }
  std::size_t end = input.size();
  while (end > begin &&
         std::isspace(static_cast<unsigned char>(input[end - 1])) != 0) {
    --end;
  }
  return std::string(input.substr(begin, end - begin));
}

auto StripInlineComment(std::string_view line) -> std::string {
  std::string output;
  output.reserve(line.size());
  bool in_string = false;
  bool escaped = false;
  for (const char ch : line) {
    if (escaped) {
      output.push_back(ch);
      escaped = false;
      continue;
    }
    if (in_string && ch == '\\') {
      output.push_back(ch);
      escaped = true;
      continue;
    }
    if (ch == '"') {
      in_string = !in_string;
      output.push_back(ch);
      continue;
    }
    if (!in_string && ch == '#') {
      break;
    }
    output.push_back(ch);
  }
  return TrimAscii(output);
}

auto IsHexColor(std::string_view value) -> bool {
  if (value.size() != 7 || value.front() != '#') {
    return false;
  }
  return std::all_of(value.begin() + 1, value.end(),
                     [](unsigned char code_point) -> bool {
                       return std::isxdigit(code_point) != 0;
                     });
}

auto ParseQuotedString(std::string_view value, std::string_view field)
    -> std::string {
  const std::string trimmed = TrimAscii(value);
  if (trimmed.size() < 2 || trimmed.front() != '"' || trimmed.back() != '"') {
    throw std::runtime_error(std::string(field) + " must be a quoted string.");
  }
  return trimmed.substr(1, trimmed.size() - 2);
}

auto ParseArrayItems(std::string_view value, std::string_view field)
    -> std::vector<std::string> {
  const std::string trimmed = TrimAscii(value);
  if (trimmed.size() < 2 || trimmed.front() != '[' || trimmed.back() != ']') {
    throw std::runtime_error(std::string(field) + " must be an array.");
  }

  std::vector<std::string> items;
  std::string current;
  bool in_string = false;
  bool escaped = false;

  for (std::size_t index = 1; index + 1 < trimmed.size(); ++index) {
    const char ch = trimmed[index];
    if (escaped) {
      current.push_back(ch);
      escaped = false;
      continue;
    }
    if (in_string && ch == '\\') {
      current.push_back(ch);
      escaped = true;
      continue;
    }
    if (ch == '"') {
      in_string = !in_string;
      current.push_back(ch);
      continue;
    }
    if (!in_string && ch == ',') {
      const std::string token = TrimAscii(current);
      if (!token.empty()) {
        items.push_back(token);
      }
      current.clear();
      continue;
    }
    current.push_back(ch);
  }

  const std::string tail = TrimAscii(current);
  if (!tail.empty()) {
    items.push_back(tail);
  }
  if (in_string) {
    throw std::runtime_error(std::string(field) +
                             " has unclosed string token.");
  }
  return items;
}

auto ParseNumberArray(std::string_view value, std::string_view field)
    -> std::vector<double> {
  const std::vector<std::string> items = ParseArrayItems(value, field);
  if (items.empty()) {
    throw std::runtime_error(std::string(field) +
                             " must be a non-empty array.");
  }
  std::vector<double> numbers;
  numbers.reserve(items.size());
  for (std::size_t index = 0; index < items.size(); ++index) {
    const std::string token = TrimAscii(items[index]);
    std::size_t consumed = 0;
    const double value_number = std::stod(token, &consumed);
    if (consumed != token.size()) {
      throw std::runtime_error(std::string(field) + "[" +
                               std::to_string(index) +
                               "] must be a valid number.");
    }
    numbers.push_back(value_number);
  }
  return numbers;
}

auto ParseStringArray(std::string_view value, std::string_view field)
    -> std::vector<std::string> {
  const std::vector<std::string> items = ParseArrayItems(value, field);
  if (items.empty()) {
    throw std::runtime_error(std::string(field) +
                             " must be a non-empty array.");
  }
  std::vector<std::string> values;
  values.reserve(items.size());
  for (std::size_t index = 0; index < items.size(); ++index) {
    values.push_back(ParseQuotedString(
        items[index], std::string(field) + "[" + std::to_string(index) + "]"));
  }
  return values;
}

auto ReadTextFile(const fs::path &path) -> std::string {
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open heatmap config: " + path.string());
  }
  std::ostringstream stream;
  stream << file.rdbuf();
  if (!file.good() && !file.eof()) {
    throw std::runtime_error("Failed to read heatmap config: " + path.string());
  }
  return stream.str();
}

auto ValidateConfig(const HeatmapConfig &config, const fs::path &path) -> void {
  if (config.positive_hours.empty()) {
    throw std::runtime_error("Invalid heatmap config [" + path.string() +
                             "]: thresholds.positive_hours must not be empty.");
  }
  for (std::size_t index = 0; index < config.positive_hours.size(); ++index) {
    const double current = config.positive_hours[index];
    if (!(current > 0.0) || !std::isfinite(current)) {
      throw std::runtime_error("Invalid heatmap config [" + path.string() +
                               "]: thresholds.positive_hours values must be "
                               "positive finite numbers.");
    }
    if (index > 0 && current <= config.positive_hours[index - 1]) {
      throw std::runtime_error("Invalid heatmap config [" + path.string() +
                               "]: thresholds.positive_hours must be strictly "
                               "increasing.");
    }
  }

  if (config.palettes.empty()) {
    throw std::runtime_error("Invalid heatmap config [" + path.string() +
                             "]: palettes must contain at least one entry.");
  }
  const std::size_t expected_palette_size = config.positive_hours.size() + 1U;
  for (const auto &[name, colors] : config.palettes) {
    if (colors.size() != expected_palette_size) {
      throw std::runtime_error("Invalid heatmap config [" + path.string() +
                               "]: palette `" + name + "` must contain " +
                               std::to_string(expected_palette_size) +
                               " colors.");
    }
    for (std::size_t index = 0; index < colors.size(); ++index) {
      if (!IsHexColor(colors[index])) {
        throw std::runtime_error("Invalid heatmap config [" + path.string() +
                                 "]: palette `" + name + "` color index " +
                                 std::to_string(index) + " must be #RRGGBB.");
      }
    }
  }

  if (config.default_light_palette.empty() ||
      config.palettes.find(config.default_light_palette) ==
          config.palettes.end()) {
    throw std::runtime_error("Invalid heatmap config [" + path.string() +
                             "]: defaults.light_palette must reference an "
                             "existing palette.");
  }
  if (config.default_dark_palette.empty() ||
      config.palettes.find(config.default_dark_palette) ==
          config.palettes.end()) {
    throw std::runtime_error("Invalid heatmap config [" + path.string() +
                             "]: defaults.dark_palette must reference an "
                             "existing palette.");
  }
}

} // namespace

auto ResolveHeatmapConfigPath(const fs::path &executable_path) -> fs::path {
  std::vector<fs::path> candidates;
  if (!executable_path.empty()) {
    std::error_code ec;
    const fs::path abs_path = fs::absolute(executable_path, ec);
    if (!ec) {
      // Prefer the new charts folder, keep legacy path for compatibility.
      candidates.push_back(abs_path.parent_path() / "config" / "charts" /
                           "heatmap.toml");
      candidates.push_back(abs_path.parent_path() / "config" / "heatmap.toml");
    }
  }
  std::error_code cwd_ec;
  const fs::path cwd = fs::current_path(cwd_ec);
  if (!cwd_ec) {
    candidates.push_back(cwd / "config" / "charts" / "heatmap.toml");
    candidates.push_back(cwd / "config" / "heatmap.toml");
  }

  for (const auto &path : candidates) {
    std::error_code exists_ec;
    if (fs::exists(path, exists_ec) && !exists_ec) {
      return path;
    }
  }

  std::ostringstream message;
  message << "Unable to locate heatmap config file (charts/heatmap.toml). "
             "Checked:";
  for (const auto &candidate : candidates) {
    message << " " << candidate.string();
  }
  throw std::runtime_error(message.str());
}

auto LoadHeatmapConfig(const fs::path &config_path) -> HeatmapConfig {
  const std::string content = ReadTextFile(config_path);
  std::istringstream input(content);
  std::string raw_line;
  ParseSection section = ParseSection::kNone;

  HeatmapConfig config{};
  bool has_schema_version = false;
  std::size_t line_number = 0;

  while (std::getline(input, raw_line)) {
    ++line_number;
    const std::string line = StripInlineComment(raw_line);
    if (line.empty()) {
      continue;
    }

    if (line.front() == '[' && line.back() == ']') {
      const std::string section_name =
          TrimAscii(std::string_view(line).substr(1, line.size() - 2));
      if (section_name == "thresholds") {
        section = ParseSection::kThresholds;
      } else if (section_name == "defaults") {
        section = ParseSection::kDefaults;
      } else if (section_name == "palettes") {
        section = ParseSection::kPalettes;
      } else {
        section = ParseSection::kNone;
      }
      continue;
    }

    const std::size_t equal_pos = line.find('=');
    if (equal_pos == std::string::npos) {
      throw std::runtime_error(
          "Invalid heatmap config [" + config_path.string() + "] line " +
          std::to_string(line_number) + ": expected key=value.");
    }

    const std::string key = TrimAscii(line.substr(0, equal_pos));
    const std::string value = TrimAscii(line.substr(equal_pos + 1));
    if (key.empty()) {
      throw std::runtime_error("Invalid heatmap config [" +
                               config_path.string() + "] line " +
                               std::to_string(line_number) + ": empty key.");
    }

    if (section == ParseSection::kNone && key == "schema_version") {
      std::size_t consumed = 0;
      const int schema = std::stoi(value, &consumed);
      if (consumed != value.size() || schema != 1) {
        throw std::runtime_error("Invalid heatmap config [" +
                                 config_path.string() +
                                 "]: schema_version must be 1.");
      }
      has_schema_version = true;
      continue;
    }

    switch (section) {
    case ParseSection::kThresholds:
      if (key == "positive_hours") {
        config.positive_hours =
            ParseNumberArray(value, "thresholds.positive_hours");
      }
      break;
    case ParseSection::kDefaults:
      if (key == "light_palette") {
        config.default_light_palette =
            ParseQuotedString(value, "defaults.light_palette");
      } else if (key == "dark_palette") {
        config.default_dark_palette =
            ParseQuotedString(value, "defaults.dark_palette");
      }
      break;
    case ParseSection::kPalettes:
      config.palettes[key] = ParseStringArray(value, "palettes." + key);
      break;
    case ParseSection::kNone:
    default:
      break;
    }
  }

  if (!has_schema_version) {
    throw std::runtime_error("Invalid heatmap config [" + config_path.string() +
                             "]: missing schema_version.");
  }
  ValidateConfig(config, config_path);
  return config;
}

auto ResolvePaletteColors(const HeatmapConfig &config,
                          std::string_view palette_name)
    -> std::vector<std::string> {
  const auto found = config.palettes.find(std::string(palette_name));
  if (found == config.palettes.end()) {
    throw std::runtime_error("Unknown heatmap palette: `" +
                             std::string(palette_name) + "`.");
  }
  return found->second;
}

auto ListPaletteNames(const HeatmapConfig &config) -> std::vector<std::string> {
  std::vector<std::string> names;
  names.reserve(config.palettes.size());
  for (const auto &[name, _] : config.palettes) {
    names.push_back(name);
  }
  return names;
}

} // namespace tracer_core::cli::impl::commands::query::heatmap
