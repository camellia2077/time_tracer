module;

#include <filesystem>
#include <string>
#include <vector>

#include "domain/reports/types/report_types.hpp"

export module tracer.core.infrastructure.reports.exporting.report_file_manager;

export namespace tracer::core::infrastructure::reports {

#include "infrastructure/reports/detail/report_file_manager_decl.inc"

}  // namespace tracer::core::infrastructure::reports
