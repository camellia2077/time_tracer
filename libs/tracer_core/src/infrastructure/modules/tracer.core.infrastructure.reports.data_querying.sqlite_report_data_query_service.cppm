module;

#include "infrastructure/sqlite_fwd.hpp"

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "application/ports/i_platform_clock.hpp"
#include "application/ports/i_report_data_query_service.hpp"

export module tracer.core.infrastructure.reports.data_querying.sqlite_report_data_query_service;

export namespace tracer::core::infrastructure::reports {

#include "infrastructure/reports/detail/sqlite_report_data_query_service_decl.inc"

}  // namespace tracer::core::infrastructure::reports
