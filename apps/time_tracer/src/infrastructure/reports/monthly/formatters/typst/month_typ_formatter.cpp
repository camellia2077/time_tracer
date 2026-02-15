// infrastructure/reports/monthly/formatters/typst/month_typ_formatter.cpp
#include "infrastructure/reports/monthly/formatters/typst/month_typ_formatter.hpp"

#include "infrastructure/reports/shared/interfaces/formatter_abi_wrapper_template.hpp"

namespace {

struct MonthTypFormatterAbiTraits final
    : infrastructure::reports::abi::MonthlyFormatterAbiTraitsBase<
          MonthTypFormatterAbiTraits, MonthTypFormatter, MonthTypConfig,
          TtMonthTypConfigV1,
          formatter_c_config_bridge_v1::GetMonthTypConfigView> {
  static constexpr const char* kFormatterName = "month typst";
};

}  // namespace

// NOLINTBEGIN(readability-identifier-naming)
TT_DEFINE_FORMATTER_ABI_EXPORTS(MonthTypFormatterAbiTraits)
// NOLINTEND(readability-identifier-naming)
