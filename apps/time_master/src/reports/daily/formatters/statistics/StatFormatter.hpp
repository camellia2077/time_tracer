// reports/daily/formatters/statistics/StatFormatter.hpp
#ifndef STAT_FORMATTER_HPP
#define STAT_FORMATTER_HPP

#include <string>
#include <memory>
#include "reports/shared/data/DailyReportData.hpp"
#include "reports/daily/formatters/base/DayBaseConfig.hpp"
#include "IStatStrategy.hpp"

/**
 * @class StatFormatter
 * @brief (上下文) 负责生成日报的统计数据部分。
 *
 * 使用策略模式将通用的数据处理逻辑与特定于格式的字符串生成分离开来。
 */
class StatFormatter {
public:
    /**
     * @brief 构造函数。
     * @param strategy 一个实现了 IStatStrategy 接口的策略对象的 unique_ptr。
     */
    explicit StatFormatter(std::unique_ptr<IStatStrategy> strategy);

    /**
     * @brief 生成格式化后的统计数据报告。
     * @param data 包含日报数据的对象。
     * @param config 指向日报基础配置的共享指针。
     * @return 格式化后的完整统计部分字符串。
     */
    std::string format(const DailyReportData& data, const std::shared_ptr<DayBaseConfig>& config) const;

private:
    std::unique_ptr<IStatStrategy> m_strategy;
};

#endif // STAT_FORMATTER_HPP