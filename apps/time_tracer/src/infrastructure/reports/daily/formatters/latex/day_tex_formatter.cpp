// infrastructure/reports/daily/formatters/latex/day_tex_formatter.cpp
#include "infrastructure/reports/daily/formatters/latex/day_tex_formatter.hpp"

#include "infrastructure/reports/shared/interfaces/formatter_abi_wrapper_template.hpp"

namespace {

struct DayTexFormatterAbiTraits final
    : infrastructure::reports::abi::DailyFormatterAbiTraitsBase<
          DayTexFormatterAbiTraits, DayTexFormatter, DayTexConfig,
          TtDayTexConfigV1, formatter_c_config_bridge_v1::GetDayTexConfigView> {
  static constexpr const char* kFormatterName = "day latex";
};

}  // namespace

// NOLINTBEGIN(readability-identifier-naming)
TT_DEFINE_FORMATTER_ABI_EXPORTS_(DayTexFormatterAbiTraits)
// NOLINTEND(readability-identifier-naming)
