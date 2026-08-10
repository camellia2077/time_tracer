#include "infra/tests/insights_formatter/insights_formatter_parity_internal.hpp"

namespace insights_formatter_parity_internal {

auto RunMarkdownSnapshotCases(const std::filesystem::path& snapshot_root,
                              const ParityOutputs& outputs,
                              bool update_snapshots, int& failures) -> void {
  constexpr size_t kMarkdownIndex = 0;
  RunFormatSnapshotCases(
      "markdown", ".md", snapshot_root, outputs.cli_by_format[kMarkdownIndex],
      outputs.android_by_format[kMarkdownIndex], update_snapshots, failures);
}

}  // namespace insights_formatter_parity_internal

auto main() -> int {
  return insights_formatter_parity_internal::RunFormatterParityTests();
}
