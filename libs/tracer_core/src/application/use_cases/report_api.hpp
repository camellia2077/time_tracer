#ifndef APPLICATION_USE_CASES_REPORT_API_HPP_
#define APPLICATION_USE_CASES_REPORT_API_HPP_

#include <memory>

#include "application/interfaces/i_report_handler.hpp"
#include "application/ports/i_report_data_query_service.hpp"
#include "application/ports/i_report_dto_formatter.hpp"
#include "application/ports/i_report_export_writer.hpp"
#include "application/use_cases/i_report_api.hpp"

namespace tracer::core::application::use_cases {

class ReportApi final : public IReportApi {
 public:
  using ReportDataQueryServicePtr =
      std::shared_ptr<tracer_core::application::ports::IReportDataQueryService>;
  using ReportDtoFormatterPtr =
      std::shared_ptr<tracer_core::application::ports::IReportDtoFormatter>;
  using ReportExportWriterPtr =
      std::shared_ptr<tracer_core::application::ports::IReportExportWriter>;

  ReportApi(IReportHandler& report_handler,
            ReportDataQueryServicePtr report_data_query_service = nullptr,
            ReportDtoFormatterPtr report_dto_formatter = nullptr,
            ReportExportWriterPtr report_export_writer = nullptr);

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

 private:
  IReportHandler& report_handler_;
  ReportDataQueryServicePtr report_data_query_service_;
  ReportDtoFormatterPtr report_dto_formatter_;
  ReportExportWriterPtr report_export_writer_;
};

}  // namespace tracer::core::application::use_cases

using ReportApi = tracer::core::application::use_cases::ReportApi;

#endif  // APPLICATION_USE_CASES_REPORT_API_HPP_
