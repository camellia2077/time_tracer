// application/tests/modules/data_query_tests.cpp
#include <memory>

#include "application/tests/modules/test_modules.hpp"
#include "application/tests/support/fakes.hpp"
#include "application/tests/support/test_support.hpp"

namespace time_tracer::application::tests {

using time_tracer::core::dto::DataQueryAction;
using time_tracer::core::dto::DataQueryRequest;

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

}  // namespace

auto RunDataQueryTests(TestState& state) -> void {
  TestDataQueryResponses(state);
}

}  // namespace time_tracer::application::tests
