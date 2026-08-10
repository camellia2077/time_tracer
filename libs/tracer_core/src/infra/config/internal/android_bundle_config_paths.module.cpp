module;

#include "infra/config/internal/config_parser_utils_internal.hpp"

module tracer.core.infrastructure.config.internal.android_bundle_config_paths;

namespace config_parser_internal = ConfigParserUtils::internal;

namespace {

[[nodiscard]] auto ToAndroidBundleInsightsConfigPathSet(
    const config_parser_internal::AndroidBundleInsightsConfigPathSet& paths)
    -> tracer::core::infrastructure::config::internal::
        AndroidBundleInsightsConfigPathSet {
  return {
      .day = paths.day,
      .month = paths.month,
      .period = paths.period,
      .week = paths.week,
      .year = paths.year,
  };
}

[[nodiscard]] auto ToAndroidBundleConfigPaths(
    const config_parser_internal::AndroidBundleConfigPaths& paths) -> tracer::
    core::infrastructure::config::internal::AndroidBundleConfigPaths {
  return {
      .converter_config_toml_path = paths.converter_config_toml_path,
      .markdown = ToAndroidBundleInsightsConfigPathSet(paths.markdown),
      .latex =
          paths.latex.has_value()
              ? std::optional<tracer::core::infrastructure::config::internal::
                                  AndroidBundleInsightsConfigPathSet>(
                    ToAndroidBundleInsightsConfigPathSet(*paths.latex))
              : std::nullopt,
      .typst =
          paths.typst.has_value()
              ? std::optional<tracer::core::infrastructure::config::internal::
                                  AndroidBundleInsightsConfigPathSet>(
                    ToAndroidBundleInsightsConfigPathSet(*paths.typst))
              : std::nullopt,
  };
}

}  // namespace

namespace tracer::core::infrastructure::config::internal {

auto TryResolveAndroidBundleConfigPaths(const std::filesystem::path& config_dir)
    -> std::optional<AndroidBundleConfigPaths> {
  const auto bundle_paths =
      config_parser_internal::TryResolveAndroidBundleConfigPathsImpl(
          config_dir);
  if (!bundle_paths.has_value()) {
    return std::nullopt;
  }
  return ToAndroidBundleConfigPaths(*bundle_paths);
}

}  // namespace tracer::core::infrastructure::config::internal
