// domain/logic/converter/convert/core/log_linker.hpp
#ifndef CONVERTER_CONVERT_CORE_LOG_LINKER_H_
#define CONVERTER_CONVERT_CORE_LOG_LINKER_H_

#include <map>
#include <string>
#include <vector>

#include "domain/model/daily_log.hpp"
#include "infrastructure/config/models/converter_config_models.hpp"

/**
 * @class LogLinker
 * @brief 负责处理跨批次（通常是跨月）的日志衔接逻辑。
 * * 当日志被并行处理或分批处理时，批次之间的“睡眠”活动往往会丢失（因为缺少前一天的上下文）。
 * 此类用于在所有数据加载到内存后，重新扫描并修复批次边界处的连续性问题。
 */
class LogLinker {
 public:
  explicit LogLinker(const ConverterConfig& config);

  /**
   * @brief 链接所有按月分组的日志数据。
   * 遍历 Map（按时间顺序），将上个月的最后一天与下个月的第一天进行逻辑衔接。
   * @param data_map 按 Key (YYYY-MM) 排序的日志数据集合
   */
  void LinkLogs(std::map<std::string, std::vector<DailyLog>>& data_map);

 private:
  const ConverterConfig& config_;

  /**
   * @brief 处理具体的跨天衔接逻辑
   * @param current_day 需要被修复的当前日期（通常是月初第一天）
   * @param prev_day 前置日期（通常是上月末最后一天）
   */
  void ProcessCrossDay(DailyLog& current_day, const DailyLog& prev_day);

  /**
   * @brief 调用 DayStats 重新计算单日统计数据
   * 当插入了新的睡眠活动后，该天的统计数据需要刷新。
   */
  static void RecalculateStats(DailyLog& day);

  // 辅助工具：格式化时间字符串 (HHMM -> HH:MM)
  static auto FormatTime(const std::string& timeStr) -> std::string;
};

#endif  // CONVERTER_CONVERT_CORE_LOG_LINKER_H_
