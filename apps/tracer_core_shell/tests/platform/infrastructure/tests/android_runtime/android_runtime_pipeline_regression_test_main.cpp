#include <iostream>

#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

auto main() -> int {
  int failures = 0;
  android_runtime_tests::RunBusinessRegressionTests(failures);
  android_runtime_tests::RunValidationIssueReporterTests(failures);
  android_runtime_tests::RunTxtMonthHeaderTests(failures);
  android_runtime_tests::RunValidateLogicStructureReportingTests(failures);

  if (failures == 0) {
    std::cout
        << "[PASS] time_tracker_android_runtime_pipeline_regression_tests\n";
    return 0;
  }

  std::cerr
      << "[FAIL] time_tracker_android_runtime_pipeline_regression_tests failures: "
      << failures << '\n';
  return 1;
}
