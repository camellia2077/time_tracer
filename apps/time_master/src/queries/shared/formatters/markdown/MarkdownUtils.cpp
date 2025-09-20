// queries/shared/formatters/markdown/MarkdownUtils.cpp
#include "MarkdownUtils.hpp"
#include "queries/shared/formatters/base/ProjectTreeFormatter.hpp"
#include <memory>
#include <sstream>
#include <iomanip>

namespace MarkdownUtils {

/**
 * @class MarkdownFormattingStrategy
 * @brief 实现了 IFormattingStrategy 接口，用于生成 Markdown 格式的字符串。
 */
class MarkdownFormattingStrategy : public reporting::IFormattingStrategy {
public:
    std::string format_category_header(
        const std::string& category_name,
        const std::string& formatted_duration,
        double percentage) const override
    {
        std::stringstream ss;
        ss << "\n## " << category_name << ": "
           << formatted_duration
           << " (" << std::fixed << std::setprecision(1) << percentage << "%) ##\n";
        return ss.str();
    }

    std::string format_tree_node(
        const std::string& project_name,
        const std::string& formatted_duration,
        int indent_level) const override
    {
        std::stringstream ss;
        ss << std::string(indent_level * 2, ' ') << "- " << project_name << ": " << formatted_duration << "\n";
        return ss.str();
    }
};

// --- Public API ---

std::string format_project_tree(const ProjectTree& tree, long long total_duration, int avg_days) {
    auto strategy = std::make_unique<MarkdownFormattingStrategy>();
    reporting::ProjectTreeFormatter formatter(std::move(strategy));
    return formatter.format_project_tree(tree, total_duration, avg_days);
}

} // namespace MarkdownUtils