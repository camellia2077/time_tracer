#ifndef APPLICATION_USE_CASES_I_TRACER_EXCHANGE_API_HPP_
#define APPLICATION_USE_CASES_I_TRACER_EXCHANGE_API_HPP_

#include "application/dto/exchange_requests.hpp"
#include "application/dto/exchange_responses.hpp"

namespace tracer::core::application::use_cases {

class ITracerExchangeApi {
 public:
  virtual ~ITracerExchangeApi() = default;

  virtual auto RunTracerExchangeExport(
      const tracer_core::core::dto::TracerExchangeExportRequest& request)
      -> tracer_core::core::dto::TracerExchangeExportResult = 0;

  virtual auto RunTracerExchangeImport(
      const tracer_core::core::dto::TracerExchangeImportRequest& request)
      -> tracer_core::core::dto::TracerExchangeImportResult = 0;

  virtual auto RunTracerExchangeUnpack(
      const tracer_core::core::dto::TracerExchangeUnpackRequest& request)
      -> tracer_core::core::dto::TracerExchangeUnpackResult = 0;

  virtual auto RunTracerExchangeInspect(
      const tracer_core::core::dto::TracerExchangeInspectRequest& request)
      -> tracer_core::core::dto::TracerExchangeInspectResult = 0;
};

}  // namespace tracer::core::application::use_cases

using ITracerExchangeApi =
    tracer::core::application::use_cases::ITracerExchangeApi;

#endif  // APPLICATION_USE_CASES_I_TRACER_EXCHANGE_API_HPP_
