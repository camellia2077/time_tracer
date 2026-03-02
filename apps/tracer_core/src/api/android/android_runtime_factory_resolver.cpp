// api/android/android_runtime_factory_resolver.cpp
#include <toml++/toml.h>

#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "api/android/android_runtime_factory_internal.hpp"
#include "infrastructure/config/internal/config_parser_utils.hpp"
#include "infrastructure/persistence/importer/sqlite/connection.hpp"

#ifndef TT_REPORT_ENABLE_LATEX
#define TT_REPORT_ENABLE_LATEX 1
#endif

#ifndef TT_REPORT_ENABLE_TYPST
#define TT_REPORT_ENABLE_TYPST 1
#endif

namespace infrastructure::bootstrap::android_runtime_detail {

#include "api/android/internal/android_runtime_factory_resolver_namespace.inc"


auto ResolveOutputRoot(const fs::path& output_root) -> fs::path {
  if (output_root.empty()) {
    throw std::invalid_argument(
        "Android runtime output_root must not be empty.");
  }
  return fs::absolute(output_root);
}

auto ResolveDbPath(const fs::path& db_path, const fs::path& output_root)
    -> fs::path {
  if (!db_path.empty()) {
    return fs::absolute(db_path);
  }
  return fs::absolute(output_root / "db" / kDatabaseFilename);
}

auto EnsureDatabaseBootstrapped(const fs::path& db_path) -> void {
  fs::create_directories(db_path.parent_path());

  infrastructure::persistence::importer::sqlite::Connection bootstrap_db(
      db_path.string());
  if (bootstrap_db.GetDb() == nullptr) {
    throw std::runtime_error("Failed to initialize database at: " +
                             db_path.string());
  }
}

auto ResolveAndroidRuntimeConfigPaths(
    const fs::path& requested_converter_config_toml_path)
    -> AndroidRuntimeConfigPaths {
  const fs::path requested_converter_config_path = RequireNonEmptyPath(
      requested_converter_config_toml_path, "converter_config_toml_path");
  const fs::path config_root =
      ResolveReportConfigRoot(requested_converter_config_path);

  const bool has_bundle = ValidateAndroidBundlePolicy(config_root);
  if (has_bundle) {
    AppConfig bundle_config;
    const bool bundle_loaded =
        ConfigParserUtils::TryParseBundlePaths(config_root, bundle_config);
    if (!bundle_loaded) {
      throw std::runtime_error(
          "Bundle path config disappeared during Android runtime bootstrap: " +
          ConfigParserUtils::ResolveBundlePath(config_root).string());
    }
    const std::optional<AndroidReportConfigPathSet> latex_paths =
        BuildBundleLatexConfigPaths(bundle_config);
    const std::optional<AndroidReportConfigPathSet> typst_paths =
        BuildBundleTypstConfigPaths(bundle_config);

    return {
        .converter_config_toml_path = ResolveConverterConfigTomlPath(
            bundle_config.pipeline.interval_processor_config_path),
        .markdown = BuildBundleMarkdownConfigPaths(bundle_config),
        .latex = latex_paths,
        .typst = typst_paths,
        .formatter_policy = {.enable_markdown = true,
                             .enable_latex = latex_paths.has_value(),
                             .enable_typst = typst_paths.has_value()},
    };
  }

  return {
      .converter_config_toml_path =
          ResolveConverterConfigTomlPath(requested_converter_config_path),
      .markdown = BuildLegacyMarkdownConfigPaths(config_root),
      .latex = std::nullopt,
      .typst = std::nullopt,
      .formatter_policy = infrastructure::reports::
          AndroidStaticReportFormatterPolicy::MarkdownOnly(),
  };
}

}  // namespace infrastructure::bootstrap::android_runtime_detail
