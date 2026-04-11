// application/tests/modules/query_tests.hpp
#ifndef APPLICATION_TESTS_MODULES_QUERY_TESTS_H_
#define APPLICATION_TESTS_MODULES_QUERY_TESTS_H_

#include "application/tests/support/test_support.hpp"

namespace tracer_core::application::tests {

auto RunDataQueryTests(TestState& state) -> void;
auto RunQuerySemanticsTests(TestState& state) -> void;

}  // namespace tracer_core::application::tests

#endif  // APPLICATION_TESTS_MODULES_QUERY_TESTS_H_
