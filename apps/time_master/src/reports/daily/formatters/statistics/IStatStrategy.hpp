// reports/daily/formatters/statistics/IStatStrategy.hpp
#ifndef I_STAT_STRATEGY_HPP
#define I_STAT_STRATEGY_HPP

#include <string>
#include <vector>

/**
 * @class IStatStrategy
 * @brief (策略接口) 为格式化统计数据定义了通用接口。
 *
 * 具体的格式化器 (Markdown, LaTeX, Typst) 将实现此接口，
 * 以提供特定于格式的字符串表示。
 */
class IStatStrategy {
public:
    virtual ~IStatStrategy() = default;

    /**
     * @brief 格式化统计部分的标题。
     * @param title 标题文本。
     * @return 格式化后的标题字符串。
     */
    virtual std::string format_header(const std::string& title) const = 0;

    /**
     * @brief 格式化一个顶层统计项。
     * @param label 统计项的标签 (例如, "Sleep Time")。
     * @param value 格式化后的时长值 (例如, "8h 15m")。
     * @return 格式化后的列表项字符串。
     */
    virtual std::string format_main_item(const std::string& label, const std::string& value) const = 0;

    /**
     * @brief 格式化一个嵌套的子项。
     * @param label 子项的标签 (例如, "Anaerobic Exercise")。
     * @param value 格式化后的时长值。
     * @return 格式化后的嵌套列表项字符串。
     */
    virtual std::string format_sub_item(const std::string& label, const std::string& value) const = 0;

    /**
     * @brief 将所有格式化后的行（主项和子项）组合成最终的字符串。
     * @param lines 包含所有已格式化行的向量。
     * @return 完整的、格式化后的统计部分字符串。
     */
    virtual std::string build_output(const std::vector<std::string>& lines) const = 0;
};

#endif // I_STAT_STRATEGY_HPP