#ifndef INFRASTRUCTURE_REPORTS_LAZY_SQLITE_REPORT_QUERY_SERVICE_H_
#define INFRASTRUCTURE_REPORTS_LAZY_SQLITE_REPORT_QUERY_SERVICE_H_

#if TT_ENABLE_CPP20_MODULES && !defined(TT_FORCE_LEGACY_HEADER_DECLS)
import tracer.core.infrastructure.reports.querying.lazy_sqlite_report_query_service;
#else
#include <filesystem>
#include <memory>

#include "application/interfaces/i_report_query_service.hpp"
#include "application/ports/i_platform_clock.hpp"
#include "infrastructure/config/models/report_catalog.hpp"

namespace tracer::core::infrastructure::reports {

#include "infrastructure/reports/detail/lazy_sqlite_report_query_service_decl.inc"

}  // namespace tracer::core::infrastructure::reports
#endif

namespace infrastructure::reports {

using tracer::core::infrastructure::reports::LazySqliteReportQueryService;

}  // namespace infrastructure::reports

#endif  // INFRASTRUCTURE_REPORTS_LAZY_SQLITE_REPORT_QUERY_SERVICE_H_
