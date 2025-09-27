// reports/shared/formatters/base/ProjectTreeFormatter.hpp
#ifndef PROJECT_TREE_FORMATTER_HPP
#define PROJECT_TREE_FORMATTER_HPP

#include <string>
#include <sstream>
#include "common/utils/ProjectTree.hpp"

namespace reporting {

/**
 * @class IFormattingStrategy
 * @brief 定义了将项目树节点格式化为特定字符串的接口（策略接口）。
 *
 * 这个接口允许 ProjectTreeFormatter 在不知道具体格式（如 Markdown, Typst）
 * 的情况下，将格式化任务委托给实现了此接口的具体策略类。
 */
class IFormattingStrategy {
public:
    virtual ~IFormattingStrategy() = default;

    /**
     * @brief 格式化一个顶层分类的标题。
     * @param category_name 类别名称。
     * @param formatted_duration 格式化后的时长字符串。
     * @param percentage 该类别占总时长的百分比。
     * @return 格式化后的标题字符串。
     */
    virtual std::string format_category_header(
        const std::string& category_name,
        const std::string& formatted_duration,
        double percentage) const = 0;

    /**
     * @brief 格式化项目树中的一个普通节点（列表项）。
     * @param project_name 项目名称。
     * @param formatted_duration 格式化后的时长字符串。
     * @param indent_level 当前节点的缩进层级。
     * @return 格式化后的列表项字符串。
     */
    virtual std::string format_tree_node(
        const std::string& project_name,
        const std::string& formatted_duration,
        int indent_level) const = 0;

    /**
     * @brief [新增] 在开始处理一个节点的子节点列表前调用。
     * @return 在子节点列表前需要添加的字符串（例如 LaTeX 的 \\begin{itemize}）。
     */
    virtual std::string start_children_list() const { return ""; }

    /**
     * @brief [新增] 在处理完一个节点的子节点列表后调用。
     * @return 在子节点列表后需要添加的字符串（例如 LaTeX 的 \\end{itemize}）。
     */
    virtual std::string end_children_list() const { return ""; }
};


/**
 * @class ProjectTreeFormatter
 * @brief 负责遍历项目树并使用指定的格式化策略来生成报告字符串。
 *
 * 这是一个上下文类，它持有一个 IFormattingStrategy 策略的引用，
 * 并用它来执行具体的格式化操作。
 */
class ProjectTreeFormatter {
public:
    /**
     * @brief 构造一个新的 ProjectTreeFormatter。
     * @param strategy 一个实现了 IFormattingStrategy 接口的策略对象的 unique_ptr。
     */
    explicit ProjectTreeFormatter(std::unique_ptr<IFormattingStrategy> strategy);

    /**
     * @brief 生成格式化后的项目树报告。
     * @param tree 要格式化的项目树。
     * @param total_duration 所有项目的总时长，用于计算百分比。
     * @param avg_days 用于计算平均时长的天数。
     * @return 格式化后的完整报告字符串。
     */
    std::string format_project_tree(
        const ProjectTree& tree,
        long long total_duration,
        int avg_days) const;

private:
    std::unique_ptr<IFormattingStrategy> m_strategy;

    /**
     * @brief 递归函数，用于生成排序后的项目列表。
     * @param ss 字符串流，用于构建输出。
     * @param node 当前正在处理的 ProjectNode。
     * @param indent 当前的缩进级别。
     * @param avg_days 用于计算平均值的天数。
     */
    void generate_sorted_output(
        std::stringstream& ss,
        const ProjectNode& node,
        int indent,
        int avg_days) const;
};

} // namespace reporting

#endif // PROJECT_TREE_FORMATTER_HPP