// infrastructure/tests/data_query/data_query_refactor_stats_tests.cpp
#include "infrastructure/tests/data_query/data_query_refactor_test_internal.hpp"

namespace android_runtime_tests::data_query_refactor_internal {

auto RunDataQueryRefactorStatsTests(int& failures) -> void {
  RunDataQueryRefactorStatsScenarioTests(failures);
}

}  // namespace android_runtime_tests::data_query_refactor_internal
