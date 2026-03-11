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

namespace tracer::core::application::use_cases::helpers {

#include "application/use_cases/detail/tracer_core_api_helpers_decl.inc"

}  // namespace tracer::core::application::use_cases::helpers

namespace tracer_core::application::use_cases {

namespace core_api_helpers = tracer::core::application::use_cases::helpers;

}  // namespace tracer_core::application::use_cases

#endif  // APPLICATION_USE_CASES_TRACER_CORE_API_HELPERS_H_
