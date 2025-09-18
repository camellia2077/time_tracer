// queries/shared/utils/TexUtils.hpp
#ifndef TEX_UTILS_HPP
#define TEX_UTILS_HPP

#include <string>
#include <vector>
#include <map>

namespace TexUtils {

/**
 * @brief 生成 LaTeX 文档的 Preamble（前导部分）。
 * @param main_font 主字体名称。
 * @param cjk_main_font 中日韩字体名称。
 * @param keyword_colors 一个 map，包含用于语法高亮的关键字和对应的十六进制颜色值。
 * @return 包含完整 Preamble 的字符串。
 */
std::string get_tex_preamble(
    const std::string& main_font,
    const std::string& cjk_main_font,
    const std::map<std::string, std::string>& keyword_colors = {}
);

/**
 * @brief 生成 LaTeX 文档的 Postfix（结尾部分）。
 * @return 包含 "\\end{document}" 的字符串。
 */
std::string get_tex_postfix();

} // namespace TexUtils

#endif // TEX_UTILS_HPP