// infrastructure/reports/range/formatters/typst/range_typ_formatter.cpp
#include "infrastructure/reports/range/formatters/typst/range_typ_formatter.hpp"

#include "infrastructure/reports/shared/interfaces/formatter_abi_wrapper_template.hpp"

namespace {

struct RangeTypFormatterAbiTraits final
    : infrastructure::reports::abi::RangeFamilyFormatterAbiTraitsBase<
          RangeTypFormatterAbiTraits, RangeTypFormatter, RangeTypConfig,
          TtRangeTypConfigV1,
          formatter_c_config_bridge_v1::GetRangeTypConfigView> {
  static constexpr const char* kFormatterName = "range typst";
};

}  // namespace

// NOLINTBEGIN(readability-identifier-naming)
TT_DEFINE_FORMATTER_ABI_EXPORTS(RangeTypFormatterAbiTraits)
// NOLINTEND(readability-identifier-naming)
