// tracer_windows/src/bootstrap/cli_runtime_factory_proxy.hpp
#pragma once

#include <memory>

#include "application/use_cases/i_time_tracer_core_api.hpp"
#include "bootstrap/cli_runtime_factory_loader.hpp"

namespace time_tracer::cli::bootstrap::internal {

[[nodiscard]] auto MakeCoreApiProxy(std::shared_ptr<CoreLibrary> library,
                                    TtCoreRuntimeHandle *runtime_handle)
    -> std::shared_ptr<ITimeTracerCoreApi>;

} // namespace time_tracer::cli::bootstrap::internal
