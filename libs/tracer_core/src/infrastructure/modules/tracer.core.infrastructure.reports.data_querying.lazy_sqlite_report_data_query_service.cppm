module;

#include <filesystem>
#include <memory>

#include "application/ports/i_platform_clock.hpp"
#include "application/ports/i_report_data_query_service.hpp"

export module tracer.core.infrastructure.reports.data_querying.lazy_sqlite_report_data_query_service;

export namespace tracer::core::infrastructure::reports {

#include "infrastructure/reports/detail/lazy_sqlite_report_data_query_service_decl.inc"

}  // namespace tracer::core::infrastructure::reports
