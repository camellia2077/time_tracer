// infrastructure/tests/android_runtime/android_runtime_smoke_io_internal.hpp
#ifndef INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_ANDROID_RUNTIME_SMOKE_IO_INTERNAL_HPP_
#define INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_ANDROID_RUNTIME_SMOKE_IO_INTERNAL_HPP_

#include "application/use_cases/i_tracer_core_api.hpp"

#include <memory>
#include <optional>
#include <string>

namespace android_runtime_tests::smoke {

struct ChartProbeContext {
  std::optional<std::string> latest_date;
  std::optional<std::string> earliest_date;
  std::optional<std::string> selected_root;
};

auto ProbeChartRange(const std::shared_ptr<ITracerCoreApi>& core_api,
                     ChartProbeContext& chart_probe, int& failures) -> void;
auto VerifyExplicitChartRange(const std::shared_ptr<ITracerCoreApi>& core_api,
                              const ChartProbeContext& chart_probe,
                              int& failures) -> void;
auto VerifyChartForRootScenarios(
    const std::shared_ptr<ITracerCoreApi>& core_api,
    const ChartProbeContext& chart_probe, int& failures) -> void;
auto VerifyReportOutputs(const std::shared_ptr<ITracerCoreApi>& core_api,
                         int& failures) -> void;

}  // namespace android_runtime_tests::smoke

#endif  // INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_ANDROID_RUNTIME_SMOKE_IO_INTERNAL_HPP_
