// infrastructure/reports/facade/android_static_report_formatter_registrar.hpp
#ifndef INFRASTRUCTURE_REPORTS_FACADE_ANDROID_STATIC_REPORT_FORMATTER_REGISTRAR_H_
#define INFRASTRUCTURE_REPORTS_FACADE_ANDROID_STATIC_REPORT_FORMATTER_REGISTRAR_H_

#include <cstdint>

#include "application/ports/i_report_formatter_registry.hpp"

namespace infrastructure::reports {

struct AndroidStaticReportFormatterPolicy {
  bool enable_markdown = true;
  bool enable_latex = false;
  bool enable_typst = false;

  [[nodiscard]] static auto MarkdownOnly()
      -> AndroidStaticReportFormatterPolicy {
    return {};
  }

  [[nodiscard]] static auto AllFormats() -> AndroidStaticReportFormatterPolicy {
    return {
        .enable_markdown = true, .enable_latex = true, .enable_typst = true};
  }
};

class AndroidStaticReportFormatterRegistrar final
    : public tracer_core::application::ports::IStaticReportFormatterRegistrar {
 public:
  explicit AndroidStaticReportFormatterRegistrar(
      AndroidStaticReportFormatterPolicy policy =
          AndroidStaticReportFormatterPolicy::MarkdownOnly());

  auto RegisterStaticFormatters() const -> void override;

 private:
  AndroidStaticReportFormatterPolicy policy_;
};

}  // namespace infrastructure::reports

#endif  // INFRASTRUCTURE_REPORTS_FACADE_ANDROID_STATIC_REPORT_FORMATTER_REGISTRAR_H_
