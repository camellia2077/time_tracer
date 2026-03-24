// infra/reporting/facade/android_static_report_formatter_registrar_internal.hpp
#ifndef INFRASTRUCTURE_REPORTS_FACADE_ANDROID_STATIC_REPORT_FORMATTER_REGISTRAR_INTERNAL_HPP_
#define INFRASTRUCTURE_REPORTS_FACADE_ANDROID_STATIC_REPORT_FORMATTER_REGISTRAR_INTERNAL_HPP_

#include "infra/reporting/facade/android_static_report_formatter_registrar.hpp"

namespace infrastructure::reports::detail {

auto RegisterMarkdownFormatters(
    const AndroidStaticReportFormatterPolicy& policy) -> void;
auto RegisterLatexFormatters(
    const AndroidStaticReportFormatterPolicy& policy) -> void;
auto RegisterTypstFormatters(const AndroidStaticReportFormatterPolicy& policy)
    -> void;

}  // namespace infrastructure::reports::detail

#endif  // INFRASTRUCTURE_REPORTS_FACADE_ANDROID_STATIC_REPORT_FORMATTER_REGISTRAR_INTERNAL_HPP_
