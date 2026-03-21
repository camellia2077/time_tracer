#ifndef APPLICATION_USE_CASES_TRACER_EXCHANGE_API_HPP_
#define APPLICATION_USE_CASES_TRACER_EXCHANGE_API_HPP_

#include <memory>

#include "application/ports/i_tracer_exchange_service.hpp"
#include "application/use_cases/i_tracer_exchange_api.hpp"

namespace tracer::core::application::use_cases {

class TracerExchangeApi final : public ITracerExchangeApi {
 public:
  using TracerExchangeServicePtr =
      std::shared_ptr<tracer_core::application::ports::ITracerExchangeService>;

  explicit TracerExchangeApi(
      TracerExchangeServicePtr tracer_exchange_service = nullptr);

  auto RunTracerExchangeExport(
      const tracer_core::core::dto::TracerExchangeExportRequest& request)
      -> tracer_core::core::dto::TracerExchangeExportResult override;

  auto RunTracerExchangeImport(
      const tracer_core::core::dto::TracerExchangeImportRequest& request)
      -> tracer_core::core::dto::TracerExchangeImportResult override;

  auto RunTracerExchangeInspect(
      const tracer_core::core::dto::TracerExchangeInspectRequest& request)
      -> tracer_core::core::dto::TracerExchangeInspectResult override;

 private:
  TracerExchangeServicePtr tracer_exchange_service_;
};

}  // namespace tracer::core::application::use_cases

using TracerExchangeApi =
    tracer::core::application::use_cases::TracerExchangeApi;

#endif  // APPLICATION_USE_CASES_TRACER_EXCHANGE_API_HPP_
