// bootstrap/cli_runtime_factory_support.hpp
#pragma once

#include <memory>

#include "bootstrap/cli_runtime_factory_check.hpp"
#include "bootstrap/cli_runtime_factory_loader.hpp"
#include "bootstrap/cli_runtime_factory_proxy.hpp"

namespace tracer_core::cli::bootstrap::internal {

struct CliRuntimeState {
  std::shared_ptr<CoreLibrary> library;
  std::shared_ptr<ITracerCoreApi> core_api;
};

} // namespace tracer_core::cli::bootstrap::internal
