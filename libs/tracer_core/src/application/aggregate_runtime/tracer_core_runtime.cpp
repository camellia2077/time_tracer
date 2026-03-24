#include "application/aggregate_runtime/tracer_core_runtime.hpp"

#include <stdexcept>
#include <utility>

namespace tracer::core::application::use_cases {

TracerCoreRuntime::TracerCoreRuntime(
    std::shared_ptr<IPipelineApi> pipeline_api,
    std::shared_ptr<IQueryApi> query_api,
    std::shared_ptr<IReportApi> report_api,
    std::shared_ptr<ITracerExchangeApi> tracer_exchange_api)
    : pipeline_api_(std::move(pipeline_api)),
      query_api_(std::move(query_api)),
      report_api_(std::move(report_api)),
      tracer_exchange_api_(std::move(tracer_exchange_api)) {
  if (!pipeline_api_ || !query_api_ || !report_api_ || !tracer_exchange_api_) {
    throw std::invalid_argument(
        "TracerCoreRuntime capability APIs must not be null.");
  }
}

auto TracerCoreRuntime::pipeline() -> IPipelineApi& { return *pipeline_api_; }
auto TracerCoreRuntime::query() -> IQueryApi& { return *query_api_; }
auto TracerCoreRuntime::report() -> IReportApi& { return *report_api_; }
auto TracerCoreRuntime::tracer_exchange() -> ITracerExchangeApi& {
  return *tracer_exchange_api_;
}

}  // namespace tracer::core::application::use_cases
