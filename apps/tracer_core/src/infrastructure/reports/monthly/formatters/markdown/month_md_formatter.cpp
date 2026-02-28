// infrastructure/reports/monthly/formatters/markdown/month_md_formatter.cpp
#include "infrastructure/reports/monthly/formatters/markdown/month_md_formatter.hpp"

#include "infrastructure/reports/shared/interfaces/formatter_abi_wrapper_template.hpp"

namespace {

struct MonthMdFormatterAbiTraits final
    : infrastructure::reports::abi::MonthlyFormatterAbiTraitsBase<
          MonthMdFormatterAbiTraits, MonthMdFormatter, MonthMdConfig,
          TtMonthMdConfigV1,
          formatter_c_config_bridge_v1::GetMonthMdConfigView> {
  static constexpr const char* kFormatterName = "month markdown";
};

}  // namespace

// NOLINTBEGIN(readability-identifier-naming)
TT_DEFINE_FORMATTER_ABI_EXPORTS(MonthMdFormatterAbiTraits)
// NOLINTEND(readability-identifier-naming)
