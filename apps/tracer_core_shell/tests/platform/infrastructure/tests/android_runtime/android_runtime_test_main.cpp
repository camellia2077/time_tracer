// infrastructure/tests/android_runtime/android_runtime_test_main.cpp
#include <iostream>

#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

auto main() -> int {
  int failures = 0;
  android_runtime_tests::RunRuntimeSmokeTests(failures);
  android_runtime_tests::RunCoreConfigValidationTests(failures);
  android_runtime_tests::RunBusinessRegressionTests(failures);
  android_runtime_tests::RunReportConsistencyTests(failures);
  android_runtime_tests::RunAndroidBundlePolicyTests(failures);
  android_runtime_tests::RunCompatibilityTests(failures);
  android_runtime_tests::RunValidationIssueReporterTests(failures);
  android_runtime_tests::RunTxtMonthHeaderTests(failures);
  android_runtime_tests::RunDataQueryRefactorTests(failures);

  if (failures == 0) {
    std::cout << "[PASS] time_tracker_android_runtime_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] time_tracker_android_runtime_tests failures: "
            << failures << '\n';
  return 1;
}
