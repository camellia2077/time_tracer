// infrastructure/reports/report_service.hpp
#ifndef INFRASTRUCTURE_REPORTS_REPORT_SERVICE_H_
#define INFRASTRUCTURE_REPORTS_REPORT_SERVICE_H_

#if TT_ENABLE_CPP20_MODULES && !defined(TT_FORCE_LEGACY_HEADER_DECLS)
import tracer.core.infrastructure.reports.querying.report_service;
#else
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

namespace tracer::core::infrastructure::reports {

#include "infrastructure/reports/detail/report_service_decl.inc"

}  // namespace tracer::core::infrastructure::reports
#endif

namespace infrastructure::reports {

using tracer::core::infrastructure::reports::ReportService;

}  // namespace infrastructure::reports

using ReportService = tracer::core::infrastructure::reports::ReportService;

#endif  // INFRASTRUCTURE_REPORTS_REPORT_SERVICE_H_

