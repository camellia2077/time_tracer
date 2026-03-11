// infrastructure/reports/sqlite_report_data_query_service.hpp
#ifndef INFRASTRUCTURE_REPORTS_SQLITE_REPORT_DATA_QUERY_SERVICE_H_
#define INFRASTRUCTURE_REPORTS_SQLITE_REPORT_DATA_QUERY_SERVICE_H_

#if TT_ENABLE_CPP20_MODULES && !defined(TT_FORCE_LEGACY_HEADER_DECLS)
import tracer.core.infrastructure.reports.data_querying.sqlite_report_data_query_service;
#else
#include "infrastructure/sqlite_fwd.hpp"

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "application/ports/i_platform_clock.hpp"
#include "application/ports/i_report_data_query_service.hpp"

namespace tracer::core::infrastructure::reports {

#include "infrastructure/reports/detail/sqlite_report_data_query_service_decl.inc"

}  // namespace tracer::core::infrastructure::reports
#endif

namespace infrastructure::reports {

using tracer::core::infrastructure::reports::SqliteReportDataQueryService;

}  // namespace infrastructure::reports

#endif  // INFRASTRUCTURE_REPORTS_SQLITE_REPORT_DATA_QUERY_SERVICE_H_

