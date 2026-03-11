#ifndef INFRASTRUCTURE_REPORTS_LAZY_SQLITE_REPORT_DATA_QUERY_SERVICE_H_
#define INFRASTRUCTURE_REPORTS_LAZY_SQLITE_REPORT_DATA_QUERY_SERVICE_H_

#if TT_ENABLE_CPP20_MODULES && !defined(TT_FORCE_LEGACY_HEADER_DECLS)
import tracer.core.infrastructure.reports.data_querying.lazy_sqlite_report_data_query_service;
#else
#include <filesystem>
#include <memory>

#include "application/ports/i_platform_clock.hpp"
#include "application/ports/i_report_data_query_service.hpp"

namespace tracer::core::infrastructure::reports {

#include "infrastructure/reports/detail/lazy_sqlite_report_data_query_service_decl.inc"

}  // namespace tracer::core::infrastructure::reports
#endif

namespace infrastructure::reports {

using tracer::core::infrastructure::reports::LazySqliteReportDataQueryService;

}  // namespace infrastructure::reports

#endif  // INFRASTRUCTURE_REPORTS_LAZY_SQLITE_REPORT_DATA_QUERY_SERVICE_H_
