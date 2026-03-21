#include "application/use_cases/tracer_core_api.hpp"

#include <memory>
#include <utility>

#include "application/use_cases/pipeline_api.hpp"
#include "application/use_cases/query_api.hpp"
#include "application/use_cases/report_api.hpp"
#include "application/use_cases/tracer_exchange_api.hpp"

namespace tracer::core::application::use_cases {

TracerCoreApi::TracerCoreApi(
    ::tracer::core::application::workflow::IWorkflowHandler& workflow_handler,
    IReportHandler& report_handler, ProjectRepositoryPtr project_repository,
    DataQueryServicePtr data_query_service,
    ReportDataQueryServicePtr report_data_query_service,
    ReportDtoFormatterPtr report_dto_formatter,
    ReportExportWriterPtr report_export_writer,
    TracerExchangeServicePtr tracer_exchange_service)
    : TracerCoreRuntime(
          std::make_shared<PipelineApi>(workflow_handler),
          std::make_shared<QueryApi>(std::move(project_repository),
                                     std::move(data_query_service)),
          std::make_shared<ReportApi>(report_handler,
                                      std::move(report_data_query_service),
                                      std::move(report_dto_formatter),
                                      std::move(report_export_writer)),
          std::make_shared<TracerExchangeApi>(
              std::move(tracer_exchange_service))) {}

}  // namespace tracer::core::application::use_cases
