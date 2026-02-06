// reports/daily/formatters/statistics/stat_formatter.hpp
#ifndef REPORTS_DAILY_FORMATTERS_STATISTICS_STAT_FORMATTER_H_
#define REPORTS_DAILY_FORMATTERS_STATISTICS_STAT_FORMATTER_H_

#include <memory>
#include <string>

#include "reports/daily/common/day_base_config.hpp"
#include "reports/daily/formatters/statistics/i_stat_strategy.hpp"
#include "reports/data/model/daily_report_data.hpp"
#include "reports/shared/api/shared_api.hpp"

/**
 * @class StatFormatter
 * @brief (上下文) 负责生成日报的统计数据部分。
 */

DISABLE_C4251_WARNING  // <-- [新增] 2. 禁用C4251 (因为有 std::unique_ptr)

    class REPORTS_SHARED_API StatFormatter {  // <-- [修改] 3. 添加API宏
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
  [[nodiscard]] auto Format(const DailyReportData& data,
                            const std::shared_ptr<DayBaseConfig>& config) const
      -> std::string;

 private:
  std::unique_ptr<IStatStrategy> m_strategy_;
};

ENABLE_C4251_WARNING  // <-- [新增] 4. 恢复C4251

#endif  // REPORTS_DAILY_FORMATTERS_STATISTICS_STAT_FORMATTER_H_