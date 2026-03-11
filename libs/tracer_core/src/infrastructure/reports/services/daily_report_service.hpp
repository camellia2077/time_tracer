// infrastructure/reports/services/daily_report_service.hpp
#ifndef INFRASTRUCTURE_REPORTS_SERVICES_DAILY_REPORT_SERVICE_H_
#define INFRASTRUCTURE_REPORTS_SERVICES_DAILY_REPORT_SERVICE_H_

#if TT_ENABLE_CPP20_MODULES && !defined(TT_FORCE_LEGACY_HEADER_DECLS)
import tracer.core.infrastructure.reports.querying.services.daily_report_service;
#else
#include "infrastructure/sqlite_fwd.hpp"

#include <string>

#include "domain/reports/models/query_data_structs.hpp"
#include "domain/reports/types/report_types.hpp"
#include "infrastructure/config/models/report_catalog.hpp"

namespace tracer::core::infrastructure::reports::services {

#include "infrastructure/reports/services/detail/daily_report_service_decl.inc"

}  // namespace tracer::core::infrastructure::reports::services
#endif

namespace infrastructure::reports::services {

using tracer::core::infrastructure::reports::services::DailyReportService;

}  // namespace infrastructure::reports::services

using DailyReportService =
    tracer::core::infrastructure::reports::services::DailyReportService;

#endif  // INFRASTRUCTURE_REPORTS_SERVICES_DAILY_REPORT_SERVICE_H_

