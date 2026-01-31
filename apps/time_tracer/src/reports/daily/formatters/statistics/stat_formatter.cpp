// reports/daily/formatters/statistics/stat_formatter.cpp
#include "stat_formatter.hpp"

#include <vector>

#include "reports/shared/utils/format/time_format.hpp"

StatFormatter::StatFormatter(std::unique_ptr<IStatStrategy> strategy)
    : m_strategy(std::move(strategy)) {
  if (!m_strategy) {
    throw std::invalid_argument(
        "Formatting strategy for StatFormatter cannot be null.");
  }
}

// [新增] 递归处理统计项的辅助函数
void ProcessStatItems(const std::vector<StatisticItemConfig>& items,
                      const DailyReportData& data,
                      const IStatStrategy* strategy,
                      std::vector<std::string>& lines, int level) {
  for (const auto& item : items) {
    if (!item.show) {
      continue;
    }

    // 1. 从 data.stats 中获取数据
    long long duration = 0;
    if (!item.db_column.empty() &&
        (data.stats.contains(item.db_column) != 0u)) {
      duration = data.stats.at(item.db_column);
    }

    // 2. 格式化当前行
    // 注意：目前 IStatStrategy 只定义了 main_item 和 sub_item。
    // 这里做一个简单的映射：Level 0 为 Main，Level > 0 为 Sub。
    if (level == 0) {
      lines.push_back(strategy->format_main_item(
          item.label, time_format_duration(duration)));
    } else {
      lines.push_back(strategy->format_sub_item(
          item.label, time_format_duration(duration)));
    }

    // 3. 递归处理子项
    if (!item.sub_items.empty()) {
      ProcessStatItems(item.sub_items, data, strategy, lines, level + 1);
    }
  }
}

auto StatFormatter::format(const DailyReportData& data,
                           const std::shared_ptr<DayBaseConfig>& config) const
    -> std::string {
  const auto& items_config =
      config->get_statistics_items();  // 现在这是一个 vector
  std::vector<std::string> lines_to_print;

  // [核心修改] 使用递归函数替代硬编码的遍历逻辑
  if (!items_config.empty()) {
    ProcessStatItems(items_config, data, m_strategy.get(), lines_to_print, 0);
  }

  if (lines_to_print.empty()) {
    return "";
  }

  std::string header =
      m_strategy->format_header(config->get_statistics_label());
  std::string body = m_strategy->build_output(lines_to_print);

  return header + body;
}