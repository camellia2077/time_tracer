// infrastructure/tests/android_runtime/android_runtime_smoke_query_internal.hpp
#ifndef INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_ANDROID_RUNTIME_SMOKE_QUERY_INTERNAL_HPP_
#define INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_ANDROID_RUNTIME_SMOKE_QUERY_INTERNAL_HPP_

#include "application/aggregate_runtime/i_tracer_core_runtime.hpp"

#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include <nlohmann/json.hpp>

#include "application/dto/query_requests.hpp"
#include "application/dto/query_responses.hpp"
#include "application/dto/shared_envelopes.hpp"

namespace android_runtime_tests::smoke {

[[nodiscard]] auto ParseJsonOrRecordFailure(const std::string& content,
                                            std::string_view context,
                                            int& failures)
    -> std::optional<nlohmann::json>;

auto ValidateChartSeriesPayload(const nlohmann::json& payload,
                                std::string_view context, int& failures)
    -> void;

[[nodiscard]] auto RunDataQueryOrRecordFailure(
    const std::shared_ptr<ITracerCoreRuntime>& runtime_api,
    const tracer_core::core::dto::DataQueryRequest& request,
    std::string_view context, int& failures)
    -> std::optional<tracer_core::core::dto::TextOutput>;

}  // namespace android_runtime_tests::smoke

#endif  // INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_ANDROID_RUNTIME_SMOKE_QUERY_INTERNAL_HPP_
