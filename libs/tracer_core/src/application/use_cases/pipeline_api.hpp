#ifndef APPLICATION_USE_CASES_PIPELINE_API_HPP_
#define APPLICATION_USE_CASES_PIPELINE_API_HPP_

#include "application/pipeline/i_pipeline_workflow.hpp"
#include "application/use_cases/i_pipeline_api.hpp"

namespace tracer::core::application::use_cases {

class PipelineApi final : public IPipelineApi {
 public:
  explicit PipelineApi(pipeline::IPipelineWorkflow& pipeline_workflow);

  auto RunConvert(const tracer_core::core::dto::ConvertRequest& request)
      -> tracer_core::core::dto::OperationAck override;

  auto RunIngest(const tracer_core::core::dto::IngestRequest& request)
      -> tracer_core::core::dto::OperationAck override;

  auto RunIngestSyncStatusQuery(
      const tracer_core::core::dto::IngestSyncStatusRequest& request)
      -> tracer_core::core::dto::IngestSyncStatusOutput override;

  auto ClearIngestSyncStatus() -> tracer_core::core::dto::OperationAck override;

  auto RunImport(const tracer_core::core::dto::ImportRequest& request)
      -> tracer_core::core::dto::OperationAck override;

  auto RunValidateStructure(
      const tracer_core::core::dto::ValidateStructureRequest& request)
      -> tracer_core::core::dto::OperationAck override;

  auto RunValidateLogic(
      const tracer_core::core::dto::ValidateLogicRequest& request)
      -> tracer_core::core::dto::OperationAck override;

  auto RunRecordActivityAtomically(
      const tracer_core::core::dto::RecordActivityAtomicallyRequest& request)
      -> tracer_core::core::dto::RecordActivityAtomicallyResponse override;
  auto RunUpdateActivityRemarkAtomically(
      const tracer_core::core::dto::UpdateActivityRemarkAtomicallyRequest&
          request)
      -> tracer_core::core::dto::UpdateActivityRemarkAtomicallyResponse
      override;
  auto RunUpdateDayRemarkAtomically(
      const tracer_core::core::dto::UpdateDayRemarkAtomicallyRequest& request)
      -> tracer_core::core::dto::UpdateDayRemarkAtomicallyResponse override;

  auto RunDefaultTxtDayMarker(
      const tracer_core::core::dto::DefaultTxtDayMarkerRequest& request)
      -> tracer_core::core::dto::DefaultTxtDayMarkerResponse override;

  auto RunResolveTxtDayBlock(
      const tracer_core::core::dto::ResolveTxtDayBlockRequest& request)
      -> tracer_core::core::dto::ResolveTxtDayBlockResponse override;

  auto RunReplaceTxtDayBlock(
      const tracer_core::core::dto::ReplaceTxtDayBlockRequest& request)
      -> tracer_core::core::dto::ReplaceTxtDayBlockResponse override;

  auto RunResolveTxtDayEdit(
      const tracer_core::core::dto::ResolveTxtDayEditRequest& request)
      -> tracer_core::core::dto::ResolveTxtDayEditResponse override;
  auto RunApplyTxtDayEdit(
      const tracer_core::core::dto::ApplyTxtDayEditRequest& request)
      -> tracer_core::core::dto::ApplyTxtDayEditResponse override;

  auto RunConvertTxtActivityNames(
      const tracer_core::core::dto::ConvertTxtActivityNamesRequest& request)
      -> tracer_core::core::dto::ConvertTxtActivityNamesResponse override;
  auto RunReplaceTxtCanonicalActivityNames(
      const tracer_core::core::dto::ReplaceTxtCanonicalActivityNamesRequest&
          request)
      -> tracer_core::core::dto::ReplaceTxtCanonicalActivityNamesResponse
      override;
  auto RunReplaceTxtAliasActivityNames(
      const tracer_core::core::dto::ReplaceTxtAliasActivityNamesRequest&
          request)
      -> tracer_core::core::dto::ReplaceTxtAliasActivityNamesResponse override;

 private:
  pipeline::IPipelineWorkflow& pipeline_workflow_;
};

}  // namespace tracer::core::application::use_cases

using PipelineApi = tracer::core::application::use_cases::PipelineApi;

#endif  // APPLICATION_USE_CASES_PIPELINE_API_HPP_
