module;

#include <filesystem>
#include <memory>

#include "application/interfaces/i_report_query_service.hpp"
#include "application/ports/i_platform_clock.hpp"
#include "infrastructure/config/models/report_catalog.hpp"

export module tracer.core.infrastructure.reports.querying.lazy_sqlite_report_query_service;

export namespace tracer::core::infrastructure::reports {

#include "infrastructure/reports/detail/lazy_sqlite_report_query_service_decl.inc"

}  // namespace tracer::core::infrastructure::reports
