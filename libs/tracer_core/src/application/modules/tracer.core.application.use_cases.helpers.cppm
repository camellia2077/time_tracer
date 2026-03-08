module;

#include "application/use_cases/tracer_core_api_helpers.hpp"

export module tracer.core.application.use_cases.helpers;

export namespace tracer::core::application::modusecases::helpers {

using ::tracer_core::application::use_cases::core_api_helpers::
    BuildErrorMessage;
using ::tracer_core::application::use_cases::core_api_helpers::
    BuildOperationFailure;
using ::tracer_core::application::use_cases::core_api_helpers::
    BuildPeriodBatchErrorLine;
using ::tracer_core::application::use_cases::core_api_helpers::
    BuildStructuredPeriodBatchFailure;
using ::tracer_core::application::use_cases::core_api_helpers::
    BuildStructuredReportFailure;
using ::tracer_core::application::use_cases::core_api_helpers::
    BuildTextFailure;
using ::tracer_core::application::use_cases::core_api_helpers::
    BuildTreeFailure;
using ::tracer_core::application::use_cases::core_api_helpers::
    FormatStructuredReport;

}  // namespace tracer::core::application::modusecases::helpers
