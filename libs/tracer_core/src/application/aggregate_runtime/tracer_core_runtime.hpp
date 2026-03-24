#ifndef APPLICATION_AGGREGATE_RUNTIME_TRACER_CORE_RUNTIME_HPP_
#define APPLICATION_AGGREGATE_RUNTIME_TRACER_CORE_RUNTIME_HPP_

#include <memory>

#include "application/aggregate_runtime/i_tracer_core_runtime.hpp"
#include "application/use_cases/i_pipeline_api.hpp"
#include "application/use_cases/i_query_api.hpp"
#include "application/use_cases/i_report_api.hpp"
#include "application/use_cases/i_tracer_exchange_api.hpp"

namespace tracer::core::application::use_cases {

class TracerCoreRuntime final : public ITracerCoreRuntime {
 public:
  TracerCoreRuntime(std::shared_ptr<IPipelineApi> pipeline_api,
                    std::shared_ptr<IQueryApi> query_api,
                    std::shared_ptr<IReportApi> report_api,
                    std::shared_ptr<ITracerExchangeApi> tracer_exchange_api);

  auto pipeline() -> IPipelineApi& override;
  auto query() -> IQueryApi& override;
  auto report() -> IReportApi& override;
  auto tracer_exchange() -> ITracerExchangeApi& override;

 private:
  std::shared_ptr<IPipelineApi> pipeline_api_;
  std::shared_ptr<IQueryApi> query_api_;
  std::shared_ptr<IReportApi> report_api_;
  std::shared_ptr<ITracerExchangeApi> tracer_exchange_api_;
};

}  // namespace tracer::core::application::use_cases

using TracerCoreRuntime =
    tracer::core::application::use_cases::TracerCoreRuntime;

#endif  // APPLICATION_AGGREGATE_RUNTIME_TRACER_CORE_RUNTIME_HPP_
