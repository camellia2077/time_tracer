// infrastructure/reports/daily/formatters/statistics/i_stat_strategy.hpp
#ifndef INFRASTRUCTURE_REPORTS_DAILY_FORMATTERS_STATISTICS_I_STAT_STRATEGY_H_
#define INFRASTRUCTURE_REPORTS_DAILY_FORMATTERS_STATISTICS_I_STAT_STRATEGY_H_

#include <string>
#include <vector>

#include "infrastructure/reports/shared/api/shared_api.hpp"

/**
 * @class IStatStrategy
 * @brief (策略接口) 为格式化统计数据定义了通用接口。
 */
class REPORTS_SHARED_API IStatStrategy {
 public:
  virtual ~IStatStrategy() = default;

  /**
   * @brief 格式化统计部分的标题。
   * @param title 标题文本。
   * @return 格式化后的标题字符串。
   */
  [[nodiscard]] virtual auto FormatHeader(const std::string& title) const
      -> std::string = 0;

  /**
   * @brief 格式化一个顶层统计项。
   * @param label 统计项的标签 (例如, "Sleep Time")。
   * @param value 格式化后的时长值 (例如, "8h 15m")。
   * @return 格式化后的列表项字符串。
   */
  [[nodiscard]] virtual auto FormatMainItem(const std::string& label,
                                            const std::string& value) const
      -> std::string = 0;

  /**
   * @brief 格式化一个嵌套的子项。
   * @param label 子项的标签 (例如, "Anaerobic Exercise")。
   * @param value 格式化后的时长值。
   * @return 格式化后的嵌套列表项字符串。
   */
  [[nodiscard]] virtual auto FormatSubItem(const std::string& label,
                                           const std::string& value) const
      -> std::string = 0;

  /**
   * @brief 将所有格式化后的行（主项和子项）组合成最终的字符串。
   * @param lines 包含所有已格式化行的向量。
   * @return 完整的、格式化后的统计部分字符串。
   */
  [[nodiscard]] virtual auto BuildOutput(
      const std::vector<std::string>& lines) const -> std::string = 0;
};

#endif  // INFRASTRUCTURE_REPORTS_DAILY_FORMATTERS_STATISTICS_I_STAT_STRATEGY_H_
