// infrastructure/reports/facade/android_static_report_formatter_registrar.cpp
#include "infrastructure/reports/facade/android_static_report_formatter_registrar.hpp"
#include "infrastructure/reports/facade/android_static_report_formatter_registrar_internal.hpp"

namespace infrastructure::reports {

AndroidStaticReportFormatterRegistrar::AndroidStaticReportFormatterRegistrar(
    AndroidStaticReportFormatterPolicy policy)
    : policy_(policy) {}

auto AndroidStaticReportFormatterRegistrar::RegisterStaticFormatters() const
    -> void {
  detail::RegisterMarkdownFormatters(policy_);
  detail::RegisterLatexFormatters(policy_);
  detail::RegisterTypstFormatters(policy_);
}

}  // namespace infrastructure::reports
