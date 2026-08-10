#ifndef APPLICATION_USE_CASES_I_INSIGHTS_API_HPP_
#define APPLICATION_USE_CASES_I_INSIGHTS_API_HPP_

#include "application/dto/insights_requests.hpp"
#include "application/dto/insights_responses.hpp"
#include "application/dto/shared_envelopes.hpp"

namespace tracer::core::application::use_cases {

class IInsightsApi {
 public:
  virtual ~IInsightsApi() = default;

  virtual auto RunTemporalInsightsQuery(
      const tracer_core::core::dto::TemporalInsightsQueryRequest& request)
      -> tracer_core::core::dto::TextOutput = 0;

  virtual auto RunTemporalStructuredInsightsQuery(
      const tracer_core::core::dto::TemporalStructuredInsightsQueryRequest&
          request)
      -> tracer_core::core::dto::TemporalStructuredInsightsOutput = 0;

  virtual auto RunPeriodBatchQuery(
      const tracer_core::core::dto::PeriodBatchQueryRequest& request)
      -> tracer_core::core::dto::TextOutput = 0;

  virtual auto RunStructuredPeriodBatchQuery(
      const tracer_core::core::dto::StructuredPeriodBatchQueryRequest& request)
      -> tracer_core::core::dto::StructuredPeriodBatchOutput = 0;

  virtual auto RunTemporalInsightsTargetsQuery(
      const tracer_core::core::dto::TemporalInsightsTargetsRequest& request)
      -> tracer_core::core::dto::TemporalInsightsTargetsOutput = 0;

  virtual auto RunTemporalInsightsExport(
      const tracer_core::core::dto::TemporalInsightsExportRequest& request)
      -> tracer_core::core::dto::OperationAck = 0;
};

}  // namespace tracer::core::application::use_cases

using IInsightsApi = tracer::core::application::use_cases::IInsightsApi;

#endif  // APPLICATION_USE_CASES_I_INSIGHTS_API_HPP_
