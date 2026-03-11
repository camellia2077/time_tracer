// infrastructure/reports/export_utils.hpp
#ifndef INFRASTRUCTURE_REPORTS_EXPORT_UTILS_H_
#define INFRASTRUCTURE_REPORTS_EXPORT_UTILS_H_

// core/ExportUtils.hpp

#if TT_ENABLE_CPP20_MODULES && !defined(TT_FORCE_LEGACY_HEADER_DECLS)
import tracer.core.infrastructure.reports.exporting.export_utils;
#else
#include <filesystem>
#include <functional>
#include <optional>
#include <string>

#include "domain/reports/types/report_types.hpp"

namespace tracer::core::infrastructure::reports {

#include "infrastructure/reports/detail/export_utils_decl.inc"

}  // namespace tracer::core::infrastructure::reports
#endif

namespace ExportUtils {

using tracer::core::infrastructure::reports::ExecuteExportTask;
using tracer::core::infrastructure::reports::GetReportFormatDetails;
using tracer::core::infrastructure::reports::ReportFormatDetails;

}  // namespace ExportUtils

#endif  // INFRASTRUCTURE_REPORTS_EXPORT_UTILS_H_
