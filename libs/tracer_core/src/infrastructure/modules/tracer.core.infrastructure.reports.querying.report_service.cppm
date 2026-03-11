module;

#include "infrastructure/sqlite_fwd.hpp"

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "application/interfaces/i_report_query_service.hpp"
#include "application/ports/i_platform_clock.hpp"
#include "domain/reports/models/period_report_models.hpp"
#include "infrastructure/config/models/report_catalog.hpp"
#include "infrastructure/reports/shared/interfaces/i_report_formatter.hpp"

export module tracer.core.infrastructure.reports.querying.report_service;

export namespace tracer::core::infrastructure::reports {

#include "infrastructure/reports/detail/report_service_decl.inc"

}  // namespace tracer::core::infrastructure::reports
