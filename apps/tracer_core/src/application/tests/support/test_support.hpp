// application/tests/support/test_support.hpp
#ifndef APPLICATION_TESTS_SUPPORT_TEST_SUPPORT_H_
#define APPLICATION_TESTS_SUPPORT_TEST_SUPPORT_H_

#include <iostream>
#include <memory>
#include <string>

#include "application/tests/support/fakes.hpp"

namespace tracer_core::application::tests {

struct TestState {
  int failures = 0;
};

auto BuildCoreApiForTest(FakeWorkflowHandler& workflow_handler,
                         FakeReportHandler& report_handler) -> TracerCoreApi;

auto BuildCoreApiForTest(
    FakeWorkflowHandler& workflow_handler, FakeReportHandler& report_handler,
    const std::shared_ptr<FakeDataQueryService>& data_query) -> TracerCoreApi;

inline auto Expect(TestState& state, bool condition, const std::string& message)
    -> void {
  if (condition) {
    return;
  }
  ++state.failures;
  std::cerr << "[FAIL] " << message << '\n';
}

[[nodiscard]] inline auto Contains(const std::string& text,
                                   const std::string& keyword) -> bool {
  return text.find(keyword) != std::string::npos;
}

}  // namespace tracer_core::application::tests

#endif  // APPLICATION_TESTS_SUPPORT_TEST_SUPPORT_H_
