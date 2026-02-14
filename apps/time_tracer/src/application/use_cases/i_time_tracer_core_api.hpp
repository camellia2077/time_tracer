// application/use_cases/i_time_tracer_core_api.hpp
#ifndef APPLICATION_USE_CASES_I_TIME_TRACER_CORE_API_H_
#define APPLICATION_USE_CASES_I_TIME_TRACER_CORE_API_H_

#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"

class ITimeTracerCoreApi {
 public:
  virtual ~ITimeTracerCoreApi() = default;

  virtual auto RunConvert(const time_tracer::core::dto::ConvertRequest& request)
      -> time_tracer::core::dto::OperationAck = 0;

  virtual auto RunIngest(const time_tracer::core::dto::IngestRequest& request)
      -> time_tracer::core::dto::OperationAck = 0;

  virtual auto RunImport(const time_tracer::core::dto::ImportRequest& request)
      -> time_tracer::core::dto::OperationAck = 0;

  virtual auto RunValidateStructure(
      const time_tracer::core::dto::ValidateStructureRequest& request)
      -> time_tracer::core::dto::OperationAck = 0;

  virtual auto RunValidateLogic(
      const time_tracer::core::dto::ValidateLogicRequest& request)
      -> time_tracer::core::dto::OperationAck = 0;

  virtual auto RunReportQuery(
      const time_tracer::core::dto::ReportQueryRequest& request)
      -> time_tracer::core::dto::TextOutput = 0;

  virtual auto RunStructuredReportQuery(
      const time_tracer::core::dto::StructuredReportQueryRequest& request)
      -> time_tracer::core::dto::StructuredReportOutput = 0;

  virtual auto RunPeriodBatchQuery(
      const time_tracer::core::dto::PeriodBatchQueryRequest& request)
      -> time_tracer::core::dto::TextOutput = 0;

  virtual auto RunStructuredPeriodBatchQuery(
      const time_tracer::core::dto::StructuredPeriodBatchQueryRequest& request)
      -> time_tracer::core::dto::StructuredPeriodBatchOutput = 0;

  virtual auto RunReportExport(
      const time_tracer::core::dto::ReportExportRequest& request)
      -> time_tracer::core::dto::OperationAck = 0;

  virtual auto RunDataQuery(
      const time_tracer::core::dto::DataQueryRequest& request)
      -> time_tracer::core::dto::TextOutput = 0;

  virtual auto RunTreeQuery(
      const time_tracer::core::dto::TreeQueryRequest& request)
      -> time_tracer::core::dto::TreeQueryResponse = 0;
};

#endif  // APPLICATION_USE_CASES_I_TIME_TRACER_CORE_API_H_
