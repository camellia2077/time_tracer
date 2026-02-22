// api/android/android_runtime_factory_internal.hpp
#ifndef API_ANDROID_ANDROID_RUNTIME_FACTORY_INTERNAL_H_
#define API_ANDROID_ANDROID_RUNTIME_FACTORY_INTERNAL_H_

#include <filesystem>
#include <optional>

#include "infrastructure/config/models/report_catalog.hpp"
#include "infrastructure/reports/facade/android_static_report_formatter_registrar.hpp"

namespace infrastructure::bootstrap::android_runtime_detail {

struct AndroidReportConfigPathSet {
  std::filesystem::path day;
  std::filesystem::path month;
  std::filesystem::path period;
  std::filesystem::path week;
  std::filesystem::path year;
};

struct AndroidRuntimeConfigPaths {
  std::filesystem::path converter_config_toml_path;
  AndroidReportConfigPathSet markdown;
  std::optional<AndroidReportConfigPathSet> latex;
  std::optional<AndroidReportConfigPathSet> typst;
  infrastructure::reports::AndroidStaticReportFormatterPolicy formatter_policy;
};

[[nodiscard]] auto ResolveOutputRoot(const std::filesystem::path& output_root)
    -> std::filesystem::path;
[[nodiscard]] auto ResolveDbPath(const std::filesystem::path& db_path,
                                 const std::filesystem::path& output_root)
    -> std::filesystem::path;
auto EnsureDatabaseBootstrapped(const std::filesystem::path& db_path) -> void;

[[nodiscard]] auto ResolveAndroidRuntimeConfigPaths(
    const std::filesystem::path& requested_converter_config_toml_path)
    -> AndroidRuntimeConfigPaths;

[[nodiscard]] auto BuildAndroidReportCatalog(
    const std::filesystem::path& output_root,
    const AndroidRuntimeConfigPaths& runtime_config_paths) -> ReportCatalog;

}  // namespace infrastructure::bootstrap::android_runtime_detail

#endif  // API_ANDROID_ANDROID_RUNTIME_FACTORY_INTERNAL_H_
