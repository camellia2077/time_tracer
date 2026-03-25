module;

#include <filesystem>

namespace tracer::core::infrastructure::config::internal {

struct CliConfigSnapshot;

}  // namespace tracer::core::infrastructure::config::internal

namespace CliConfigSnapshotBridge::internal {

auto LoadCliConfigSnapshotCachedImpl(
    const std::filesystem::path& executable_path)
    -> tracer::core::infrastructure::config::internal::CliConfigSnapshot;

}  // namespace CliConfigSnapshotBridge::internal

module tracer.core.infrastructure.config.internal.cli_config_snapshot;

namespace bridge_internal = CliConfigSnapshotBridge::internal;

namespace tracer::core::infrastructure::config::internal {

auto LoadCliConfigSnapshotCached(const std::filesystem::path& executable_path)
    -> CliConfigSnapshot {
  return bridge_internal::LoadCliConfigSnapshotCachedImpl(executable_path);
}

}  // namespace tracer::core::infrastructure::config::internal
