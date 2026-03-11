// infrastructure/reports/report_dto_export_writer.hpp
#ifndef INFRASTRUCTURE_REPORTS_REPORT_DTO_EXPORT_WRITER_H_
#define INFRASTRUCTURE_REPORTS_REPORT_DTO_EXPORT_WRITER_H_

#if TT_ENABLE_CPP20_MODULES && !defined(TT_FORCE_LEGACY_HEADER_DECLS)
import tracer.core.infrastructure.reports.dto.export_writer;
#else
#include <map>
#include <memory>
#include <string>

#include "application/interfaces/i_report_exporter.hpp"
#include "application/ports/i_report_dto_formatter.hpp"
#include "application/ports/i_report_export_writer.hpp"

namespace tracer::core::infrastructure::reports {

#include "infrastructure/reports/detail/report_dto_export_writer_decl.inc"

}  // namespace tracer::core::infrastructure::reports
#endif

namespace infrastructure::reports {

using tracer::core::infrastructure::reports::ReportDtoExportWriter;

}  // namespace infrastructure::reports

#endif  // INFRASTRUCTURE_REPORTS_REPORT_DTO_EXPORT_WRITER_H_
