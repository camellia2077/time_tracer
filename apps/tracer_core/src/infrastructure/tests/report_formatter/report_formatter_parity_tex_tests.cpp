// infrastructure/tests/report_formatter/report_formatter_parity_tex_tests.cpp
#include "infrastructure/tests/report_formatter/report_formatter_parity_internal.hpp"

namespace report_formatter_parity_internal {

auto RunLatexSnapshotCases(const std::filesystem::path& snapshot_root,
                           const ParityOutputs& outputs, bool update_snapshots,
                           int& failures) -> void {
  constexpr size_t kLatexIndex = 1;
  RunFormatSnapshotCases(
      "latex", ".tex", snapshot_root, outputs.cli_by_format[kLatexIndex],
      outputs.android_by_format[kLatexIndex], update_snapshots, failures);
}

}  // namespace report_formatter_parity_internal
