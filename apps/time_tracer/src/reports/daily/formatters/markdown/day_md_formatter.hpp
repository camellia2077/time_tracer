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
  [[nodiscard]] auto IsEmptyData(const DailyReportData& data) const
      -> bool override;
  [[nodiscard]] auto GetAvgDays(const DailyReportData& data) const
      -> int override;
  [[nodiscard]] auto GetNoRecordsMsg() const -> std::string override;

  void FormatHeaderContent(std::stringstream& report_stream,
                           const DailyReportData& data) const override;
  void FormatExtraContent(std::stringstream& report_stream,
                          const DailyReportData& data) const override;

 private:
  // 原本的私有辅助函数现在变成了钩子函数的具体实现，或者被内联
  // 原本的私有辅助函数现在变成了钩子函数的具体实现，或者被内联
  void DisplayDetailedActivities(std::stringstream& report_stream,
                                 const DailyReportData& data) const;
};

#endif  // REPORTS_DAILY_FORMATTERS_MARKDOWN_DAY_MD_FORMATTER_H_
