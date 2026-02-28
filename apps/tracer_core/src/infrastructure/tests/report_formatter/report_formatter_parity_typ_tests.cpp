// infrastructure/tests/report_formatter/report_formatter_parity_typ_tests.cpp
#include "infrastructure/tests/report_formatter/report_formatter_parity_internal.hpp"

namespace report_formatter_parity_internal {

auto RunTypstSnapshotCases(const std::filesystem::path& snapshot_root,
                           const ParityOutputs& outputs, bool update_snapshots,
                           int& failures) -> void {
  constexpr size_t kTypstIndex = 2;
  RunFormatSnapshotCases(
      "typst", ".typ", snapshot_root, outputs.cli_by_format[kTypstIndex],
      outputs.android_by_format[kTypstIndex], update_snapshots, failures);
}

}  // namespace report_formatter_parity_internal
