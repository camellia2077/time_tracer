// infrastructure/reports/services/weekly_report_service.hpp
#ifndef INFRASTRUCTURE_REPORTS_SERVICES_WEEKLY_REPORT_SERVICE_H_
#define INFRASTRUCTURE_REPORTS_SERVICES_WEEKLY_REPORT_SERVICE_H_

#if TT_ENABLE_CPP20_MODULES && !defined(TT_FORCE_LEGACY_HEADER_DECLS)
import tracer.core.infrastructure.reports.querying.services.weekly_report_service;
#else
#include "infrastructure/sqlite_fwd.hpp"

#include <map>
#include <string>

#include "domain/reports/models/query_data_structs.hpp"
#include "domain/reports/types/report_types.hpp"
#include "infrastructure/config/models/report_catalog.hpp"

namespace tracer::core::infrastructure::reports::services {

#include "infrastructure/reports/services/detail/weekly_report_service_decl.inc"

}  // namespace tracer::core::infrastructure::reports::services
#endif

namespace infrastructure::reports::services {

using tracer::core::infrastructure::reports::services::WeeklyReportService;

}  // namespace infrastructure::reports::services

using WeeklyReportService =
    tracer::core::infrastructure::reports::services::WeeklyReportService;

#endif  // INFRASTRUCTURE_REPORTS_SERVICES_WEEKLY_REPORT_SERVICE_H_

