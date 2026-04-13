// application/use_cases/tracer_core_api_helpers.cpp
#include "application/use_cases/tracer_core_api_helpers.hpp"

namespace tracer::core::application::use_cases::helpers {

using tracer_core::core::dto::OperationAck;
using tracer_core::core::dto::StructuredPeriodBatchOutput;
using tracer_core::core::dto::TextOutput;
using tracer_core::core::dto::TreeQueryResponse;

auto BuildErrorMessage(std::string_view operation, std::string_view details)
    -> std::string {
  if (details.empty()) {
    return std::string(operation) + " failed.";
  }
  return std::string(operation) + " failed: " + std::string(details);
}

auto BuildOperationFailure(std::string_view operation,
                           const std::exception& exception) -> OperationAck {
  return {.ok = false,
          .error_message = BuildErrorMessage(operation, exception.what())};
}

auto BuildOperationFailure(std::string_view operation) -> OperationAck {
  return {.ok = false,
          .error_message =
              BuildErrorMessage(operation, "Unknown non-standard exception.")};
}

auto BuildTextFailure(std::string_view operation,
                      const std::exception& exception) -> TextOutput {
  return {.ok = false,
          .content = "",
          .error_message = BuildErrorMessage(operation, exception.what())};
}

auto BuildTextFailure(std::string_view operation, std::string_view details)
    -> TextOutput {
  return {.ok = false,
          .content = "",
          .error_message = BuildErrorMessage(operation, details)};
}

auto BuildTextFailure(std::string_view operation) -> TextOutput {
  return {.ok = false,
          .content = "",
          .error_message =
              BuildErrorMessage(operation, "Unknown non-standard exception.")};
}

auto BuildTreeFailure(std::string_view operation,
                      const std::exception& exception) -> TreeQueryResponse {
  return {.ok = false,
          .found = false,
          .roots = {},
          .tree = {},
          .error_message = BuildErrorMessage(operation, exception.what())};
}

auto BuildTreeFailure(std::string_view operation) -> TreeQueryResponse {
  return {
      .ok = false,
      .found = false,
      .roots = {},
      .tree = {},
      .error_message =
          BuildErrorMessage(operation, "Unknown non-standard exception."),
  };
}

auto BuildStructuredPeriodBatchFailure(std::string_view operation,
                                       std::string_view details)
    -> StructuredPeriodBatchOutput {
  return {.ok = false,
          .items = {},
          .error_message = BuildErrorMessage(operation, details)};
}

auto BuildStructuredPeriodBatchFailure(std::string_view operation,
                                       const std::exception& exception)
    -> StructuredPeriodBatchOutput {
  return BuildStructuredPeriodBatchFailure(operation, exception.what());
}

auto BuildStructuredPeriodBatchFailure(std::string_view operation)
    -> StructuredPeriodBatchOutput {
  return BuildStructuredPeriodBatchFailure(operation,
                                           "Unknown non-standard exception.");
}

auto BuildPeriodBatchErrorLine(int days, std::string_view details)
    -> std::string {
  if (details.empty()) {
    return "Error querying period " + std::to_string(days) + " days.";
  }
  return "Error querying period " + std::to_string(days) +
         " days: " + std::string(details);
}

}  // namespace tracer::core::application::use_cases::helpers
