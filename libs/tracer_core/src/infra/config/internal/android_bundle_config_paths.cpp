#include <filesystem>
#include <optional>

#include "infra/config/internal/config_parser_utils_internal.hpp"

namespace tracer::core::infrastructure::config::internal {

#include "infra/config/internal/detail/android_bundle_config_paths_decl.inc"

}  // namespace tracer::core::infrastructure::config::internal

namespace config_parser_internal = ConfigParserUtils::internal;

namespace tracer::core::infrastructure::config::internal {

namespace {

[[nodiscard]] auto ToAndroidBundleInsightsConfigPathSet(
    const config_parser_internal::AndroidBundleInsightsConfigPathSet& paths)
    -> AndroidBundleInsightsConfigPathSet {
  return {
      .day = paths.day,
      .month = paths.month,
      .period = paths.period,
      .week = paths.week,
      .year = paths.year,
  };
}

[[nodiscard]] auto ToAndroidBundleConfigPaths(
    const config_parser_internal::AndroidBundleConfigPaths& paths)
    -> AndroidBundleConfigPaths {
  return {
      .converter_config_toml_path = paths.converter_config_toml_path,
      .markdown = ToAndroidBundleInsightsConfigPathSet(paths.markdown),
      .latex = paths.latex.has_value()
                   ? std::optional<AndroidBundleInsightsConfigPathSet>(
                         ToAndroidBundleInsightsConfigPathSet(*paths.latex))
                   : std::nullopt,
      .typst = paths.typst.has_value()
                   ? std::optional<AndroidBundleInsightsConfigPathSet>(
                         ToAndroidBundleInsightsConfigPathSet(*paths.typst))
                   : std::nullopt,
  };
}

}  // namespace

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
