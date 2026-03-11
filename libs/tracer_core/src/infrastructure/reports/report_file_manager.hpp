// infrastructure/reports/report_file_manager.hpp
#ifndef INFRASTRUCTURE_REPORTS_REPORT_FILE_MANAGER_H_
#define INFRASTRUCTURE_REPORTS_REPORT_FILE_MANAGER_H_

#if TT_ENABLE_CPP20_MODULES && !defined(TT_FORCE_LEGACY_HEADER_DECLS)
import tracer.core.infrastructure.reports.exporting.report_file_manager;
#else
#include <filesystem>
#include <string>
#include <vector>

#include "domain/reports/types/report_types.hpp"

namespace tracer::core::infrastructure::reports {

#include "infrastructure/reports/detail/report_file_manager_decl.inc"

}  // namespace tracer::core::infrastructure::reports
#endif

using ReportFileManager =
    tracer::core::infrastructure::reports::ReportFileManager;

#endif  // INFRASTRUCTURE_REPORTS_REPORT_FILE_MANAGER_H_
