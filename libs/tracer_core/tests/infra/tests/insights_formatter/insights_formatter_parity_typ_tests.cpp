// infrastructure/tests/insights_formatter/insights_formatter_parity_typ_tests.cpp
#include "infra/tests/insights_formatter/insights_formatter_parity_internal.hpp"

namespace insights_formatter_parity_internal {

auto RunTypstSnapshotCases(const std::filesystem::path& snapshot_root,
                           const ParityOutputs& outputs, bool update_snapshots,
                           int& failures) -> void {
  constexpr size_t kTypstIndex = 2;
  RunFormatSnapshotCases(
      "typst", ".typ", snapshot_root, outputs.cli_by_format[kTypstIndex],
      outputs.android_by_format[kTypstIndex], update_snapshots, failures);
}

}  // namespace insights_formatter_parity_internal
