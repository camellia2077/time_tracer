// infrastructure/config/internal/config_parser_field_readers.cpp
#include <stdexcept>

#include "infrastructure/config/internal/config_parser_utils_internal.hpp"

namespace ConfigParserUtils::internal {

auto JoinFieldPath(std::string_view prefix, std::string_view key)
    -> std::string {
  if (prefix.empty()) {
    return std::string(key);
  }
  return std::string(prefix) + "." + std::string(key);
}

[[noreturn]] auto ThrowConfigFieldError(const fs::path& source_path,
                                        const std::string& field_path,
                                        const std::string& message) -> void {
  throw std::runtime_error("Invalid config [" + source_path.string() +
                           "] field '" + field_path + "': " + message);
}

auto TryReadTableField(const toml::table& tbl, std::string_view key,
                       const fs::path& source_path,
                       std::string_view field_prefix) -> const toml::table* {
  const toml::node* node = tbl.get(key);
  if (node == nullptr) {
    return nullptr;
  }

  const toml::table* table = node->as_table();
  if (table == nullptr) {
    ThrowConfigFieldError(source_path, JoinFieldPath(field_prefix, key),
                          "must be a table.");
  }
  return table;
}

auto RequireNonEmptyStringField(const toml::table& tbl, std::string_view key,
                                const fs::path& source_path,
                                std::string_view field_prefix) -> std::string {
  const auto value = RequireTypedField<std::string>(tbl, key, source_path,
                                                    field_prefix, "a string");
  if (value.empty()) {
    ThrowConfigFieldError(source_path, JoinFieldPath(field_prefix, key),
                          "must be a non-empty string.");
  }
  return value;
}

auto EnsureFieldAbsent(const toml::table& tbl, std::string_view key,
                       const fs::path& source_path,
                       std::string_view field_prefix,
                       std::string_view replacement_hint) -> void {
  if (tbl.get(key) == nullptr) {
    return;
  }
  ThrowConfigFieldError(
      source_path, JoinFieldPath(field_prefix, key),
      "is no longer supported. Use '" + std::string(replacement_hint) + "'.");
}

}  // namespace ConfigParserUtils::internal
