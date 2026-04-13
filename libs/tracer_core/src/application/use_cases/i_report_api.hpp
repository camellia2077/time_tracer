#ifndef APPLICATION_USE_CASES_I_REPORT_API_HPP_
#define APPLICATION_USE_CASES_I_REPORT_API_HPP_

#include "application/dto/reporting_requests.hpp"
#include "application/dto/reporting_responses.hpp"
#include "application/dto/shared_envelopes.hpp"

namespace tracer::core::application::use_cases {

class IReportApi {
 public:
  virtual ~IReportApi() = default;

  virtual auto RunTemporalReportQuery(
      const tracer_core::core::dto::TemporalReportQueryRequest& request)
      -> tracer_core::core::dto::TextOutput = 0;

  virtual auto RunTemporalStructuredReportQuery(
      const tracer_core::core::dto::TemporalStructuredReportQueryRequest& request)
      -> tracer_core::core::dto::TemporalStructuredReportOutput = 0;

  virtual auto RunPeriodBatchQuery(
      const tracer_core::core::dto::PeriodBatchQueryRequest& request)
      -> tracer_core::core::dto::TextOutput = 0;

  virtual auto RunStructuredPeriodBatchQuery(
      const tracer_core::core::dto::StructuredPeriodBatchQueryRequest& request)
      -> tracer_core::core::dto::StructuredPeriodBatchOutput = 0;

  virtual auto RunTemporalReportTargetsQuery(
      const tracer_core::core::dto::TemporalReportTargetsRequest& request)
      -> tracer_core::core::dto::TemporalReportTargetsOutput = 0;

  virtual auto RunTemporalReportExport(
      const tracer_core::core::dto::TemporalReportExportRequest& request)
      -> tracer_core::core::dto::OperationAck = 0;
};

}  // namespace tracer::core::application::use_cases

using IReportApi = tracer::core::application::use_cases::IReportApi;

#endif  // APPLICATION_USE_CASES_I_REPORT_API_HPP_
