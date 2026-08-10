// infra/insights/facade/android_static_insights_formatter_registrar.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_FACADE_ANDROID_STATIC_INSIGHTS_FORMATTER_REGISTRAR_H_
#define INFRASTRUCTURE_INSIGHTS_FACADE_ANDROID_STATIC_INSIGHTS_FORMATTER_REGISTRAR_H_

#include <cstdint>

#include "application/ports/insights/i_insights_formatter_registry.hpp"

namespace infrastructure::insights {

struct AndroidStaticInsightsFormatterPolicy {
  bool enable_markdown = true;
  bool enable_latex = false;
  bool enable_typst = false;

  [[nodiscard]] static auto MarkdownOnly()
      -> AndroidStaticInsightsFormatterPolicy {
    return {};
  }

  [[nodiscard]] static auto AllFormats() -> AndroidStaticInsightsFormatterPolicy {
    return {
        .enable_markdown = true, .enable_latex = true, .enable_typst = true};
  }
};

class AndroidStaticInsightsFormatterRegistrar final
    : public tracer_core::application::ports::IStaticInsightsFormatterRegistrar {
 public:
  explicit AndroidStaticInsightsFormatterRegistrar(
      AndroidStaticInsightsFormatterPolicy policy =
          AndroidStaticInsightsFormatterPolicy::MarkdownOnly());

  auto RegisterStaticFormatters() const -> void override;

 private:
  AndroidStaticInsightsFormatterPolicy policy_;
};

}  // namespace infrastructure::insights

#endif  // INFRASTRUCTURE_INSIGHTS_FACADE_ANDROID_STATIC_INSIGHTS_FORMATTER_REGISTRAR_H_
