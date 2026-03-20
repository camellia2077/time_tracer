#ifndef INFRASTRUCTURE_CRYPTO_TRACER_EXCHANGE_SERVICE_HPP_
#define INFRASTRUCTURE_CRYPTO_TRACER_EXCHANGE_SERVICE_HPP_

#include <memory>

namespace tracer::core::application::workflow {

class IWorkflowHandler;

}  // namespace tracer::core::application::workflow

namespace tracer_core::application::ports {

class ITracerExchangeService;

}  // namespace tracer_core::application::ports

namespace tracer_core::infrastructure::crypto {

[[nodiscard]] auto CreateTracerExchangeService(
    tracer::core::application::workflow::IWorkflowHandler& workflow_handler)
    -> std::shared_ptr<tracer_core::application::ports::ITracerExchangeService>;

}  // namespace tracer_core::infrastructure::crypto

#endif  // INFRASTRUCTURE_CRYPTO_TRACER_EXCHANGE_SERVICE_HPP_
