#include "application/ports/config/quick_access_toml_store.hpp"

#include <toml++/toml.h>

#include <cctype>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace tracer::core::application::config {
namespace {

constexpr std::string_view kQuickAccessKey = "quick_access";

[[nodiscard]] auto HasAsciiWhitespace(unsigned char value) -> bool {
  return std::isspace(value) != 0;
}

[[nodiscard]] auto HasSurroundingWhitespace(std::string_view value) -> bool {
  return !value.empty() &&
         (HasAsciiWhitespace(static_cast<unsigned char>(value.front())) ||
          HasAsciiWhitespace(static_cast<unsigned char>(value.back())));
}

auto ValidateAliases(const std::vector<std::string>& aliases) -> void {
  std::set<std::string> seen;
  for (const auto& alias : aliases) {
    if (alias.empty()) {
      throw std::invalid_argument(
          "quick_access aliases must not contain empty strings.");
    }
    if (HasSurroundingWhitespace(alias)) {
      throw std::invalid_argument(
          "quick_access aliases must not contain surrounding whitespace: " +
          alias);
    }
    if (!seen.insert(alias).second) {
      throw std::invalid_argument("quick_access aliases must be unique: " +
                                  alias);
    }
  }
}

auto ValidateTopLevelKeys(const toml::table& table) -> void {
  for (const auto& [key, value] : table) {
    static_cast<void>(value);
    if (key.str() != kQuickAccessKey) {
      throw std::invalid_argument(
          "Unsupported quick_access.toml top-level key: " +
          std::string(key.str()));
    }
  }
}

[[nodiscard]] auto ParseQuickAccessTable(const toml::table& table)
    -> QuickAccessConfig {
  ValidateTopLevelKeys(table);
  const toml::array* array = table.get_as<toml::array>(kQuickAccessKey);
  if (array == nullptr) {
    throw std::invalid_argument(
        "quick_access.toml must define `quick_access` as an array.");
  }

  QuickAccessConfig config;
  config.aliases.reserve(array->size());
  for (const toml::node& node : *array) {
    const auto alias = node.value<std::string>();
    if (!alias.has_value()) {
      throw std::invalid_argument(
          "quick_access must contain only string values.");
    }
    config.aliases.push_back(*alias);
  }
  ValidateAliases(config.aliases);
  return config;
}

[[nodiscard]] auto SerializeQuickAccessTable(const QuickAccessConfig& config)
    -> std::string {
  ValidateAliases(config.aliases);
  toml::table table;
  toml::array aliases;
  for (const auto& alias : config.aliases) {
    aliases.push_back(alias);
  }
  table.insert(std::string(kQuickAccessKey), std::move(aliases));

  std::ostringstream output;
  output << table;
  return output.str();
}

}  // namespace

auto ParseQuickAccessToml(std::string_view content)
    -> QuickAccessConfig {
  try {
    return ParseQuickAccessTable(
        toml::parse(content, std::string_view{"quick_access.toml"}));
  } catch (const toml::parse_error& error) {
    throw std::runtime_error("Quick Access TOML parse error: " +
                             std::string(error.description()));
  }
}

auto RenderQuickAccessToml(const QuickAccessConfig& config) -> std::string {
  return SerializeQuickAccessTable(config);
}

}  // namespace tracer::core::application::config
