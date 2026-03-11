// infrastructure/reports/exporter.hpp
#ifndef INFRASTRUCTURE_REPORTS_EXPORTER_H_
#define INFRASTRUCTURE_REPORTS_EXPORTER_H_

#if TT_ENABLE_CPP20_MODULES && !defined(TT_FORCE_LEGACY_HEADER_DECLS)
import tracer.core.infrastructure.reports.exporting.exporter;
#else
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "application/interfaces/i_report_exporter.hpp"
#include "infrastructure/reports/report_file_manager.hpp"

namespace tracer::core::infrastructure::reports {

#include "infrastructure/reports/detail/exporter_decl.inc"

}  // namespace tracer::core::infrastructure::reports
#endif

using Exporter = tracer::core::infrastructure::reports::Exporter;

#endif  // INFRASTRUCTURE_REPORTS_EXPORTER_H_
