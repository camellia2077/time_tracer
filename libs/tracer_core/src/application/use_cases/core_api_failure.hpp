#ifndef APPLICATION_USE_CASES_CORE_API_FAILURE_HPP_
#define APPLICATION_USE_CASES_CORE_API_FAILURE_HPP_

#include <exception>
#include <string>
#include <string_view>

#include "application/dto/query_responses.hpp"
#include "application/dto/shared_envelopes.hpp"

namespace tracer::core::application::use_cases::failure {

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

}  // namespace tracer::core::application::use_cases::failure

namespace tracer_core::application::use_cases {

namespace core_api_failure = tracer::core::application::use_cases::failure;

}  // namespace tracer_core::application::use_cases

#endif  // APPLICATION_USE_CASES_CORE_API_FAILURE_HPP_
