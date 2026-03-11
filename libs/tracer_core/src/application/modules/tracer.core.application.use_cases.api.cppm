module;

#include <memory>

#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"

class IReportHandler;
class IProjectRepository;
namespace tracer::core::application::workflow {
class IWorkflowHandler;
}
namespace tracer_core::application::ports {
class IDataQueryService;
class IReportDataQueryService;
class IReportDtoFormatter;
class IReportExportWriter;
}  // namespace tracer_core::application::ports

export module tracer.core.application.use_cases.api;

export import tracer.core.application.use_cases.interface;

export namespace tracer::core::application::use_cases {

#include "application/use_cases/detail/tracer_core_api_decl.inc"

}  // namespace tracer::core::application::use_cases

export namespace tracer::core::application::modusecases {

using tracer::core::application::use_cases::TracerCoreApi;

}  // namespace tracer::core::application::modusecases
