#include "application/use_cases/core_api_failure.hpp"

namespace tracer::core::application::use_cases::failure {

using namespace tracer_core::core::dto;

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
  return {.ok = false,
          .found = false,
          .roots = {},
          .tree = {},
          .error_message =
              BuildErrorMessage(operation, "Unknown non-standard exception.")};
}

}  // namespace tracer::core::application::use_cases::failure
