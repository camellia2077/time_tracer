#ifndef APPLICATION_USE_CASES_INSIGHTS_API_SUPPORT_HPP_
#define APPLICATION_USE_CASES_INSIGHTS_API_SUPPORT_HPP_

#include <exception>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

#include "application/dto/insights_requests.hpp"
#include "application/dto/insights_responses.hpp"
#include "application/dto/shared_envelopes.hpp"
#include "domain/insights/types/insights_types.hpp"

namespace tracer::core::application::use_cases::insights_support {

struct DateRangeArgument {
  std::string start_date;
  std::string end_date;
};

auto ParseRecentDaysArgument(std::string_view argument) -> int;
auto ParseRangeArgument(std::string_view argument) -> DateRangeArgument;

auto BuildStructuredPeriodBatchFailure(std::string_view operation,
                                       std::string_view details)
    -> tracer_core::core::dto::StructuredPeriodBatchOutput;
auto BuildStructuredPeriodBatchFailure(std::string_view operation,
                                       const std::exception& exception)
    -> tracer_core::core::dto::StructuredPeriodBatchOutput;
auto BuildStructuredPeriodBatchFailure(std::string_view operation)
    -> tracer_core::core::dto::StructuredPeriodBatchOutput;

auto BuildPeriodBatchErrorLine(int days, std::string_view details)
    -> std::string;

auto BuildDayPath(const std::filesystem::path& export_root,
                  InsightsFormat format, std::string_view date)
    -> std::filesystem::path;
auto BuildMonthPath(const std::filesystem::path& export_root,
                    InsightsFormat format, std::string_view month)
    -> std::filesystem::path;
auto BuildRecentPath(const std::filesystem::path& export_root,
                     InsightsFormat format, int days) -> std::filesystem::path;
auto BuildWeekPath(const std::filesystem::path& export_root,
                   InsightsFormat format, std::string_view iso_week)
    -> std::filesystem::path;
auto BuildYearPath(const std::filesystem::path& export_root,
                   InsightsFormat format, std::string_view year)
    -> std::filesystem::path;
auto BuildRangePath(const std::filesystem::path& export_root,
                    InsightsFormat format, std::string_view start_date,
                    std::string_view end_date) -> std::filesystem::path;
auto WriteExportFileIfNeeded(const std::filesystem::path& output_path,
                             std::string_view content) -> void;

}  // namespace tracer::core::application::use_cases::insights_support

namespace tracer_core::application::use_cases {

namespace insights_api_support =
    tracer::core::application::use_cases::insights_support;

}  // namespace tracer_core::application::use_cases

#endif  // APPLICATION_USE_CASES_INSIGHTS_API_SUPPORT_HPP_
