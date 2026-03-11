module;

#include "infrastructure/sqlite_fwd.hpp"

#include <map>
#include <string>

#include "domain/reports/models/query_data_structs.hpp"
#include "domain/reports/types/report_types.hpp"
#include "infrastructure/config/models/report_catalog.hpp"

export module tracer.core.infrastructure.reports.querying.services.weekly_report_service;

export namespace tracer::core::infrastructure::reports::services {

#include "infrastructure/reports/services/detail/weekly_report_service_decl.inc"

}  // namespace tracer::core::infrastructure::reports::services
