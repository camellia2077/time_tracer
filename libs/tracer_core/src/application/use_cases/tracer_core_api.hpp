#ifndef APPLICATION_USE_CASES_TRACER_CORE_API_H_
#define APPLICATION_USE_CASES_TRACER_CORE_API_H_

#include <memory>

#include "application/interfaces/i_report_handler.hpp"
#include "application/interfaces/i_workflow_handler.hpp"
#include "application/ports/i_data_query_service.hpp"
#include "application/ports/i_report_data_query_service.hpp"
#include "application/ports/i_report_dto_formatter.hpp"
#include "application/ports/i_report_export_writer.hpp"
#include "application/ports/i_tracer_exchange_service.hpp"
#include "application/use_cases/tracer_core_runtime.hpp"
#include "domain/repositories/i_project_repository.hpp"

namespace tracer::core::application::use_cases {

class TracerCoreApi final : public TracerCoreRuntime {
 public:
  using ProjectRepositoryPtr = std::shared_ptr<IProjectRepository>;
  using DataQueryServicePtr =
      std::shared_ptr<tracer_core::application::ports::IDataQueryService>;
  using ReportDataQueryServicePtr =
      std::shared_ptr<tracer_core::application::ports::IReportDataQueryService>;
  using ReportDtoFormatterPtr =
      std::shared_ptr<tracer_core::application::ports::IReportDtoFormatter>;
  using ReportExportWriterPtr =
      std::shared_ptr<tracer_core::application::ports::IReportExportWriter>;
  using TracerExchangeServicePtr =
      std::shared_ptr<tracer_core::application::ports::ITracerExchangeService>;

  TracerCoreApi(
      ::tracer::core::application::workflow::IWorkflowHandler& workflow_handler,
      IReportHandler& report_handler,
      ProjectRepositoryPtr project_repository,
      DataQueryServicePtr data_query_service,
      ReportDataQueryServicePtr report_data_query_service = nullptr,
      ReportDtoFormatterPtr report_dto_formatter = nullptr,
      ReportExportWriterPtr report_export_writer = nullptr,
      TracerExchangeServicePtr tracer_exchange_service = nullptr);
};

}  // namespace tracer::core::application::use_cases

using TracerCoreApi = tracer::core::application::use_cases::TracerCoreApi;

#endif  // APPLICATION_USE_CASES_TRACER_CORE_API_H_
