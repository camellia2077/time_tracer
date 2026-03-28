// application/tests/support/test_support.cpp
#include "application/tests/support/test_support.hpp"

namespace tracer_core::application::tests {

auto BuildRuntimeApiForTest(FakePipelineWorkflow& pipeline_workflow,
                            FakeReportHandler& report_handler)
    -> TracerCoreRuntime {
  auto repository = std::make_shared<FakeProjectRepository>();
  auto data_query = std::make_shared<FakeDataQueryService>();
  return BuildRuntimeApi(pipeline_workflow, report_handler, repository,
                         data_query);
}

auto BuildRuntimeApiForTest(
    FakePipelineWorkflow& pipeline_workflow, FakeReportHandler& report_handler,
    const std::shared_ptr<FakeDataQueryService>& data_query)
    -> TracerCoreRuntime {
  auto repository = std::make_shared<FakeProjectRepository>();
  return BuildRuntimeApi(pipeline_workflow, report_handler, repository,
                         data_query);
}

auto BuildRuntimeApiForTest(
    FakePipelineWorkflow& pipeline_workflow, FakeReportHandler& report_handler,
    const std::shared_ptr<FakeReportDataQueryService>& report_data_query)
    -> TracerCoreRuntime {
  auto repository = std::make_shared<FakeProjectRepository>();
  auto data_query = std::make_shared<FakeDataQueryService>();
  return BuildRuntimeApi(pipeline_workflow, report_handler, repository,
                         data_query, nullptr, report_data_query);
}

}  // namespace tracer_core::application::tests
