// infra/insights/facade/android_static_insights_formatter_registrar_internal.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_FACADE_ANDROID_STATIC_INSIGHTS_FORMATTER_REGISTRAR_INTERNAL_HPP_
#define INFRASTRUCTURE_INSIGHTS_FACADE_ANDROID_STATIC_INSIGHTS_FORMATTER_REGISTRAR_INTERNAL_HPP_

#include "infra/insights/facade/android_static_insights_formatter_registrar.hpp"

namespace infrastructure::insights::detail {

auto RegisterMarkdownFormatters(
    const AndroidStaticInsightsFormatterPolicy& policy) -> void;
auto RegisterLatexFormatters(const AndroidStaticInsightsFormatterPolicy& policy)
    -> void;
auto RegisterTypstFormatters(const AndroidStaticInsightsFormatterPolicy& policy)
    -> void;

}  // namespace infrastructure::insights::detail

#endif  // INFRASTRUCTURE_INSIGHTS_FACADE_ANDROID_STATIC_INSIGHTS_FORMATTER_REGISTRAR_INTERNAL_HPP_
