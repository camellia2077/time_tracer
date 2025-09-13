// queries/shared/formatters/breakdown/tex/BreakdownTex.hpp
#ifndef PROJECT_BREAKDOWN_TEX_FORMAT_HPP
#define PROJECT_BREAKDOWN_TEX_FORMAT_HPP

#include "queries/shared/interface/ITreeFmt.hpp"

/**
 * @class BreakdownTex
 * @brief 将项目明细数据格式化为 LaTeX 字符串。
 */
class BreakdownTex : public ITreeFmt {
public:
    /**
     * @brief 实现接口的 format 方法，生成完整的 LaTeX 格式化字符串。
     */
    std::string format(const ProjectTree& tree, long long total_duration, int avg_days) const override;

private:
    /**
     * @brief 递归辅助函数，用于生成层级的 LaTeX 列表。
     * @param ss 输出流。
     * @param node 当前项目节点。
     * @param avg_days 用于格式化时间的平均天数。
     */
    void generate_sorted_output(std::stringstream& ss, const ProjectNode& node, int avg_days) const;

    /**
     * @brief 转义 LaTeX 特殊字符，以防止编译错误。
     * @param input 需要转义的原始字符串。
     * @return 转义后的字符串。
     */
    std::string escape_latex(const std::string& input) const;
};

#endif // PROJECT_BREAKDOWN_TEX_FORMAT_HPP