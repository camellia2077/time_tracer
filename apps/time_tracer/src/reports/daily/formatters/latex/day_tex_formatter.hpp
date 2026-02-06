// reports/daily/formatters/latex/day_tex_formatter.hpp
#ifndef REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_FORMATTER_H_
#define REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_FORMATTER_H_

#include "reports/daily/formatters/latex/day_tex_config.hpp"
#include "reports/data/model/daily_report_data.hpp"
#include "reports/shared/formatters/templates/base_tex_formatter.hpp"

class DayTexFormatter : public BaseTexFormatter<DailyReportData, DayTexConfig> {
 public:
  explicit DayTexFormatter(std::shared_ptr<DayTexConfig> config);

 protected:
  // --- 实现基类钩子 ---
  [[nodiscard]] auto IsEmptyData(const DailyReportData& data) const
      -> bool override;
  [[nodiscard]] auto GetAvgDays(const DailyReportData& data) const
      -> int override;

  void FormatHeaderContent(std::stringstream& report_stream,
                           const DailyReportData& data) const override;
  void FormatExtraContent(std::stringstream& report_stream,
                          const DailyReportData& data) const override;

  [[nodiscard]] auto GetKeywordColors() const
      -> std::map<std::string, std::string> override;

  // [新增] 实现基类定义的纯虚函数
  [[nodiscard]] auto GetNoRecordsMsg() const -> std::string override;
};

#endif  // REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_FORMATTER_H_
