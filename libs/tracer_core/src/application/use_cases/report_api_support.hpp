#ifndef APPLICATION_USE_CASES_REPORT_API_SUPPORT_HPP_
#define APPLICATION_USE_CASES_REPORT_API_SUPPORT_HPP_

#include <exception>
#include <string>
#include <string_view>

#include "application/dto/reporting_responses.hpp"
#include "application/dto/shared_envelopes.hpp"
#include "domain/reports/types/report_types.hpp"

namespace tracer_core::application::ports {
class IReportDtoFormatter;
}  // namespace tracer_core::application::ports

namespace tracer::core::application::use_cases::report_support {

struct DateRangeArgument {
  std::string start_date;
  std::string end_date;
};

auto ParseRecentDaysArgument(std::string_view argument) -> int;
auto ParseRangeArgument(std::string_view argument) -> DateRangeArgument;

auto BuildStructuredReportFailure(std::string_view operation,
                                  std::string_view details)
    -> tracer_core::core::dto::StructuredReportOutput;
auto BuildStructuredReportFailure(std::string_view operation,
                                  const std::exception& exception)
    -> tracer_core::core::dto::StructuredReportOutput;
auto BuildStructuredReportFailure(std::string_view operation)
    -> tracer_core::core::dto::StructuredReportOutput;

auto BuildStructuredPeriodBatchFailure(std::string_view operation,
                                       std::string_view details)
    -> tracer_core::core::dto::StructuredPeriodBatchOutput;
auto BuildStructuredPeriodBatchFailure(std::string_view operation,
                                       const std::exception& exception)
    -> tracer_core::core::dto::StructuredPeriodBatchOutput;
auto BuildStructuredPeriodBatchFailure(std::string_view operation)
    -> tracer_core::core::dto::StructuredPeriodBatchOutput;

auto FormatStructuredReport(
    const tracer_core::core::dto::StructuredReportOutput& output,
    ReportFormat format,
    tracer_core::application::ports::IReportDtoFormatter& formatter)
    -> tracer_core::core::dto::TextOutput;

auto BuildPeriodBatchErrorLine(int days, std::string_view details)
    -> std::string;

}  // namespace tracer::core::application::use_cases::report_support

namespace tracer_core::application::use_cases {

namespace report_api_support =
    tracer::core::application::use_cases::report_support;

}  // namespace tracer_core::application::use_cases

#endif  // APPLICATION_USE_CASES_REPORT_API_SUPPORT_HPP_
