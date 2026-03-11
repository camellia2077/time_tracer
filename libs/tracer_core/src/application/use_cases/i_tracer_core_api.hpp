// application/use_cases/i_tracer_core_api.hpp
#ifndef APPLICATION_USE_CASES_I_TRACER_CORE_API_H_
#define APPLICATION_USE_CASES_I_TRACER_CORE_API_H_

#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"

namespace tracer::core::application::use_cases {

#include "application/use_cases/detail/i_tracer_core_api_decl.inc"

}  // namespace tracer::core::application::use_cases

using ITracerCoreApi = tracer::core::application::use_cases::ITracerCoreApi;

#endif  // APPLICATION_USE_CASES_I_TRACER_CORE_API_H_
