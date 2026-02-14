// application/tests/support/test_support.cpp
#include "application/tests/support/test_support.hpp"

namespace time_tracer::application::tests {

auto BuildCoreApiForTest(FakeWorkflowHandler& workflow_handler,
                         FakeReportHandler& report_handler)
    -> TimeTracerCoreApi {
  auto repository = std::make_shared<FakeProjectRepository>();
  auto data_query = std::make_shared<FakeDataQueryService>();
  return BuildCoreApi(workflow_handler, report_handler, repository, data_query);
}

auto BuildCoreApiForTest(
    FakeWorkflowHandler& workflow_handler, FakeReportHandler& report_handler,
    const std::shared_ptr<FakeDataQueryService>& data_query)
    -> TimeTracerCoreApi {
  auto repository = std::make_shared<FakeProjectRepository>();
  return BuildCoreApi(workflow_handler, report_handler, repository, data_query);
}

}  // namespace time_tracer::application::tests
