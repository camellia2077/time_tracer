module;

#include <map>
#include <memory>

#include "application/ports/i_report_dto_formatter.hpp"
#include "infrastructure/config/models/report_catalog.hpp"
#include "infrastructure/reports/shared/interfaces/i_report_formatter.hpp"

export module tracer.core.infrastructure.reports.dto.formatter;

export namespace tracer::core::infrastructure::reports {

#include "infrastructure/reports/detail/report_dto_formatter_decl.inc"

}  // namespace tracer::core::infrastructure::reports
