// application/tests/modules/test_modules.hpp
#ifndef APPLICATION_TESTS_MODULES_TEST_MODULES_H_
#define APPLICATION_TESTS_MODULES_TEST_MODULES_H_

#include "application/tests/support/test_support.hpp"

namespace time_tracer::application::tests {

auto RunConvertIngestValidateTests(TestState& state) -> void;
auto RunReportTests(TestState& state) -> void;
auto RunDataQueryTests(TestState& state) -> void;
auto RunImportServiceTests(TestState& state) -> void;

}  // namespace time_tracer::application::tests

#endif  // APPLICATION_TESTS_MODULES_TEST_MODULES_H_
