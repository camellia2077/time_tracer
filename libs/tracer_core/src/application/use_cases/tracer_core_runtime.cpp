#include "application/use_cases/tracer_core_runtime.hpp"

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

auto TracerCoreRuntime::RunConvert(
    const tracer_core::core::dto::ConvertRequest& request)
    -> tracer_core::core::dto::OperationAck {
  return pipeline().RunConvert(request);
}

auto TracerCoreRuntime::RunIngest(
    const tracer_core::core::dto::IngestRequest& request)
    -> tracer_core::core::dto::OperationAck {
  return pipeline().RunIngest(request);
}

auto TracerCoreRuntime::RunImport(
    const tracer_core::core::dto::ImportRequest& request)
    -> tracer_core::core::dto::OperationAck {
  return pipeline().RunImport(request);
}

auto TracerCoreRuntime::RunValidateStructure(
    const tracer_core::core::dto::ValidateStructureRequest& request)
    -> tracer_core::core::dto::OperationAck {
  return pipeline().RunValidateStructure(request);
}

auto TracerCoreRuntime::RunValidateLogic(
    const tracer_core::core::dto::ValidateLogicRequest& request)
    -> tracer_core::core::dto::OperationAck {
  return pipeline().RunValidateLogic(request);
}

auto TracerCoreRuntime::RunReportQuery(
    const tracer_core::core::dto::ReportQueryRequest& request)
    -> tracer_core::core::dto::TextOutput {
  return report().RunReportQuery(request);
}

auto TracerCoreRuntime::RunStructuredReportQuery(
    const tracer_core::core::dto::StructuredReportQueryRequest& request)
    -> tracer_core::core::dto::StructuredReportOutput {
  return report().RunStructuredReportQuery(request);
}

auto TracerCoreRuntime::RunPeriodBatchQuery(
    const tracer_core::core::dto::PeriodBatchQueryRequest& request)
    -> tracer_core::core::dto::TextOutput {
  return report().RunPeriodBatchQuery(request);
}

auto TracerCoreRuntime::RunStructuredPeriodBatchQuery(
    const tracer_core::core::dto::StructuredPeriodBatchQueryRequest& request)
    -> tracer_core::core::dto::StructuredPeriodBatchOutput {
  return report().RunStructuredPeriodBatchQuery(request);
}

auto TracerCoreRuntime::RunReportExport(
    const tracer_core::core::dto::ReportExportRequest& request)
    -> tracer_core::core::dto::OperationAck {
  return report().RunReportExport(request);
}

auto TracerCoreRuntime::RunDataQuery(
    const tracer_core::core::dto::DataQueryRequest& request)
    -> tracer_core::core::dto::TextOutput {
  return query().RunDataQuery(request);
}

auto TracerCoreRuntime::RunTreeQuery(
    const tracer_core::core::dto::TreeQueryRequest& request)
    -> tracer_core::core::dto::TreeQueryResponse {
  return query().RunTreeQuery(request);
}

auto TracerCoreRuntime::RunTracerExchangeExport(
    const tracer_core::core::dto::TracerExchangeExportRequest& request)
    -> tracer_core::core::dto::TracerExchangeExportResult {
  return tracer_exchange().RunTracerExchangeExport(request);
}

auto TracerCoreRuntime::RunTracerExchangeImport(
    const tracer_core::core::dto::TracerExchangeImportRequest& request)
    -> tracer_core::core::dto::TracerExchangeImportResult {
  return tracer_exchange().RunTracerExchangeImport(request);
}

auto TracerCoreRuntime::RunTracerExchangeInspect(
    const tracer_core::core::dto::TracerExchangeInspectRequest& request)
    -> tracer_core::core::dto::TracerExchangeInspectResult {
  return tracer_exchange().RunTracerExchangeInspect(request);
}

}  // namespace tracer::core::application::use_cases
