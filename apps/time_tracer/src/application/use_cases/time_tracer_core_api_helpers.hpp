// application/use_cases/time_tracer_core_api_helpers.hpp
#ifndef APPLICATION_USE_CASES_TIME_TRACER_CORE_API_HELPERS_H_
#define APPLICATION_USE_CASES_TIME_TRACER_CORE_API_HELPERS_H_

#include <exception>
#include <string>
#include <string_view>

#include "application/dto/core_responses.hpp"
#include "domain/reports/types/report_types.hpp"

namespace time_tracer::application::ports {
class IReportDtoFormatter;
}  // namespace time_tracer::application::ports

namespace time_tracer::application::use_cases::core_api_helpers {

auto BuildErrorMessage(std::string_view operation, std::string_view details)
    -> std::string;

auto BuildOperationFailure(std::string_view operation,
                           const std::exception& exception)
    -> time_tracer::core::dto::OperationAck;
auto BuildOperationFailure(std::string_view operation)
    -> time_tracer::core::dto::OperationAck;

auto BuildTextFailure(std::string_view operation,
                      const std::exception& exception)
    -> time_tracer::core::dto::TextOutput;
auto BuildTextFailure(std::string_view operation, std::string_view details)
    -> time_tracer::core::dto::TextOutput;
auto BuildTextFailure(std::string_view operation)
    -> time_tracer::core::dto::TextOutput;

auto BuildTreeFailure(std::string_view operation,
                      const std::exception& exception)
    -> time_tracer::core::dto::TreeQueryResponse;
auto BuildTreeFailure(std::string_view operation)
    -> time_tracer::core::dto::TreeQueryResponse;

auto BuildStructuredReportFailure(std::string_view operation,
                                  std::string_view details)
    -> time_tracer::core::dto::StructuredReportOutput;
auto BuildStructuredReportFailure(std::string_view operation,
                                  const std::exception& exception)
    -> time_tracer::core::dto::StructuredReportOutput;
auto BuildStructuredReportFailure(std::string_view operation)
    -> time_tracer::core::dto::StructuredReportOutput;

auto BuildStructuredPeriodBatchFailure(std::string_view operation,
                                       std::string_view details)
    -> time_tracer::core::dto::StructuredPeriodBatchOutput;
auto BuildStructuredPeriodBatchFailure(std::string_view operation,
                                       const std::exception& exception)
    -> time_tracer::core::dto::StructuredPeriodBatchOutput;
auto BuildStructuredPeriodBatchFailure(std::string_view operation)
    -> time_tracer::core::dto::StructuredPeriodBatchOutput;

auto FormatStructuredReport(
    const time_tracer::core::dto::StructuredReportOutput& output,
    ReportFormat format,
    time_tracer::application::ports::IReportDtoFormatter& formatter)
    -> time_tracer::core::dto::TextOutput;

auto BuildPeriodBatchErrorLine(int days, std::string_view details)
    -> std::string;

}  // namespace time_tracer::application::use_cases::core_api_helpers

#endif  // APPLICATION_USE_CASES_TIME_TRACER_CORE_API_HELPERS_H_
