// host/bootstrap/android_runtime_factory_internal.hpp
#ifndef API_ANDROID_ANDROID_RUNTIME_FACTORY_INTERNAL_H_
#define API_ANDROID_ANDROID_RUNTIME_FACTORY_INTERNAL_H_

#include <filesystem>
#include <optional>

#include "infra/config/models/insights_catalog.hpp"
#include "infra/insights/facade/android_static_insights_formatter_registrar.hpp"

namespace infrastructure::bootstrap::android_runtime_detail {

struct AndroidInsightsConfigPathSet {
  std::filesystem::path daily;
  std::filesystem::path month;
  std::filesystem::path period;
  std::filesystem::path week;
  std::filesystem::path year;
};

struct AndroidRuntimeConfigPaths {
  std::filesystem::path converter_config_toml_path;
  AndroidInsightsConfigPathSet markdown;
  std::optional<AndroidInsightsConfigPathSet> latex;
  std::optional<AndroidInsightsConfigPathSet> typst;
  infrastructure::insights::AndroidStaticInsightsFormatterPolicy
      formatter_policy;
};

[[nodiscard]] auto ResolveOutputRoot(const std::filesystem::path& output_root)
    -> std::filesystem::path;
[[nodiscard]] auto ResolveDbPath(const std::filesystem::path& db_path,
                                 const std::filesystem::path& output_root)
    -> std::filesystem::path;

[[nodiscard]] auto ResolveAndroidRuntimeConfigPaths(
    const std::filesystem::path& requested_converter_config_toml_path)
    -> AndroidRuntimeConfigPaths;

[[nodiscard]] auto ResolveAndroidPipelineConfigPath(
    const std::filesystem::path& requested_converter_config_toml_path)
    -> std::filesystem::path;

[[nodiscard]] auto BuildAndroidInsightsCatalog(
    const std::filesystem::path& output_root,
    const AndroidRuntimeConfigPaths& runtime_config_paths) -> InsightsCatalog;

}  // namespace infrastructure::bootstrap::android_runtime_detail

#endif  // API_ANDROID_ANDROID_RUNTIME_FACTORY_INTERNAL_H_
