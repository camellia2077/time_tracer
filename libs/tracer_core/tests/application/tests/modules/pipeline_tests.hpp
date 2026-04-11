// application/tests/modules/pipeline_tests.hpp
#ifndef APPLICATION_TESTS_MODULES_PIPELINE_TESTS_H_
#define APPLICATION_TESTS_MODULES_PIPELINE_TESTS_H_

#include "application/tests/support/test_support.hpp"

namespace tracer_core::application::tests {

auto RunConvertIngestValidateTests(TestState& state) -> void;
auto RunImportServiceTests(TestState& state) -> void;
auto RunRecordTimeOrderModeTests(TestState& state) -> void;
auto RunTxtDayBlockTests(TestState& state) -> void;

}  // namespace tracer_core::application::tests

#endif  // APPLICATION_TESTS_MODULES_PIPELINE_TESTS_H_
