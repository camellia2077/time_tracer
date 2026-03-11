// application/use_cases/tracer_core_api.hpp
#ifndef APPLICATION_USE_CASES_TRACER_CORE_API_H_
#define APPLICATION_USE_CASES_TRACER_CORE_API_H_

#include <memory>

#include "application/use_cases/i_tracer_core_api.hpp"

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

namespace tracer::core::application::use_cases {

#include "application/use_cases/detail/tracer_core_api_decl.inc"

}  // namespace tracer::core::application::use_cases

using TracerCoreApi = tracer::core::application::use_cases::TracerCoreApi;

#endif  // APPLICATION_USE_CASES_TRACER_CORE_API_H_
