module;

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "application/interfaces/i_report_exporter.hpp"

export module tracer.core.infrastructure.reports.exporting.exporter;

export import tracer.core.infrastructure.reports.exporting.report_file_manager;

export namespace tracer::core::infrastructure::reports {

#include "infrastructure/reports/detail/exporter_decl.inc"

}  // namespace tracer::core::infrastructure::reports
