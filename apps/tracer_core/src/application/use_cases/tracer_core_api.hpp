// application/use_cases/tracer_core_api.hpp
#ifndef APPLICATION_USE_CASES_TRACER_CORE_API_H_
#define APPLICATION_USE_CASES_TRACER_CORE_API_H_

#include <filesystem>
#include <memory>

#include "application/use_cases/i_tracer_core_api.hpp"

class IReportHandler;
class IProjectRepository;
class IWorkflowHandler;
namespace tracer_core::application::ports {
class IDataQueryService;
class IReportDataQueryService;
class IReportDtoFormatter;
class IReportExportWriter;
}  // namespace tracer_core::application::ports

class TracerCoreApi : public ITracerCoreApi {
 public:
  TracerCoreApi(
      IWorkflowHandler& workflow_handler, IReportHandler& report_handler,
      std::shared_ptr<IProjectRepository> project_repository,
      std::shared_ptr<tracer_core::application::ports::IDataQueryService>
          data_query_service,
      std::shared_ptr<tracer_core::application::ports::IReportDataQueryService>
          report_data_query_service = nullptr,
      std::shared_ptr<tracer_core::application::ports::IReportDtoFormatter>
          report_dto_formatter = nullptr,
      std::shared_ptr<tracer_core::application::ports::IReportExportWriter>
          report_export_writer = nullptr);

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

 private:
  IWorkflowHandler& workflow_handler_;
  IReportHandler& report_handler_;
  std::shared_ptr<IProjectRepository> project_repository_;
  std::shared_ptr<tracer_core::application::ports::IDataQueryService>
      data_query_service_;
  std::shared_ptr<tracer_core::application::ports::IReportDataQueryService>
      kReport_;
  std::shared_ptr<tracer_core::application::ports::IReportDtoFormatter>
      report_dto_formatter_;
  std::shared_ptr<tracer_core::application::ports::IReportExportWriter>
      report_export_writer_;
};

#endif  // APPLICATION_USE_CASES_TRACER_CORE_API_H_
