#ifndef APPLICATION_AGGREGATE_RUNTIME_I_TRACER_CORE_RUNTIME_HPP_
#define APPLICATION_AGGREGATE_RUNTIME_I_TRACER_CORE_RUNTIME_HPP_

#include "application/use_cases/i_pipeline_api.hpp"
#include "application/use_cases/i_query_api.hpp"
#include "application/use_cases/i_report_api.hpp"
#include "application/use_cases/i_tracer_exchange_api.hpp"

namespace tracer::core::application::use_cases {

class ITracerCoreRuntime {
 public:
  virtual ~ITracerCoreRuntime() = default;

  virtual auto pipeline() -> IPipelineApi& = 0;
  virtual auto query() -> IQueryApi& = 0;
  virtual auto report() -> IReportApi& = 0;
  virtual auto tracer_exchange() -> ITracerExchangeApi& = 0;
};

}  // namespace tracer::core::application::use_cases

using ITracerCoreRuntime =
    tracer::core::application::use_cases::ITracerCoreRuntime;

#endif  // APPLICATION_AGGREGATE_RUNTIME_I_TRACER_CORE_RUNTIME_HPP_
