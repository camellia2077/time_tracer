// reports/shared/formatters/typst/TypUtils.cpp
#include "TypUtils.hpp"
#include "reports/shared/formatters/base/ProjectTreeFormatter.hpp"
#include <memory>
#include <format>
#include <sstream>

namespace TypUtils {

/**
 * @class TypstFormattingStrategy
 * @brief 实现了 IFormattingStrategy 接口，用于生成 Typst 格式的字符串。
 */
class TypstFormattingStrategy : public reporting::IFormattingStrategy {
public:
    TypstFormattingStrategy(std::string font, int font_size)
        : m_font(std::move(font)), m_font_size(font_size) {}

    std::string format_category_header(
        const std::string& category_name,
        const std::string& formatted_duration,
        double percentage) const override
    {
        return std::format(R"(#text(font: "{}", size: {}pt)[= {}])",
            m_font,
            m_font_size,
            std::format("{}: {} ({:.1f}%)",
                category_name,
                formatted_duration,
                percentage
            )
        ) + "\n";
    }

    std::string format_tree_node(
        const std::string& project_name,
        const std::string& formatted_duration,
        int indent_level) const override
    {
        return std::string(indent_level * 2, ' ') + "+ " + project_name + ": " + formatted_duration + "\n";
    }

private:
    std::string m_font;
    int m_font_size;
};

// --- Public API ---

std::string format_project_tree(
    const ProjectTree& tree,
    long long total_duration,
    int avg_days,
    const std::string& category_title_font,
    int category_title_font_size)
{
    auto strategy = std::make_unique<TypstFormattingStrategy>(category_title_font, category_title_font_size);
    reporting::ProjectTreeFormatter formatter(std::move(strategy));
    return formatter.format_project_tree(tree, total_duration, avg_days);
}

} // namespace TypUtils