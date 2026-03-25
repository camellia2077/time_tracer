module;

#include "application/use_cases/core_api_failure.hpp"
#include "application/use_cases/report_api_support.hpp"

export module tracer.core.application.use_cases.helpers;

export namespace tracer::core::application::use_cases::helpers {

using ::tracer::core::application::use_cases::failure::BuildErrorMessage;
using ::tracer::core::application::use_cases::failure::BuildOperationFailure;
using ::tracer::core::application::use_cases::failure::BuildTextFailure;
using ::tracer::core::application::use_cases::failure::BuildTreeFailure;
using ::tracer::core::application::use_cases::report_support::
    BuildPeriodBatchErrorLine;
using ::tracer::core::application::use_cases::report_support::
    BuildStructuredPeriodBatchFailure;
using ::tracer::core::application::use_cases::report_support::
    BuildStructuredReportFailure;
using ::tracer::core::application::use_cases::report_support::
    FormatStructuredReport;

}  // namespace tracer::core::application::use_cases::helpers

export namespace tracer::core::application::modusecases::helpers {

using tracer::core::application::use_cases::helpers::BuildErrorMessage;
using tracer::core::application::use_cases::helpers::BuildOperationFailure;
using tracer::core::application::use_cases::helpers::BuildPeriodBatchErrorLine;
using tracer::core::application::use_cases::helpers::
    BuildStructuredPeriodBatchFailure;
using tracer::core::application::use_cases::helpers::
    BuildStructuredReportFailure;
using tracer::core::application::use_cases::helpers::BuildTextFailure;
using tracer::core::application::use_cases::helpers::BuildTreeFailure;
using tracer::core::application::use_cases::helpers::FormatStructuredReport;

}  // namespace tracer::core::application::modusecases::helpers
