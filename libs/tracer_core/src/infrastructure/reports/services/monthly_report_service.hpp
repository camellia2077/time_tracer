// infrastructure/reports/services/monthly_report_service.hpp
#ifndef INFRASTRUCTURE_REPORTS_SERVICES_MONTHLY_REPORT_SERVICE_H_
#define INFRASTRUCTURE_REPORTS_SERVICES_MONTHLY_REPORT_SERVICE_H_

#if TT_ENABLE_CPP20_MODULES && !defined(TT_FORCE_LEGACY_HEADER_DECLS)
import tracer.core.infrastructure.reports.querying.services.monthly_report_service;
#else
#include "infrastructure/sqlite_fwd.hpp"

#include <map>
#include <string>

#include "domain/reports/models/query_data_structs.hpp"
#include "domain/reports/types/report_types.hpp"
#include "infrastructure/config/models/report_catalog.hpp"

namespace tracer::core::infrastructure::reports::services {

#include "infrastructure/reports/services/detail/monthly_report_service_decl.inc"

}  // namespace tracer::core::infrastructure::reports::services
#endif

namespace infrastructure::reports::services {

using tracer::core::infrastructure::reports::services::MonthlyReportService;

}  // namespace infrastructure::reports::services

using MonthlyReportService =
    tracer::core::infrastructure::reports::services::MonthlyReportService;

#endif  // INFRASTRUCTURE_REPORTS_SERVICES_MONTHLY_REPORT_SERVICE_H_

