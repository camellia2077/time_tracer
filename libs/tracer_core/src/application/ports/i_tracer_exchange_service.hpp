#ifndef APPLICATION_PORTS_I_TRACER_EXCHANGE_SERVICE_HPP_
#define APPLICATION_PORTS_I_TRACER_EXCHANGE_SERVICE_HPP_

#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"

namespace tracer_core::application::ports {

class ITracerExchangeService {
 public:
  virtual ~ITracerExchangeService() = default;

  virtual auto RunExport(
      const tracer_core::core::dto::TracerExchangeExportRequest& request)
      -> tracer_core::core::dto::TracerExchangeExportResult = 0;

  virtual auto RunImport(
      const tracer_core::core::dto::TracerExchangeImportRequest& request)
      -> tracer_core::core::dto::TracerExchangeImportResult = 0;

  virtual auto RunInspect(
      const tracer_core::core::dto::TracerExchangeInspectRequest& request)
      -> tracer_core::core::dto::TracerExchangeInspectResult = 0;
};

}  // namespace tracer_core::application::ports

#endif  // APPLICATION_PORTS_I_TRACER_EXCHANGE_SERVICE_HPP_
