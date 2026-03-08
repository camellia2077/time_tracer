// infrastructure/tests/android_runtime/android_runtime_report_consistency_internal.hpp
#ifndef INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_ANDROID_RUNTIME_REPORT_CONSISTENCY_INTERNAL_HPP_
#define INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_ANDROID_RUNTIME_REPORT_CONSISTENCY_INTERNAL_HPP_

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

#include "application/use_cases/i_tracer_core_api.hpp"

namespace android_runtime_tests::report_consistency_internal {

auto ComputeSha256Hex(std::string_view text) -> std::string;
auto BuildDiffDiagnostics(std::string_view left, std::string_view right)
    -> std::string;

auto RunReportConsistencyFieldVerificationTests(
    const std::shared_ptr<ITracerCoreApi>& core_api, int& failures) -> void;
auto RunReportConsistencyCrossIngestTests(
    const std::shared_ptr<ITracerCoreApi>& core_api,
    const std::filesystem::path& input_path, int& failures) -> void;
auto RunReportConsistencyStructureTests(
    const std::shared_ptr<ITracerCoreApi>& core_api, int& failures) -> void;

}  // namespace android_runtime_tests::report_consistency_internal

#endif  // INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_ANDROID_RUNTIME_REPORT_CONSISTENCY_INTERNAL_HPP_
