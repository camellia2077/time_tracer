// infrastructure/tests/android_runtime/android_runtime_insights_consistency_internal.hpp
#ifndef INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_ANDROID_RUNTIME_INSIGHTS_CONSISTENCY_INTERNAL_HPP_
#define INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_ANDROID_RUNTIME_INSIGHTS_CONSISTENCY_INTERNAL_HPP_

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

#include "application/aggregate_runtime/i_tracer_core_runtime.hpp"

namespace android_runtime_tests::insights_consistency_internal {

auto ComputeSha256Hex(std::string_view text) -> std::string;
auto BuildDiffDiagnostics(std::string_view left, std::string_view right)
    -> std::string;

auto RunInsightsConsistencyFieldVerificationTests(
    const std::shared_ptr<
        tracer::core::application::use_cases::ITracerCoreRuntime>& runtime_api,
    int& failures) -> void;
auto RunInsightsConsistencyCrossIngestTests(
    const std::shared_ptr<
        tracer::core::application::use_cases::ITracerCoreRuntime>& runtime_api,
    const std::filesystem::path& input_path, int& failures) -> void;
auto RunInsightsConsistencyStructureTests(
    const std::shared_ptr<
        tracer::core::application::use_cases::ITracerCoreRuntime>& runtime_api,
    int& failures) -> void;

}  // namespace android_runtime_tests::insights_consistency_internal

#endif  // INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_ANDROID_RUNTIME_INSIGHTS_CONSISTENCY_INTERNAL_HPP_
