// infrastructure/reports/shared/formatters/typst/typ_utils.hpp
#ifndef REPORTS_SHARED_FORMATTERS_TYPST_TYP_UTILS_H_
#define REPORTS_SHARED_FORMATTERS_TYPST_TYP_UTILS_H_

#include <string>

#include "domain/reports/models/project_tree.hpp"
#include "infrastructure/reports/shared/api/shared_api.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"

namespace TypUtils {

// Public API: keep parameter order and naming for ABI compatibility.
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
[[nodiscard]] REPORTS_SHARED_API auto BuildTextSetup(
    const std::string& base_font, int base_font_size, double line_spacing_em)
    -> std::string;
// NOLINTEND(bugprone-easily-swappable-parameters)

// Public API: keep parameter order and naming for ABI compatibility.
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
[[nodiscard]] REPORTS_SHARED_API auto BuildPageSetup(double margin_top_cm,
                                                     double margin_bottom_cm,
                                                     double margin_left_cm,
                                                     double margin_right_cm)
    -> std::string;
// NOLINTEND(bugprone-easily-swappable-parameters)

// Public API: keep parameter order and naming for ABI compatibility.
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
[[nodiscard]] REPORTS_SHARED_API auto BuildTitleText(
    const std::string& category_title_font, int category_title_font_size,
    const std::string& title_text) -> std::string;
// NOLINTEND(bugprone-easily-swappable-parameters)

/**
 * @brief Format a project tree as Typst content.
 *
 * @param tree Project tree to format.
 * @param total_duration Total duration of all projects.
 * @param avg_days Days count used for averaging.
 * @param category_title_font Category title font.
 * @param category_title_font_size Category title font size.
 * @return Formatted Typst content.
 */
// Public API: keep parameter order and naming for ABI compatibility.
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
REPORTS_SHARED_API auto FormatProjectTree(
    const reporting::ProjectTree& tree, long long total_duration, int avg_days,
    const std::string& category_title_font, int category_title_font_size)
    -> std::string;
REPORTS_SHARED_API auto FormatProjectTree(
    const TtProjectTreeNodeV1* nodes, uint32_t node_count,
    long long total_duration, int avg_days,
    const std::string& category_title_font, int category_title_font_size)
    -> std::string;
// NOLINTEND(bugprone-easily-swappable-parameters)

}  // namespace TypUtils

#endif  // REPORTS_SHARED_FORMATTERS_TYPST_TYP_UTILS_H_
