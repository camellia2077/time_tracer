// infrastructure/config/internal/config_parser_path_utils.cpp
#include "infrastructure/config/internal/config_parser_utils_internal.hpp"

namespace ConfigParserUtils::internal {

auto ResolveDefaultPath(const fs::path& exe_path, const std::string& path_value)
    -> fs::path {
  fs::path path = path_value;
  if (path.is_relative()) {
    return exe_path / path;
  }
  return path;
}

auto EnsureFileExists(const fs::path& source_path,
                      const std::string& field_path, const fs::path& file_path)
    -> void {
  if (file_path.empty()) {
    ThrowConfigFieldError(source_path, field_path, "must not be empty.");
  }

  std::error_code error;
  const bool exists = fs::exists(file_path, error);
  if (error) {
    ThrowConfigFieldError(source_path, field_path,
                          "failed to check file existence for '" +
                              file_path.string() + "': " + error.message());
  }
  if (!exists) {
    ThrowConfigFieldError(
        source_path, field_path,
        "referenced file does not exist: " + file_path.string());
  }

  const bool is_regular_file = fs::is_regular_file(file_path, error);
  if (error) {
    ThrowConfigFieldError(source_path, field_path,
                          "failed to check file type for '" +
                              file_path.string() + "': " + error.message());
  }
  if (!is_regular_file) {
    ThrowConfigFieldError(
        source_path, field_path,
        "referenced path is not a file: " + file_path.string());
  }
}

auto NormalizeConfigRelativePath(const fs::path& config_dir,
                                 std::string raw_path) -> fs::path {
  for (char& character : raw_path) {
    if (character == '\\') {
      character = '/';
    }
  }

  while (raw_path.starts_with("./")) {
    raw_path.erase(0, 2);
  }

  const fs::path parsed(raw_path);
  if (parsed.is_absolute()) {
    return parsed.lexically_normal();
  }
  return (config_dir / parsed).lexically_normal();
}

auto ResolveBundlePathImpl(const fs::path& config_dir) -> fs::path {
  return config_dir / "meta" / "bundle.toml";
}

}  // namespace ConfigParserUtils::internal
