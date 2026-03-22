module;

#include <filesystem>
#include <optional>
#include <string>

export module tracer.core.infrastructure.config.internal.cli_config_snapshot;

import tracer.core.domain.types.date_check_mode;

export namespace tracer::core::infrastructure::config::internal {

#include "infra/config/internal/detail/cli_config_snapshot_decl.inc"

}  // namespace tracer::core::infrastructure::config::internal

export namespace tracer::core::infrastructure::modconfig::internal {

using tracer::core::infrastructure::config::internal::CliCommandDefaultsSnapshot;
using tracer::core::infrastructure::config::internal::CliConfigSnapshot;
using tracer::core::infrastructure::config::internal::CliGlobalDefaultsSnapshot;
using tracer::core::infrastructure::config::internal::LoadCliConfigSnapshotCached;

}  // namespace tracer::core::infrastructure::modconfig::internal
