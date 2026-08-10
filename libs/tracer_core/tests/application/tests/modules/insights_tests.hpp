// application/tests/modules/insights_tests.hpp
#ifndef APPLICATION_TESTS_MODULES_INSIGHTS_TESTS_H_
#define APPLICATION_TESTS_MODULES_INSIGHTS_TESTS_H_

#include "application/tests/support/test_support.hpp"

namespace tracer_core::application::tests {

auto RunInsightsTests(TestState& state) -> void;
auto RunInsightsSemanticsTests(TestState& state) -> void;

}  // namespace tracer_core::application::tests

#endif  // APPLICATION_TESTS_MODULES_INSIGHTS_TESTS_H_
