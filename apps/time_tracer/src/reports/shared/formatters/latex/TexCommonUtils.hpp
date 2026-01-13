// reports/shared/formatters/latex/TexCommonUtils.hpp
#ifndef TEX_COMMON_UTILS_HPP
#define TEX_COMMON_UTILS_HPP

#include <string>
#include <vector>
#include <sstream>
#include "reports/shared/api/shared_api.hpp"

namespace TexCommonUtils {

/**
 * @brief 用于定义摘要列表中的一项
 */
struct SummaryItem {
    std::string label;
    std::string value; // 已经转义或格式化好的 LaTeX 字符串
};

/**
 * @brief 渲染带有特定字体大小设置的 LaTeX 标题
 * 生成: {\fontsize{size}{size*1.2}\selectfont \section*{content}}
 * @param is_subsection 如果为 true，则使用 \subsection*，否则使用 \section*
 */
REPORTS_SHARED_API void render_title(
    std::stringstream& ss, 
    const std::string& content, 
    int font_size, 
    bool is_subsection = false
);

/**
 * @brief 渲染紧凑的摘要信息列表 (itemize 环境)
 */
REPORTS_SHARED_API void render_summary_list(
    std::stringstream& ss,
    const std::vector<SummaryItem>& items,
    double top_sep_pt,
    double item_sep_ex
);

} // namespace TexCommonUtils

#endif // TEX_COMMON_UTILS_HPP