// infrastructure/tests/report_formatter/report_formatter_parity_internal.hpp
#ifndef INFRASTRUCTURE_TESTS_REPORT_FORMATTER_PARITY_INTERNAL_HPP_
#define INFRASTRUCTURE_TESTS_REPORT_FORMATTER_PARITY_INTERNAL_HPP_

#include <array>
#include <filesystem>
#include <optional>
#include <string>

namespace report_formatter_parity_internal {

struct CaseOutputs {
  std::string day;
  std::string month;
  std::string week;
  std::string year;
  std::string range;
};

struct ParityOutputs {
  std::array<CaseOutputs, 3> cli_by_format{};
  std::array<CaseOutputs, 3> android_by_format{};
};

[[nodiscard]] auto NormalizeNewlines(std::string text) -> std::string;
[[nodiscard]] auto ReadFileText(const std::filesystem::path& path)
    -> std::optional<std::string>;
auto WriteFileText(const std::filesystem::path& path,
                   const std::string& content) -> bool;

[[nodiscard]] auto BuildFirstDiffMessage(const std::string& left,
                                         const std::string& right)
    -> std::string;
auto AssertParityAndSnapshot(const std::string& case_name,
                             const std::string& snapshot_content,
                             const std::string& cli_content,
                             const std::string& android_content, int& failures)
    -> void;
auto RunCaseWithSnapshot(const std::string& case_name,
                         const std::filesystem::path& snapshot_file,
                         const std::string& cli_output,
                         const std::string& android_output,
                         bool update_snapshots, int& failures) -> void;

auto RunFormatSnapshotCases(const std::string& format_label,
                            const std::string& extension,
                            const std::filesystem::path& snapshot_root,
                            const CaseOutputs& cli_outputs,
                            const CaseOutputs& android_outputs,
                            bool update_snapshots, int& failures) -> void;

auto RunMarkdownSnapshotCases(const std::filesystem::path& snapshot_root,
                              const ParityOutputs& outputs,
                              bool update_snapshots, int& failures) -> void;
auto RunLatexSnapshotCases(const std::filesystem::path& snapshot_root,
                           const ParityOutputs& outputs, bool update_snapshots,
                           int& failures) -> void;
auto RunTypstSnapshotCases(const std::filesystem::path& snapshot_root,
                           const ParityOutputs& outputs, bool update_snapshots,
                           int& failures) -> void;

}  // namespace report_formatter_parity_internal

#endif  // INFRASTRUCTURE_TESTS_REPORT_FORMATTER_PARITY_INTERNAL_HPP_
