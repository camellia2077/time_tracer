// reports/daily/formatters/markdown/day_md_formatter.hpp
#ifndef REPORTS_DAILY_FORMATTERS_MARKDOWN_DAY_MD_FORMATTER_H_
#define REPORTS_DAILY_FORMATTERS_MARKDOWN_DAY_MD_FORMATTER_H_

#include "reports/daily/formatters/markdown/day_md_config.hpp"
#include "reports/data/model/daily_report_data.hpp"
#include "reports/shared/formatters/templates/base_md_formatter.hpp"

class DayMdFormatter : public BaseMdFormatter<DailyReportData, DayMdConfig> {
 public:
  explicit DayMdFormatter(std::shared_ptr<DayMdConfig> config);

 protected:
  // --- 实现基类钩子 ---
  auto is_empty_data(const DailyReportData& data) const -> bool override;
  auto get_avg_days(const DailyReportData& data) const -> int override;

  void format_header_content(std::stringstream& report_stream,
                             const DailyReportData& data) const override;
  void format_extra_content(std::stringstream& report_stream,
                            const DailyReportData& data) const override;

  // 适配接口：DayConfig 使用 get_no_records() 而非 get_no_records_message()
  auto get_no_records_msg() const -> std::string override;

 private:
  // 原本的私有辅助函数现在变成了钩子函数的具体实现，或者被内联
  void _display_detailed_activities(std::stringstream& report_stream,
                                    const DailyReportData& data) const;
};

#endif  // REPORTS_DAILY_FORMATTERS_MARKDOWN_DAY_MD_FORMATTER_H_
