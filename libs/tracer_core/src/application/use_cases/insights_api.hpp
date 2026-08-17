#ifndef APPLICATION_USE_CASES_INSIGHTS_API_HPP_
#define APPLICATION_USE_CASES_INSIGHTS_API_HPP_

#include <memory>

#include "application/compat/insights/i_insights_handler.hpp"
#include "application/ports/insights/i_insights_data_query_service.hpp"
#include "application/ports/insights/i_insights_dto_formatter.hpp"
#include "application/use_cases/i_insights_api.hpp"

namespace tracer::core::application::use_cases {

class InsightsApi final : public IInsightsApi {
 public:
  using InsightsDataQueryServicePtr = std::shared_ptr<
      tracer_core::application::ports::IInsightsDataQueryService>;
  using InsightsDtoFormatterPtr =
      std::shared_ptr<tracer_core::application::ports::IInsightsDtoFormatter>;

  InsightsApi(IInsightsHandler& insights_handler,
              InsightsDataQueryServicePtr insights_data_query_service = nullptr,
              InsightsDtoFormatterPtr insights_dto_formatter = nullptr);

  auto RunTemporalInsightsQuery(
      const tracer_core::core::dto::TemporalInsightsQueryRequest& request)
      -> tracer_core::core::dto::TextOutput override;

  auto RunTemporalStructuredInsightsQuery(
      const tracer_core::core::dto::TemporalStructuredInsightsQueryRequest&
          request)
      -> tracer_core::core::dto::TemporalStructuredInsightsOutput override;

  auto RunPeriodBatchQuery(
      const tracer_core::core::dto::PeriodBatchQueryRequest& request)
      -> tracer_core::core::dto::TextOutput override;

  auto RunStructuredPeriodBatchQuery(
      const tracer_core::core::dto::StructuredPeriodBatchQueryRequest& request)
      -> tracer_core::core::dto::StructuredPeriodBatchOutput override;

  auto RunTemporalInsightsTargetsQuery(
      const tracer_core::core::dto::TemporalInsightsTargetsRequest& request)
      -> tracer_core::core::dto::TemporalInsightsTargetsOutput override;

  auto RunTemporalInsightsExport(
      const tracer_core::core::dto::TemporalInsightsExportRequest& request)
      -> tracer_core::core::dto::OperationAck override;

 private:
  IInsightsHandler& insights_handler_;
  InsightsDataQueryServicePtr insights_data_query_service_;
  InsightsDtoFormatterPtr insights_dto_formatter_;
};

}  // namespace tracer::core::application::use_cases

using InsightsApi = tracer::core::application::use_cases::InsightsApi;

#endif  // APPLICATION_USE_CASES_INSIGHTS_API_HPP_
