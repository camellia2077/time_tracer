// infrastructure/reports/daily/formatters/typst/day_typ_formatter.cpp
#include "infrastructure/reports/daily/formatters/typst/day_typ_formatter.hpp"

#include "infrastructure/reports/shared/interfaces/formatter_abi_wrapper_template.hpp"

namespace {

struct DayTypFormatterAbiTraits final
    : infrastructure::reports::abi::DailyFormatterAbiTraitsBase<
          DayTypFormatterAbiTraits, DayTypFormatter, DayTypConfig,
          TtDayTypConfigV1, formatter_c_config_bridge_v1::GetDayTypConfigView> {
  static constexpr const char* kFormatterName = "day typst";
};

}  // namespace

// NOLINTBEGIN(readability-identifier-naming)
TT_DEFINE_FORMATTER_ABI_EXPORTS(DayTypFormatterAbiTraits)
// NOLINTEND(readability-identifier-naming)
