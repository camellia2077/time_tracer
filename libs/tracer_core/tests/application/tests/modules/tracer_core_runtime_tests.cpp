#include "application/tests/modules/aggregate_runtime_tests.hpp"

#include "application/tests/support/fakes.hpp"
#include "application/tests/support/test_support.hpp"
#include "application/use_cases/pipeline_api.hpp"
#include "application/use_cases/query_api.hpp"
#include "application/use_cases/report_api.hpp"
#include "application/aggregate_runtime/tracer_core_runtime.hpp"
#include "application/use_cases/tracer_exchange_api.hpp"

namespace tracer_core::application::tests {

namespace {

auto TestRuntimeAccessorsAndForwarding(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeReportHandler report_handler;
  auto repository = std::make_shared<FakeProjectRepository>();
  auto data_query = std::make_shared<FakeDataQueryService>();
  auto tracer_exchange = std::make_shared<FakeTracerExchangeService>();
  auto report_data_query = std::make_shared<FakeReportDataQueryService>();

  auto pipeline_api = std::make_shared<PipelineApi>(pipeline_workflow);
  auto query_api = std::make_shared<QueryApi>(repository, data_query);
  auto report_formatter = std::make_shared<FakeReportDtoFormatter>();
  auto report_api = std::make_shared<ReportApi>(report_handler,
                                                report_data_query,
                                                report_formatter);
  auto tracer_exchange_api =
      std::make_shared<TracerExchangeApi>(tracer_exchange);

  TracerCoreRuntime runtime(pipeline_api, query_api, report_api,
                            tracer_exchange_api);

  Expect(state, &runtime.pipeline() == pipeline_api.get(),
         "TracerCoreRuntime should expose the same pipeline API instance.");
  Expect(state, &runtime.query() == query_api.get(),
         "TracerCoreRuntime should expose the same query API instance.");
  Expect(state, &runtime.report() == report_api.get(),
         "TracerCoreRuntime should expose the same report API instance.");
  Expect(
      state, &runtime.tracer_exchange() == tracer_exchange_api.get(),
      "TracerCoreRuntime should expose the same tracer-exchange API instance.");

  const auto ingest_result = runtime.pipeline().RunIngest(
      {.input_path = "input-root",
       .date_check_mode = DateCheckMode::kContinuity,
       .save_processed_output = true,
       .ingest_mode = IngestMode::kStandard});
  Expect(state, ingest_result.ok,
         "Pipeline API should still report ingest success through runtime "
         "accessors.");

  auto data_request = tracer_core::core::dto::DataQueryRequest{};
  data_request.action = tracer_core::core::dto::DataQueryAction::kYears;
  const auto data_result = runtime.query().RunDataQuery(data_request);
  Expect(state, data_result.ok && data_result.content == "data-query-result",
         "Query API should still return the delegated data query result.");

  const auto report_result = runtime.report().RunTemporalReportQuery(
      {.display_mode = tracer_core::core::dto::ReportDisplayMode::kDay,
       .selection =
           {.kind = tracer_core::core::dto::TemporalSelectionKind::kSingleDay,
            .date = "2026-03-21"},
       .format = ReportFormat::kMarkdown});
  Expect(state, report_result.ok && report_result.content == "daily:2026-03-21",
         "Report API should still return the delegated report result.");

  const auto exchange_result =
      runtime.tracer_exchange().RunTracerExchangeInspect(
          {.input_tracer_path = "sample.tracer", .passphrase = "secret"});
  Expect(
      state, exchange_result.ok,
      "Tracer exchange API should still return the delegated inspect result.");

  Expect(state, pipeline_workflow.ingest_call_count == 1,
         "Runtime aggregate should not hide pipeline-side delegation.");
  Expect(state, data_query->call_count == 1,
         "Runtime aggregate should not hide query-side delegation.");
  Expect(state, tracer_exchange->inspect_call_count == 1,
         "Runtime aggregate should not hide tracer-exchange delegation.");
}

}  // namespace

auto RunTracerCoreRuntimeTests(TestState& state) -> void {
  TestRuntimeAccessorsAndForwarding(state);
}

}  // namespace tracer_core::application::tests
