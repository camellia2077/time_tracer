#ifndef APPLICATION_USE_CASES_I_PIPELINE_API_HPP_
#define APPLICATION_USE_CASES_I_PIPELINE_API_HPP_

#include "application/dto/pipeline_requests.hpp"
#include "application/dto/pipeline_responses.hpp"
#include "application/dto/shared_envelopes.hpp"

namespace tracer::core::application::use_cases {

class IPipelineApi {
 public:
  virtual ~IPipelineApi() = default;

  virtual auto RunConvert(const tracer_core::core::dto::ConvertRequest& request)
      -> tracer_core::core::dto::OperationAck = 0;

  virtual auto RunIngest(const tracer_core::core::dto::IngestRequest& request)
      -> tracer_core::core::dto::OperationAck = 0;

  virtual auto RunIngestSyncStatusQuery(
      const tracer_core::core::dto::IngestSyncStatusRequest& request)
      -> tracer_core::core::dto::IngestSyncStatusOutput = 0;

  virtual auto ClearIngestSyncStatus() -> tracer_core::core::dto::OperationAck =
      0;

  virtual auto RunImport(const tracer_core::core::dto::ImportRequest& request)
      -> tracer_core::core::dto::OperationAck = 0;

  virtual auto RunValidateStructure(
      const tracer_core::core::dto::ValidateStructureRequest& request)
      -> tracer_core::core::dto::OperationAck = 0;

  virtual auto RunValidateLogic(
      const tracer_core::core::dto::ValidateLogicRequest& request)
      -> tracer_core::core::dto::OperationAck = 0;

  virtual auto RunRecordActivityAtomically(
      const tracer_core::core::dto::RecordActivityAtomicallyRequest& request)
      -> tracer_core::core::dto::RecordActivityAtomicallyResponse = 0;
};

}  // namespace tracer::core::application::use_cases

using IPipelineApi = tracer::core::application::use_cases::IPipelineApi;

#endif  // APPLICATION_USE_CASES_I_PIPELINE_API_HPP_
