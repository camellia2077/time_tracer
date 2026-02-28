// infrastructure/reports/range/formatters/latex/range_tex_formatter.cpp
#include "infrastructure/reports/range/formatters/latex/range_tex_formatter.hpp"

#include "infrastructure/reports/shared/interfaces/formatter_abi_wrapper_template.hpp"

namespace {

struct RangeTexFormatterAbiTraits final
    : infrastructure::reports::abi::RangeFamilyFormatterAbiTraitsBase<
          RangeTexFormatterAbiTraits, RangeTexFormatter, RangeTexConfig,
          TtRangeTexConfigV1,
          formatter_c_config_bridge_v1::GetRangeTexConfigView> {
  static constexpr const char* kFormatterName = "range latex";
};

}  // namespace

// NOLINTBEGIN(readability-identifier-naming)
TT_DEFINE_FORMATTER_ABI_EXPORTS(RangeTexFormatterAbiTraits)
// NOLINTEND(readability-identifier-naming)
