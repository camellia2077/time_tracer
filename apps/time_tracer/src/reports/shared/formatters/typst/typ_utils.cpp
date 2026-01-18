// reports/shared/formatters/typst/typ_utils.cpp
#include "typ_utils.hpp"
#include "reports/shared/formatters/base/project_tree_formatter.hpp"
#include <memory>
#include <format>
#include <sstream>

namespace TypUtils {

class TypstFormattingStrategy : public reporting::IFormattingStrategy {
public:
    TypstFormattingStrategy(std::string font, int font_size)
        : m_font(std::move(font)), m_font_size(font_size) {}

    std::string format_category_header(
        const std::string& category_name,
        const std::string& formatted_duration,
        double percentage) const override
    {
        // [核心修改] 将 = (Level 1) 改为 == (Level 2)
        // 这样它们就会成为 "Project Breakdown" 的子项
        return std::format(R"(#text(font: "{}", size: {}pt)[== {}])",
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

// [修正] 添加 reporting:: 命名空间前缀
std::string format_project_tree(
    const reporting::ProjectTree& tree,
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