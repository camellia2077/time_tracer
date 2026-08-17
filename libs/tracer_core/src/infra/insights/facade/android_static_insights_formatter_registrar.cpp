// infra/insights/facade/android_static_insights_formatter_registrar.cpp
#include "infra/insights/facade/android_static_insights_formatter_registrar.hpp"
#include "infra/insights/facade/android_static_insights_formatter_registrar_internal.hpp"

namespace infrastructure::insights {

AndroidStaticInsightsFormatterRegistrar::
    AndroidStaticInsightsFormatterRegistrar(
        AndroidStaticInsightsFormatterPolicy policy)
    : policy_(policy) {}

auto AndroidStaticInsightsFormatterRegistrar::RegisterStaticFormatters() const
    -> void {
  detail::RegisterMarkdownFormatters(policy_);
  detail::RegisterLatexFormatters(policy_);
  detail::RegisterTypstFormatters(policy_);
}

}  // namespace infrastructure::insights
