// application/tests/test_main_pipeline.cpp
#include "application/tests/modules/pipeline_tests.hpp"
#include "application/tests/support/test_support.hpp"

auto main() -> int {
  tracer_core::application::tests::TestState state;

  tracer_core::application::tests::RunConvertIngestValidateTests(state);
  tracer_core::application::tests::RunImportServiceTests(state);
  tracer_core::application::tests::RunRecordTimeOrderModeTests(state);
  tracer_core::application::tests::RunTxtDayBlockTests(state);

  if (state.failures == 0) {
    std::cout << "[PASS] time_tracker_pipeline_api_tests" << '\n';
    return 0;
  }

  std::cerr << "[FAIL] time_tracker_pipeline_api_tests failures: "
            << state.failures << '\n';
  return 1;
}
