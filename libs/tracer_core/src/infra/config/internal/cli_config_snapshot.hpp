// infra/config/internal/cli_config_snapshot.hpp
#ifndef INFRASTRUCTURE_CONFIG_INTERNAL_CLI_CONFIG_SNAPSHOT_H_
#define INFRASTRUCTURE_CONFIG_INTERNAL_CLI_CONFIG_SNAPSHOT_H_

#include <filesystem>
#include <optional>
#include <string>

#include "domain/types/date_check_mode.hpp"

namespace tracer::core::infrastructure::config::internal {

#include "infra/config/internal/detail/cli_config_snapshot_decl.inc"

}  // namespace tracer::core::infrastructure::config::internal

namespace tracer::core::infrastructure::modconfig::internal {

using tracer::core::infrastructure::config::internal::
    CliCommandDefaultsSnapshot;
using tracer::core::infrastructure::config::internal::CliConfigSnapshot;
using tracer::core::infrastructure::config::internal::CliGlobalDefaultsSnapshot;
using tracer::core::infrastructure::config::internal::
    LoadCliConfigSnapshotCached;

}  // namespace tracer::core::infrastructure::modconfig::internal

#endif  // INFRASTRUCTURE_CONFIG_INTERNAL_CLI_CONFIG_SNAPSHOT_H_
