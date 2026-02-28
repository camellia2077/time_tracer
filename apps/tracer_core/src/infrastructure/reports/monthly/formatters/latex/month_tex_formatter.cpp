// infrastructure/reports/monthly/formatters/latex/month_tex_formatter.cpp
#include "infrastructure/reports/monthly/formatters/latex/month_tex_formatter.hpp"

#include "infrastructure/reports/shared/interfaces/formatter_abi_wrapper_template.hpp"

namespace {

struct MonthTexFormatterAbiTraits final
    : infrastructure::reports::abi::MonthlyFormatterAbiTraitsBase<
          MonthTexFormatterAbiTraits, MonthTexFormatter, MonthTexConfig,
          TtMonthTexConfigV1,
          formatter_c_config_bridge_v1::GetMonthTexConfigView> {
  static constexpr const char* kFormatterName = "month latex";
};

}  // namespace

// NOLINTBEGIN(readability-identifier-naming)
TT_DEFINE_FORMATTER_ABI_EXPORTS(MonthTexFormatterAbiTraits)
// NOLINTEND(readability-identifier-naming)
