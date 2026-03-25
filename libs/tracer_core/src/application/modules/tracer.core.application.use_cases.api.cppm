module;

#include "application/use_cases/pipeline_api.hpp"
#include "application/use_cases/query_api.hpp"
#include "application/use_cases/report_api.hpp"
#include "application/use_cases/tracer_exchange_api.hpp"
#include "application/aggregate_runtime/tracer_core_runtime.hpp"

export module tracer.core.application.use_cases.api;

export import tracer.core.application.use_cases.interface;

export namespace tracer::core::application::use_cases {

using ::tracer::core::application::use_cases::PipelineApi;
using ::tracer::core::application::use_cases::QueryApi;
using ::tracer::core::application::use_cases::ReportApi;
using ::tracer::core::application::use_cases::TracerCoreRuntime;
using ::tracer::core::application::use_cases::TracerExchangeApi;

}  // namespace tracer::core::application::use_cases

export namespace tracer::core::application::modusecases {

using ::tracer::core::application::use_cases::PipelineApi;
using ::tracer::core::application::use_cases::QueryApi;
using ::tracer::core::application::use_cases::ReportApi;
using ::tracer::core::application::use_cases::TracerCoreRuntime;
using ::tracer::core::application::use_cases::TracerExchangeApi;

}  // namespace tracer::core::application::modusecases
