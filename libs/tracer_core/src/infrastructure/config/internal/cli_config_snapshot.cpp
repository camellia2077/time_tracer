#include <filesystem>
#include <mutex>
#include <optional>
#include <string>
#include <system_error>

import tracer.core.infrastructure.config.config_loader;
import tracer.core.domain.types.date_check_mode;

using ModAppConfig = tracer::core::infrastructure::modconfig::AppConfig;

namespace tracer::core::infrastructure::config::internal {

#include "infrastructure/config/internal/detail/cli_config_snapshot_decl.inc"

}  // namespace tracer::core::infrastructure::config::internal

namespace infra_config_internal = tracer::core::infrastructure::config::internal;

namespace {

namespace fs = std::filesystem;
using ConfigLoader = tracer::core::infrastructure::config::ConfigLoader;

struct CliConfigSnapshotCacheEntry {
  fs::path normalized_executable_path;
  infra_config_internal::CliConfigSnapshot snapshot;
};

std::mutex g_cli_config_snapshot_cache_mutex;
std::optional<CliConfigSnapshotCacheEntry> g_cli_config_snapshot_cache{};

[[nodiscard]] auto NormalizeExecutablePath(const fs::path& executable_path)
    -> fs::path {
  std::error_code error;
  const fs::path kCanonical = fs::weakly_canonical(executable_path, error);
  if (!error) {
    return kCanonical;
  }
  return fs::absolute(executable_path);
}

[[nodiscard]] auto ToCliConfigSnapshot(const ModAppConfig& app_config)
    -> infra_config_internal::CliConfigSnapshot {
  return {
      .exe_dir_path = app_config.exe_dir_path,
      .export_path = app_config.kExportPath,
      .converter_config_toml_path =
          app_config.pipeline.interval_processor_config_path,
      .default_save_processed_output = app_config.default_save_processed_output,
      .default_date_check_mode = app_config.default_date_check_mode,
      .defaults =
          {
              .db_path = app_config.defaults.kDbPath,
              .output_root = app_config.defaults.output_root,
              .default_format = app_config.defaults.default_format,
          },
      .command_defaults =
          {
              .export_format = app_config.command_defaults.export_format,
              .query_format = app_config.command_defaults.query_format,
              .convert_date_check_mode =
                  app_config.command_defaults.convert_date_check_mode,
              .convert_save_processed_output =
                  app_config.command_defaults.convert_save_processed_output,
              .convert_validate_logic =
                  app_config.command_defaults.convert_validate_logic,
              .convert_validate_structure =
                  app_config.command_defaults.convert_validate_structure,
              .ingest_date_check_mode =
                  app_config.command_defaults.ingest_date_check_mode,
              .ingest_save_processed_output =
                  app_config.command_defaults.ingest_save_processed_output,
              .validate_logic_date_check_mode =
                  app_config.command_defaults.validate_logic_date_check_mode,
          },
  };
}

}  // namespace

namespace CliConfigSnapshotBridge::internal {

auto LoadCliConfigSnapshotCachedImpl(const std::filesystem::path& executable_path)
    -> infra_config_internal::CliConfigSnapshot {
  const fs::path kNormalizedPath = NormalizeExecutablePath(executable_path);
  {
    std::scoped_lock lock(g_cli_config_snapshot_cache_mutex);
    if (g_cli_config_snapshot_cache.has_value() &&
        g_cli_config_snapshot_cache->normalized_executable_path ==
            kNormalizedPath) {
      return g_cli_config_snapshot_cache->snapshot;
    }
  }

  ConfigLoader config_loader(kNormalizedPath.string());
  const ModAppConfig kAppConfig = config_loader.LoadConfiguration();
  infra_config_internal::CliConfigSnapshot snapshot =
      ToCliConfigSnapshot(kAppConfig);

  {
    std::scoped_lock lock(g_cli_config_snapshot_cache_mutex);
    g_cli_config_snapshot_cache = CliConfigSnapshotCacheEntry{
        .normalized_executable_path = kNormalizedPath,
        .snapshot = snapshot,
    };
  }

  return snapshot;
}

}  // namespace CliConfigSnapshotBridge::internal

namespace bridge_internal = CliConfigSnapshotBridge::internal;

namespace tracer::core::infrastructure::config::internal {

auto LoadCliConfigSnapshotCached(const std::filesystem::path& executable_path)
    -> CliConfigSnapshot {
  return bridge_internal::LoadCliConfigSnapshotCachedImpl(executable_path);
}

}  // namespace tracer::core::infrastructure::config::internal
