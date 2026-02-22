// tracer_windows/src/bootstrap/cli_runtime_factory_support.hpp
#pragma once

#include <memory>

#include "bootstrap/cli_runtime_factory_check.hpp"
#include "bootstrap/cli_runtime_factory_loader.hpp"
#include "bootstrap/cli_runtime_factory_proxy.hpp"

namespace time_tracer::cli::bootstrap::internal {

struct CliRuntimeState {
  std::shared_ptr<CoreLibrary> library;
  std::shared_ptr<ITimeTracerCoreApi> core_api;
};

} // namespace time_tracer::cli::bootstrap::internal
