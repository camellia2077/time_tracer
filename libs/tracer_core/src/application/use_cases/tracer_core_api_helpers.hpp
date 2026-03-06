// application/use_cases/tracer_core_api_helpers.hpp
#ifndef APPLICATION_USE_CASES_TRACER_CORE_API_HELPERS_H_
#define APPLICATION_USE_CASES_TRACER_CORE_API_HELPERS_H_

#include <exception>
#include <string>
#include <string_view>

#include "application/dto/core_responses.hpp"
#include "domain/reports/types/report_types.hpp"

namespace tracer_core::application::ports {
class IReportDtoFormatter;
}  // namespace tracer_core::application::ports

namespace tracer_core::application::use_cases::core_api_helpers {

auto BuildErrorMessage(std::string_view operation, std::string_view details)
    -> std::string;

auto BuildOperationFailure(std::string_view operation,
                           const std::exception& exception)
    -> tracer_core::core::dto::OperationAck;
auto BuildOperationFailure(std::string_view operation)
    -> tracer_core::core::dto::OperationAck;

auto BuildTextFailure(std::string_view operation,
                      const std::exception& exception)
    -> tracer_core::core::dto::TextOutput;
auto BuildTextFailure(std::string_view operation, std::string_view details)
    -> tracer_core::core::dto::TextOutput;
auto BuildTextFailure(std::string_view operation)
    -> tracer_core::core::dto::TextOutput;

auto BuildTreeFailure(std::string_view operation,
                      const std::exception& exception)
    -> tracer_core::core::dto::TreeQueryResponse;
auto BuildTreeFailure(std::string_view operation)
    -> tracer_core::core::dto::TreeQueryResponse;

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

}  // namespace tracer_core::application::use_cases::core_api_helpers

#endif  // APPLICATION_USE_CASES_TRACER_CORE_API_HELPERS_H_
