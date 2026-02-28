// application/tests/modules/data_query_tests.cpp
#include <memory>

#include "application/tests/modules/test_modules.hpp"
#include "application/tests/support/fakes.hpp"
#include "application/tests/support/test_support.hpp"

namespace tracer_core::application::tests {

using tracer_core::core::dto::DataQueryAction;
using tracer_core::core::dto::DataQueryRequest;
using tracer_core::core::dto::TreeQueryRequest;

namespace {

auto TestDataQueryResponses(TestState& state) -> void {
  FakeWorkflowHandler workflow_handler;
  FakeReportHandler report_handler;
  auto data_query = std::make_shared<FakeDataQueryService>();
  auto core_api =
      BuildCoreApiForTest(workflow_handler, report_handler, data_query);

  data_query->response = {.ok = true, .content = "years", .error_message = ""};
  DataQueryRequest success_request;
  success_request.action = DataQueryAction::kYears;
  const auto kSuccess = core_api.RunDataQuery(success_request);
  Expect(state, kSuccess.ok, "RunDataQuery should return ok on success.");
  Expect(state, kSuccess.content == "years",
         "RunDataQuery should return service content.");
  Expect(state, data_query->call_count == 1,
         "RunDataQuery should call data query service once.");

  data_query->response = {
      .ok = false, .content = "", .error_message = "query rejected"};
  DataQueryRequest failed_request;
  failed_request.action = DataQueryAction::kDays;
  const auto kFailedResponse = core_api.RunDataQuery(failed_request);
  Expect(state, !kFailedResponse.ok,
         "RunDataQuery should preserve failure DTO from service.");
  Expect(state, kFailedResponse.error_message == "query rejected",
         "RunDataQuery should preserve failure message from service.");

  data_query->fail_query = true;
  DataQueryRequest exception_request;
  exception_request.action = DataQueryAction::kMonths;
  const auto kFailure = core_api.RunDataQuery(exception_request);
  Expect(state, !kFailure.ok,
         "RunDataQuery should return failed DTO when service throws.");
  Expect(state, Contains(kFailure.error_message, "RunDataQuery failed"),
         "RunDataQuery thrown error should include operation name.");
}

auto TestTreeQueryResponses(TestState& state) -> void {
  FakeWorkflowHandler workflow_handler;
  FakeReportHandler report_handler;
  auto data_query = std::make_shared<FakeDataQueryService>();
  auto repository = std::make_shared<FakeProjectRepository>();
  constexpr int kRootBId = 5;
  repository->projects = {
      {.id = 1, .parent_id = std::nullopt, .name = "root"},
      {.id = 2, .parent_id = 1, .name = "child"},
      {.id = 3, .parent_id = 2, .name = "leaf"},
      {.id = 4, .parent_id = 1, .name = "focus"},
      {.id = kRootBId, .parent_id = std::nullopt, .name = "root_b"},
  };
  auto core_api =
      BuildCoreApi(workflow_handler, report_handler, repository, data_query);

  TreeQueryRequest list_roots_request{};
  list_roots_request.list_roots = true;
  const auto kRootsResponse = core_api.RunTreeQuery(list_roots_request);
  Expect(state, kRootsResponse.ok,
         "RunTreeQuery list roots should return ok=true.");
  Expect(state, kRootsResponse.found,
         "RunTreeQuery list roots should return found=true.");
  Expect(state, kRootsResponse.roots.size() == 2U,
         "RunTreeQuery list roots should return two roots.");
  Expect(state, kRootsResponse.roots[0] == "root",
         "RunTreeQuery list roots should include `root` first.");
  Expect(state, kRootsResponse.roots[1] == "root_b",
         "RunTreeQuery list roots should include `root_b` second.");

  TreeQueryRequest filtered_request{};
  filtered_request.root_pattern = "root_child";
  const auto kFilteredResponse = core_api.RunTreeQuery(filtered_request);
  Expect(state, kFilteredResponse.ok,
         "RunTreeQuery root pattern should return ok=true.");
  Expect(state, kFilteredResponse.found,
         "RunTreeQuery root pattern should return found=true.");
  Expect(state, kFilteredResponse.nodes.size() == 1U,
         "RunTreeQuery root pattern should return one subtree root.");
  Expect(state, kFilteredResponse.nodes[0].name == "child",
         "RunTreeQuery root pattern should match child node.");
  Expect(state, kFilteredResponse.nodes[0].path == "root_child",
         "RunTreeQuery root pattern should preserve node path.");
  Expect(state, kFilteredResponse.nodes[0].children.size() == 1U,
         "RunTreeQuery root pattern should keep subtree children.");

  TreeQueryRequest depth_limited_request{};
  depth_limited_request.root_pattern = "root";
  depth_limited_request.max_depth = 1;
  const auto kDepthLimitedResponse =
      core_api.RunTreeQuery(depth_limited_request);
  Expect(state, kDepthLimitedResponse.ok,
         "RunTreeQuery depth limit should return ok=true.");
  Expect(state, kDepthLimitedResponse.found,
         "RunTreeQuery depth limit should return found=true.");
  Expect(state, kDepthLimitedResponse.nodes.size() == 1U,
         "RunTreeQuery depth limit should return one root node.");
  Expect(state, kDepthLimitedResponse.nodes[0].children.size() == 2U,
         "RunTreeQuery depth limit should keep first-level children.");
  Expect(state,
         kDepthLimitedResponse.nodes[0].children[0].children.empty() &&
             kDepthLimitedResponse.nodes[0].children[1].children.empty(),
         "RunTreeQuery depth limit should trim nodes deeper than max_depth.");

  TreeQueryRequest missing_request{};
  missing_request.root_pattern = "missing";
  const auto kMissingResponse = core_api.RunTreeQuery(missing_request);
  Expect(state, kMissingResponse.ok,
         "RunTreeQuery missing root should still return ok=true.");
  Expect(state, !kMissingResponse.found,
         "RunTreeQuery missing root should return found=false.");
  Expect(state, kMissingResponse.nodes.empty(),
         "RunTreeQuery missing root should return empty nodes.");

  repository->fail_get_all_projects = true;
  const auto kFailureResponse = core_api.RunTreeQuery(list_roots_request);
  Expect(state, !kFailureResponse.ok,
         "RunTreeQuery should return failed DTO when repository throws.");
  Expect(state, Contains(kFailureResponse.error_message, "RunTreeQuery failed"),
         "RunTreeQuery thrown error should include operation name.");
}

}  // namespace

auto RunDataQueryTests(TestState& state) -> void {
  TestDataQueryResponses(state);
  TestTreeQueryResponses(state);
}

}  // namespace tracer_core::application::tests
