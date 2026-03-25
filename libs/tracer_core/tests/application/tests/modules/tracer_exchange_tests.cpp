#include "application/tests/modules/exchange_tests.hpp"
#include "application/tests/support/fakes.hpp"
#include "application/tests/support/test_support.hpp"

namespace tracer_core::application::tests {

namespace {

auto TestExportDelegatesToExchangeService(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeReportHandler report_handler;
  auto repository = std::make_shared<FakeProjectRepository>();
  auto data_query = std::make_shared<FakeDataQueryService>();
  auto tracer_exchange = std::make_shared<FakeTracerExchangeService>();
  auto runtime_api = BuildRuntimeApi(pipeline_workflow, report_handler,
                                     repository, data_query, tracer_exchange);

  const tracer_core::core::dto::TracerExchangeExportRequest request{
      .input_text_root_path = "input",
      .requested_output_path = "out/export.tracer",
      .active_converter_main_config_path =
          "config/converter/interval_processor_config.toml",
      .passphrase = "secret",
      .producer_platform = "windows",
      .producer_app = "time_tracer_cli",
  };

  const auto result =
      runtime_api.tracer_exchange().RunTracerExchangeExport(request);
  Expect(state, result.ok,
         "RunTracerExchangeExport should return the service success result.");
  Expect(state, tracer_exchange->export_call_count == 1,
         "RunTracerExchangeExport should delegate to the exchange service.");
  Expect(state,
         tracer_exchange->last_export_request.requested_output_path ==
             request.requested_output_path,
         "RunTracerExchangeExport should forward requested_output_path.");
}

auto TestImportFailureIsWrapped(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeReportHandler report_handler;
  auto repository = std::make_shared<FakeProjectRepository>();
  auto data_query = std::make_shared<FakeDataQueryService>();
  auto tracer_exchange = std::make_shared<FakeTracerExchangeService>();
  tracer_exchange->throw_on_import = true;
  auto runtime_api = BuildRuntimeApi(pipeline_workflow, report_handler,
                                     repository, data_query, tracer_exchange);

  const auto result = runtime_api.tracer_exchange().RunTracerExchangeImport(
      {.input_tracer_path = "sample.tracer",
       .active_text_root_path = "runtime/input/full",
       .active_converter_main_config_path =
           "config/converter/interval_processor_config.toml",
       .runtime_work_root = "runtime/work",
       .passphrase = "secret"});
  Expect(state, !result.ok,
         "RunTracerExchangeImport should convert service exceptions into "
         "failed DTOs.");
  Expect(state,
         Contains(result.error_message, "RunTracerExchangeImport failed"),
         "RunTracerExchangeImport failure should include operation name.");
  Expect(state, Contains(result.error_message, "exchange import failed"),
         "RunTracerExchangeImport failure should preserve the service error.");
}

auto TestInspectWithoutServiceFailsGracefully(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeReportHandler report_handler;
  auto runtime_api = BuildRuntimeApiForTest(pipeline_workflow, report_handler);

  const auto result = runtime_api.tracer_exchange().RunTracerExchangeInspect(
      {.input_tracer_path = "sample.tracer", .passphrase = "secret"});
  Expect(state, !result.ok,
         "RunTracerExchangeInspect should fail cleanly when no exchange "
         "service is configured.");
  Expect(state, Contains(result.error_message, "service is not configured"),
         "Missing exchange service should surface a clear error message.");
}

}  // namespace

auto RunTracerExchangeTests(TestState& state) -> void {
  TestExportDelegatesToExchangeService(state);
  TestImportFailureIsWrapped(state);
  TestInspectWithoutServiceFailsGracefully(state);
}

}  // namespace tracer_core::application::tests
