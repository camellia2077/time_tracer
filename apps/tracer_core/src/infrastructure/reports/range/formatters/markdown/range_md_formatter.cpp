// infrastructure/reports/range/formatters/markdown/range_md_formatter.cpp
#include "infrastructure/reports/range/formatters/markdown/range_md_formatter.hpp"

#include "infrastructure/reports/shared/interfaces/formatter_abi_wrapper_template.hpp"

namespace {

struct RangeMdFormatterAbiTraits final
    : infrastructure::reports::abi::RangeFamilyFormatterAbiTraitsBase<
          RangeMdFormatterAbiTraits, RangeMdFormatter, RangeMdConfig,
          TtRangeMdConfigV1,
          formatter_c_config_bridge_v1::GetRangeMdConfigView> {
  static constexpr const char* kFormatterName = "range markdown";
};

}  // namespace

// NOLINTBEGIN(readability-identifier-naming)
TT_DEFINE_FORMATTER_ABI_EXPORTS(RangeMdFormatterAbiTraits)
// NOLINTEND(readability-identifier-naming)
