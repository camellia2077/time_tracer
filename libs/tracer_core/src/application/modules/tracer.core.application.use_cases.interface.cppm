module;

#include "application/use_cases/i_pipeline_api.hpp"
#include "application/use_cases/i_query_api.hpp"
#include "application/use_cases/i_report_api.hpp"
#include "application/use_cases/i_tracer_exchange_api.hpp"
#include "application/use_cases/i_tracer_core_api.hpp"
#include "application/use_cases/i_tracer_core_runtime.hpp"

export module tracer.core.application.use_cases.interface;

export namespace tracer::core::application::use_cases {

using ::tracer::core::application::use_cases::IPipelineApi;
using ::tracer::core::application::use_cases::IQueryApi;
using ::tracer::core::application::use_cases::IReportApi;
using ::tracer::core::application::use_cases::ITracerExchangeApi;
using ::tracer::core::application::use_cases::ITracerCoreApi;
using ::tracer::core::application::use_cases::ITracerCoreRuntime;

}  // namespace tracer::core::application::use_cases

export namespace tracer::core::application::modusecases {

using ::tracer::core::application::use_cases::IPipelineApi;
using ::tracer::core::application::use_cases::IQueryApi;
using ::tracer::core::application::use_cases::IReportApi;
using ::tracer::core::application::use_cases::ITracerExchangeApi;
using ::tracer::core::application::use_cases::ITracerCoreApi;
using ::tracer::core::application::use_cases::ITracerCoreRuntime;

}  // namespace tracer::core::application::modusecases
