#ifndef APPLICATION_USE_CASES_I_REPORT_API_HPP_
#define APPLICATION_USE_CASES_I_REPORT_API_HPP_

#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"

namespace tracer::core::application::use_cases {

class IReportApi {
 public:
  virtual ~IReportApi() = default;

  virtual auto RunReportQuery(
      const tracer_core::core::dto::ReportQueryRequest& request)
      -> tracer_core::core::dto::TextOutput = 0;

  virtual auto RunStructuredReportQuery(
      const tracer_core::core::dto::StructuredReportQueryRequest& request)
      -> tracer_core::core::dto::StructuredReportOutput = 0;

  virtual auto RunPeriodBatchQuery(
      const tracer_core::core::dto::PeriodBatchQueryRequest& request)
      -> tracer_core::core::dto::TextOutput = 0;

  virtual auto RunStructuredPeriodBatchQuery(
      const tracer_core::core::dto::StructuredPeriodBatchQueryRequest& request)
      -> tracer_core::core::dto::StructuredPeriodBatchOutput = 0;

  virtual auto RunReportExport(
      const tracer_core::core::dto::ReportExportRequest& request)
      -> tracer_core::core::dto::OperationAck = 0;
};

}  // namespace tracer::core::application::use_cases

using IReportApi = tracer::core::application::use_cases::IReportApi;

#endif  // APPLICATION_USE_CASES_I_REPORT_API_HPP_
