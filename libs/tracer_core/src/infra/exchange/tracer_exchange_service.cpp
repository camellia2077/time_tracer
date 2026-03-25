#include "infra/exchange/tracer_exchange_service.hpp"

#include <memory>

#include "infra/exchange/tracer_exchange_service_internal.hpp"

namespace tracer_core::infrastructure::crypto {

auto CreateTracerExchangeService(
    tracer::core::application::workflow::IWorkflowHandler& workflow_handler)
    -> std::shared_ptr<
        tracer_core::application::ports::ITracerExchangeService> {
  return std::make_shared<tracer_exchange_internal::TracerExchangeService>(
      workflow_handler);
}

}  // namespace tracer_core::infrastructure::crypto
