import tracer.core.infrastructure;
import tracer.core.infrastructure.config;

#include <toml++/toml.h>
#include <unordered_map>

#include "application/runtime_bridge/logger.hpp"
#include "domain/types/converter_config.hpp"
#include "infra/config/loader/alias_mapping_index_utils.hpp"
#include "infra/config/models/app_config.hpp"
#include "infra/tests/modules_smoke/config.hpp"
#include "infra/tests/modules_smoke/support.hpp"

namespace {

auto BuildRepoRoot() -> std::filesystem::path {
  return std::filesystem::path(__FILE__)
      .parent_path()   // modules_smoke
      .parent_path()   // tests
      .parent_path()   // infra
      .parent_path()   // tests
      .parent_path()   // tracer_core
      .parent_path()   // libs
      .parent_path();  // repo root
}

}  // namespace

auto RunInfrastructureModuleLoggingPlatformConfigSmoke() -> int {
  tracer::core::infrastructure::logging::ConsoleLogger logger;
  logger.Log(tracer_core::application::runtime_bridge::LogSeverity::kInfo,
             "phase6 infrastructure module smoke logger");

  tracer::core::infrastructure::logging::ConsoleDiagnosticsSink sink;
  (void)sink;

  tracer::core::infrastructure::logging::ValidationIssueReporter reporter;
  (void)reporter;

  tracer::core::infrastructure::logging::FileErrorReportWriter writer(
      "temp/phase6_infra_module_smoke_error.log");
  if (!writer.Append("phase6 infrastructure module smoke\n")) {
    return 1;
  }

  tracer::core::infrastructure::modplatform::WindowsPlatformClock windows_clock;
  const auto windows_date = windows_clock.TodayLocalDateIso();
  if (windows_date.size() != 10U) {
    return 2;
  }

  tracer::core::infrastructure::modplatform::AndroidPlatformClock android_clock;
  const auto android_date = android_clock.TodayLocalDateIso();
  if (android_date.size() != 10U) {
    return 3;
  }

  ConverterConfig config;
  tracer::core::infrastructure::config::StaticConverterConfigProvider provider(
      config);
  const auto loaded = provider.LoadConverterConfig();

  const auto load_converter_config = &tracer::core::infrastructure::config::
                                         ConverterConfigLoader::LoadFromFile;
  const auto load_daily_md_config = &tracer::core::infrastructure::config::
                                        InsightsConfigLoader::LoadDailyMdConfig;
  const auto read_toml =
      &tracer::core::infrastructure::config::loader::ReadToml;
  const auto load_detailed_insights =
      &tracer::core::infrastructure::config::internal::LoadDetailedInsights;
  const auto parse_system_settings =
      &tracer::core::infrastructure::config::internal::ParseSystemSettings;
  const auto load_configuration =
      &tracer::core::infrastructure::config::ConfigLoader::LoadConfiguration;
  const auto load_file_provider =
      &tracer::core::infrastructure::config::FileConverterConfigProvider::
          LoadConverterConfig;
  (void)load_converter_config;
  (void)load_daily_md_config;
  (void)read_toml;
  (void)load_detailed_insights;
  (void)parse_system_settings;
  (void)load_configuration;
  (void)load_file_provider;

  std::error_code cleanup_error;

  const std::filesystem::path kConfigSmokeDir =
      BuildRepoRoot() / "temp" / "phase4_config_infra_module_smoke";
  const std::filesystem::path kFakeExePath =
      kConfigSmokeDir / "bin" / "tracer_core_smoke.exe";
  const std::filesystem::path kCopiedConfigRoot =
      kFakeExePath.parent_path() / "config";
  const std::filesystem::path kSourceConfigRoot =
      BuildRepoRoot() / "config" / "program";
  const std::filesystem::path kSourceUserConfigRoot =
      BuildRepoRoot() / "config" / "user";
  const std::filesystem::path kSourceActivityHierarchyRoot =
      BuildRepoRoot() / "test" / "data" / "activity_hierarchy";
  std::filesystem::remove_all(kConfigSmokeDir, cleanup_error);
  std::filesystem::create_directories(kConfigSmokeDir);
  std::filesystem::create_directories(kCopiedConfigRoot);
  if (!std::filesystem::exists(kSourceConfigRoot)) {
    return 404;
  }
  if (!std::filesystem::exists(kSourceActivityHierarchyRoot)) {
    return 408;
  }
  std::filesystem::copy(kSourceConfigRoot, kCopiedConfigRoot / "program",
                        std::filesystem::copy_options::recursive);
  std::filesystem::copy(kSourceUserConfigRoot, kCopiedConfigRoot / "user",
                        std::filesystem::copy_options::recursive |
                            std::filesystem::copy_options::overwrite_existing);
  std::filesystem::copy(kSourceActivityHierarchyRoot,
                        kCopiedConfigRoot / "user" / "activity_hierarchy",
                        std::filesystem::copy_options::recursive |
                            std::filesystem::copy_options::overwrite_existing);
  WriteSmokeFile(kFakeExePath, "smoke");

  const std::filesystem::path kDailyMarkdownConfig =
      kCopiedConfigRoot / "program" / "insights" / "markdown" / "en" /
      "day.toml";
  const toml::table kDailyMarkdownTable =
      tracer::core::infrastructure::config::loader::ReadToml(
          kDailyMarkdownConfig);
  if (!kDailyMarkdownTable.contains("summary_section_label")) {
    return 400;
  }

  const auto kDailyMarkdown = tracer::core::infrastructure::config::
      InsightsConfigLoader::LoadDailyMdConfig(kDailyMarkdownConfig);
  if (kDailyMarkdown.labels.summary_section_label != "Summary") {
    return 401;
  }

  tracer::core::infrastructure::config::FileConverterConfigProvider
      file_provider(
          kCopiedConfigRoot / "user" / "behavior.toml",
          std::unordered_map<std::filesystem::path, std::filesystem::path>{});
  const ConverterConfig kLoadedFileConfig = file_provider.LoadConverterConfig();
  if (!kLoadedFileConfig.text_mapping.contains("wake")) {
    return 402;
  }
  if (kLoadedFileConfig.text_mapping.at("rest") != "rest_rest" ||
      kLoadedFileConfig.text_mapping.at("休息") != "rest_rest" ||
      kLoadedFileConfig.text_mapping.at("r") != "rest_rest" ||
      kLoadedFileConfig.text_mapping.at("吃饭") != "meal_dining" ||
      kLoadedFileConfig.text_mapping.at("有氧训练") != "exercise_cardio" ||
      kLoadedFileConfig.text_mapping.at("有氧") != "exercise_cardio") {
    return 404;
  }

  const std::filesystem::path kLegacyAliasDir =
      kConfigSmokeDir / "legacy_aliases";
  const std::filesystem::path kLegacyAliasFile =
      kLegacyAliasDir / "legacy.toml";
  WriteSmokeFile(kLegacyAliasFile,
                 "parent = \"legacy\"\n\n[canonical]\n"
                 "\"old-alias\" = \"activity\"\n");
  bool rejected_legacy_shape = false;
  try {
    static_cast<void>(
        tracer::core::infrastructure::config::loader::detail::
            LoadAliasMappingDefinition(
                kLegacyAliasDir,
                tracer::core::infrastructure::config::loader::ReadToml));
  } catch (const std::runtime_error&) {
    rejected_legacy_shape = true;
  }
  if (!rejected_legacy_shape) {
    return 405;
  }

  const std::filesystem::path kDuplicateGroupAliasDir =
      kConfigSmokeDir / "duplicate_group_aliases";
  WriteSmokeFile(kDuplicateGroupAliasDir / "duplicate.toml",
                 "parent = \"exercise\"\n\n[canonical.cardio]\n"
                 "group_aliases = [\"有氧训练\", \"有氧\"]\n\n"
                 "[canonical.other]\n"
                 "group_aliases = [\"有氧\"]\n");
  bool rejected_duplicate_group_alias = false;
  std::string duplicate_group_alias_error;
  try {
    static_cast<void>(
        tracer::core::infrastructure::config::loader::detail::
            LoadAliasMappingDefinition(
                kDuplicateGroupAliasDir,
                tracer::core::infrastructure::config::loader::ReadToml));
  } catch (const std::runtime_error& error) {
    rejected_duplicate_group_alias = true;
    duplicate_group_alias_error = error.what();
  }
  if (!rejected_duplicate_group_alias) {
    return 406;
  }
  if (duplicate_group_alias_error.find("duplicate.toml:7") ==
          std::string::npos ||
      duplicate_group_alias_error.find("group_aliases = [\"有氧\"]") ==
          std::string::npos ||
      duplicate_group_alias_error.find("first defined at") ==
          std::string::npos ||
      duplicate_group_alias_error.find("duplicate.toml:4") ==
          std::string::npos) {
    return 407;
  }

  tracer::core::infrastructure::config::ConfigLoader config_loader(
      kFakeExePath.string());
  const AppConfig kLoadedAppConfig = config_loader.LoadConfiguration();
  if (kLoadedAppConfig.pipeline.converter_main_config_path.filename() !=
          "behavior.toml" ||
      kLoadedAppConfig.loaded_insights.markdown.daily.labels
              .summary_section_label != "Summary") {
    return 403;
  }

  std::filesystem::remove_all(kConfigSmokeDir, cleanup_error);
  return 0;
}
