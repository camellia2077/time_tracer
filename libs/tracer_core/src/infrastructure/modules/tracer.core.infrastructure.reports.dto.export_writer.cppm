module;

#include <map>
#include <memory>
#include <string>

#include "application/interfaces/i_report_exporter.hpp"
#include "infrastructure/config/models/report_catalog.hpp"
#include "application/ports/i_report_dto_formatter.hpp"
#include "application/ports/i_report_export_writer.hpp"

export module tracer.core.infrastructure.reports.dto.export_writer;

export namespace tracer::core::infrastructure::reports {

#include "infrastructure/reports/detail/report_dto_export_writer_decl.inc"

}  // namespace tracer::core::infrastructure::reports
