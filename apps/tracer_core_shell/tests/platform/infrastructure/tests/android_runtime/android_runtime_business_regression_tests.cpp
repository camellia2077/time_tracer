// infrastructure/tests/android_runtime/android_runtime_business_regression_tests.cpp
#include "infrastructure/tests/android_runtime/android_runtime_business_regression_internal.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

namespace android_runtime_tests {

auto RunBusinessRegressionTests(int& failures) -> void {
  business_regression_internal::RunBusinessRegressionHistoryTests(failures);
  business_regression_internal::RunBusinessRegressionIngestGuardTests(failures);
}

}  // namespace android_runtime_tests
