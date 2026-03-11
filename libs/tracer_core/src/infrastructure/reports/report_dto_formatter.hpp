// infrastructure/reports/report_dto_formatter.hpp
#ifndef INFRASTRUCTURE_REPORTS_REPORT_DTO_FORMATTER_H_
#define INFRASTRUCTURE_REPORTS_REPORT_DTO_FORMATTER_H_

#if TT_ENABLE_CPP20_MODULES && !defined(TT_FORCE_LEGACY_HEADER_DECLS)
import tracer.core.infrastructure.reports.dto.formatter;
#else
#include <map>
#include <memory>

#include "application/ports/i_report_dto_formatter.hpp"
#include "infrastructure/config/models/report_catalog.hpp"
#include "infrastructure/reports/shared/interfaces/i_report_formatter.hpp"

namespace tracer::core::infrastructure::reports {

#include "infrastructure/reports/detail/report_dto_formatter_decl.inc"

}  // namespace tracer::core::infrastructure::reports
#endif

namespace infrastructure::reports {

using tracer::core::infrastructure::reports::ReportDtoFormatter;

}  // namespace infrastructure::reports

#endif  // INFRASTRUCTURE_REPORTS_REPORT_DTO_FORMATTER_H_
