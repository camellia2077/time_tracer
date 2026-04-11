// application/tests/modules/query_semantics_tests.cpp
#include <memory>

#include "application/tests/modules/query_tests.hpp"
#include "application/tests/support/fakes.hpp"
#include "application/tests/support/test_support.hpp"

namespace tracer_core::application::tests {

using tracer_core::core::dto::DataQueryAction;
using tracer_core::core::dto::DataQueryRequest;

namespace {

auto TestDataQueryFailureMessageNormalization(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeReportHandler report_handler;
  auto data_query = std::make_shared<FakeDataQueryService>();
  auto runtime_api =
      BuildRuntimeApiForTest(pipeline_workflow, report_handler, data_query);

  data_query->response = {.ok = false, .content = "", .error_message = ""};

  DataQueryRequest request{};
  request.action = DataQueryAction::kMappingNames;
  const auto response = runtime_api.query().RunDataQuery(request);

  Expect(state, !response.ok,
         "RunDataQuery should keep failed service responses as failures.");
  Expect(state, Contains(response.error_message, "RunDataQuery"),
         "RunDataQuery should normalize empty failure messages with operation name.");
  Expect(state,
         Contains(response.error_message,
                  "Data query service returned a failed response."),
         "RunDataQuery should explain failed service responses with empty messages.");
}

auto TestDataQueryPreservesEmptySuccessfulPayload(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeReportHandler report_handler;
  auto data_query = std::make_shared<FakeDataQueryService>();
  auto runtime_api =
      BuildRuntimeApiForTest(pipeline_workflow, report_handler, data_query);

  data_query->response = {.ok = true, .content = "", .error_message = ""};

  DataQueryRequest request{};
  request.action = DataQueryAction::kWakeKeywords;
  const auto response = runtime_api.query().RunDataQuery(request);

  Expect(state, response.ok,
         "RunDataQuery should keep empty successful payloads as successes.");
  Expect(state, response.content.empty(),
         "RunDataQuery should preserve empty successful content.");
  Expect(state, response.error_message.empty(),
         "RunDataQuery should keep empty success error_message.");
}

auto TestDataQueryForwardsStableActionPayload(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeReportHandler report_handler;
  auto data_query = std::make_shared<FakeDataQueryService>();
  auto runtime_api =
      BuildRuntimeApiForTest(pipeline_workflow, report_handler, data_query);

  data_query->response = {.ok = true,
                          .content = "[\"alias_a\",\"alias_b\"]",
                          .error_message = ""};

  DataQueryRequest request{};
  request.action = DataQueryAction::kMappingAliasKeys;
  request.output_mode = tracer_core::core::dto::DataQueryOutputMode::kSemanticJson;
  request.root = "study";
  request.limit = 5;
  request.reverse = true;
  request.activity_score_by_duration = true;
  const auto response = runtime_api.query().RunDataQuery(request);

  Expect(state, response.ok,
         "RunDataQuery should succeed for stable alias-key actions.");
  Expect(state, response.content == "[\"alias_a\",\"alias_b\"]",
         "RunDataQuery should preserve service content for stable alias-key actions.");
  Expect(state, data_query->call_count == 1,
         "RunDataQuery should call the data query service exactly once.");
  Expect(state, data_query->last_request.action == DataQueryAction::kMappingAliasKeys,
         "RunDataQuery should forward stable action type unchanged.");
  Expect(state,
         data_query->last_request.output_mode ==
             tracer_core::core::dto::DataQueryOutputMode::kSemanticJson,
         "RunDataQuery should forward output_mode unchanged.");
  Expect(state, data_query->last_request.root == request.root,
         "RunDataQuery should forward root unchanged.");
  Expect(state, data_query->last_request.limit == request.limit,
         "RunDataQuery should forward limit unchanged.");
  Expect(state, data_query->last_request.reverse == request.reverse,
         "RunDataQuery should forward reverse unchanged.");
  Expect(state,
         data_query->last_request.activity_score_by_duration ==
             request.activity_score_by_duration,
         "RunDataQuery should forward score flag unchanged.");
}

}  // namespace

auto RunQuerySemanticsTests(TestState& state) -> void {
  TestDataQueryFailureMessageNormalization(state);
  TestDataQueryPreservesEmptySuccessfulPayload(state);
  TestDataQueryForwardsStableActionPayload(state);
}

}  // namespace tracer_core::application::tests
