module;

#include <exception>
#include <string>
#include <string_view>

#include "application/dto/core_responses.hpp"
#include "domain/reports/types/report_types.hpp"

namespace tracer_core::application::ports {
class IReportDtoFormatter;
}  // namespace tracer_core::application::ports

export module tracer.core.application.use_cases.helpers;

export namespace tracer::core::application::use_cases::helpers {

#include "application/use_cases/detail/tracer_core_api_helpers_decl.inc"

}  // namespace tracer::core::application::use_cases::helpers

export namespace tracer::core::application::modusecases::helpers {

using tracer::core::application::use_cases::helpers::BuildErrorMessage;
using tracer::core::application::use_cases::helpers::BuildOperationFailure;
using tracer::core::application::use_cases::helpers::BuildTextFailure;
using tracer::core::application::use_cases::helpers::BuildTreeFailure;
using tracer::core::application::use_cases::helpers::
    BuildStructuredReportFailure;
using tracer::core::application::use_cases::helpers::
    BuildStructuredPeriodBatchFailure;
using tracer::core::application::use_cases::helpers::FormatStructuredReport;
using tracer::core::application::use_cases::helpers::BuildPeriodBatchErrorLine;

}  // namespace tracer::core::application::modusecases::helpers
