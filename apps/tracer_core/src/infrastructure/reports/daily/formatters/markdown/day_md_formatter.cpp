// infrastructure/reports/daily/formatters/markdown/day_md_formatter.cpp
#include "infrastructure/reports/daily/formatters/markdown/day_md_formatter.hpp"

#include "infrastructure/reports/shared/interfaces/formatter_abi_wrapper_template.hpp"

namespace {

struct DayMdFormatterAbiTraits final
    : infrastructure::reports::abi::DailyFormatterAbiTraitsBase<
          DayMdFormatterAbiTraits, DayMdFormatter, DayMdConfig, TtDayMdConfigV1,
          formatter_c_config_bridge_v1::GetDayMdConfigView> {
  static constexpr const char* kFormatterName = "day markdown";
};

}  // namespace

// NOLINTBEGIN(readability-identifier-naming)
TT_DEFINE_FORMATTER_ABI_EXPORTS(DayMdFormatterAbiTraits)
// NOLINTEND(readability-identifier-naming)
