// application/use_cases/time_tracer_core_api.hpp
#ifndef APPLICATION_USE_CASES_TIME_TRACER_CORE_API_H_
#define APPLICATION_USE_CASES_TIME_TRACER_CORE_API_H_

#include <filesystem>
#include <memory>

#include "application/use_cases/i_time_tracer_core_api.hpp"

class IReportHandler;
class IProjectRepository;
class IWorkflowHandler;
namespace time_tracer::application::ports {
class IDataQueryService;
class IReportDataQueryService;
class IReportDtoFormatter;
class IReportExportWriter;
}  // namespace time_tracer::application::ports

class TimeTracerCoreApi : public ITimeTracerCoreApi {
 public:
  TimeTracerCoreApi(
      IWorkflowHandler& workflow_handler, IReportHandler& report_handler,
      std::shared_ptr<IProjectRepository> project_repository,
      std::shared_ptr<time_tracer::application::ports::IDataQueryService>
          data_query_service,
      std::shared_ptr<time_tracer::application::ports::IReportDataQueryService>
          report_data_query_service = nullptr,
      std::shared_ptr<time_tracer::application::ports::IReportDtoFormatter>
          report_dto_formatter = nullptr,
      std::shared_ptr<time_tracer::application::ports::IReportExportWriter>
          report_export_writer = nullptr);

  auto RunConvert(const time_tracer::core::dto::ConvertRequest& request)
      -> time_tracer::core::dto::OperationAck override;

  auto RunIngest(const time_tracer::core::dto::IngestRequest& request)
      -> time_tracer::core::dto::OperationAck override;

  auto RunImport(const time_tracer::core::dto::ImportRequest& request)
      -> time_tracer::core::dto::OperationAck override;

  auto RunValidateStructure(
      const time_tracer::core::dto::ValidateStructureRequest& request)
      -> time_tracer::core::dto::OperationAck override;

  auto RunValidateLogic(
      const time_tracer::core::dto::ValidateLogicRequest& request)
      -> time_tracer::core::dto::OperationAck override;

  auto RunReportQuery(const time_tracer::core::dto::ReportQueryRequest& request)
      -> time_tracer::core::dto::TextOutput override;

  auto RunStructuredReportQuery(
      const time_tracer::core::dto::StructuredReportQueryRequest& request)
      -> time_tracer::core::dto::StructuredReportOutput override;

  auto RunPeriodBatchQuery(
      const time_tracer::core::dto::PeriodBatchQueryRequest& request)
      -> time_tracer::core::dto::TextOutput override;

  auto RunStructuredPeriodBatchQuery(
      const time_tracer::core::dto::StructuredPeriodBatchQueryRequest& request)
      -> time_tracer::core::dto::StructuredPeriodBatchOutput override;

  auto RunReportExport(
      const time_tracer::core::dto::ReportExportRequest& request)
      -> time_tracer::core::dto::OperationAck override;

  auto RunDataQuery(const time_tracer::core::dto::DataQueryRequest& request)
      -> time_tracer::core::dto::TextOutput override;

  auto RunTreeQuery(const time_tracer::core::dto::TreeQueryRequest& request)
      -> time_tracer::core::dto::TreeQueryResponse override;

 private:
  IWorkflowHandler& workflow_handler_;
  IReportHandler& report_handler_;
  std::shared_ptr<IProjectRepository> project_repository_;
  std::shared_ptr<time_tracer::application::ports::IDataQueryService>
      data_query_service_;
  std::shared_ptr<time_tracer::application::ports::IReportDataQueryService>
      kReport_;
  std::shared_ptr<time_tracer::application::ports::IReportDtoFormatter>
      report_dto_formatter_;
  std::shared_ptr<time_tracer::application::ports::IReportExportWriter>
      report_export_writer_;
};

#endif  // APPLICATION_USE_CASES_TIME_TRACER_CORE_API_H_
