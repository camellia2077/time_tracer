#ifndef APPLICATION_USE_CASES_TRACER_CORE_RUNTIME_HPP_
#define APPLICATION_USE_CASES_TRACER_CORE_RUNTIME_HPP_

#include <memory>

#include "application/use_cases/i_pipeline_api.hpp"
#include "application/use_cases/i_query_api.hpp"
#include "application/use_cases/i_report_api.hpp"
#include "application/use_cases/i_tracer_core_api.hpp"
#include "application/use_cases/i_tracer_core_runtime.hpp"
#include "application/use_cases/i_tracer_exchange_api.hpp"

namespace tracer::core::application::use_cases {

class TracerCoreRuntime : public ITracerCoreRuntime, public ITracerCoreApi {
 public:
  TracerCoreRuntime(std::shared_ptr<IPipelineApi> pipeline_api,
                    std::shared_ptr<IQueryApi> query_api,
                    std::shared_ptr<IReportApi> report_api,
                    std::shared_ptr<ITracerExchangeApi> tracer_exchange_api);

  auto pipeline() -> IPipelineApi& override;
  auto query() -> IQueryApi& override;
  auto report() -> IReportApi& override;
  auto tracer_exchange() -> ITracerExchangeApi& override;

  auto RunConvert(const tracer_core::core::dto::ConvertRequest& request)
      -> tracer_core::core::dto::OperationAck override;
  auto RunIngest(const tracer_core::core::dto::IngestRequest& request)
      -> tracer_core::core::dto::OperationAck override;
  auto RunImport(const tracer_core::core::dto::ImportRequest& request)
      -> tracer_core::core::dto::OperationAck override;
  auto RunValidateStructure(
      const tracer_core::core::dto::ValidateStructureRequest& request)
      -> tracer_core::core::dto::OperationAck override;
  auto RunValidateLogic(
      const tracer_core::core::dto::ValidateLogicRequest& request)
      -> tracer_core::core::dto::OperationAck override;
  auto RunReportQuery(const tracer_core::core::dto::ReportQueryRequest& request)
      -> tracer_core::core::dto::TextOutput override;
  auto RunStructuredReportQuery(
      const tracer_core::core::dto::StructuredReportQueryRequest& request)
      -> tracer_core::core::dto::StructuredReportOutput override;
  auto RunPeriodBatchQuery(
      const tracer_core::core::dto::PeriodBatchQueryRequest& request)
      -> tracer_core::core::dto::TextOutput override;
  auto RunStructuredPeriodBatchQuery(
      const tracer_core::core::dto::StructuredPeriodBatchQueryRequest& request)
      -> tracer_core::core::dto::StructuredPeriodBatchOutput override;
  auto RunReportExport(
      const tracer_core::core::dto::ReportExportRequest& request)
      -> tracer_core::core::dto::OperationAck override;
  auto RunDataQuery(const tracer_core::core::dto::DataQueryRequest& request)
      -> tracer_core::core::dto::TextOutput override;
  auto RunTreeQuery(const tracer_core::core::dto::TreeQueryRequest& request)
      -> tracer_core::core::dto::TreeQueryResponse override;
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
  std::shared_ptr<IPipelineApi> pipeline_api_;
  std::shared_ptr<IQueryApi> query_api_;
  std::shared_ptr<IReportApi> report_api_;
  std::shared_ptr<ITracerExchangeApi> tracer_exchange_api_;
};

}  // namespace tracer::core::application::use_cases

using TracerCoreRuntime =
    tracer::core::application::use_cases::TracerCoreRuntime;

#endif  // APPLICATION_USE_CASES_TRACER_CORE_RUNTIME_HPP_
