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
  auto is_empty_data(const DailyReportData& data) const -> bool override;
  auto get_avg_days(const DailyReportData& data) const -> int override;

  void format_header_content(std::stringstream& report_stream,
                             const DailyReportData& data) const override;
  void format_extra_content(std::stringstream& report_stream,
                            const DailyReportData& data) const override;

  auto get_keyword_colors() const
      -> std::map<std::string, std::string> override;

  // [新增] 实现基类定义的纯虚函数
  auto get_no_records_msg() const -> std::string override;
};

#endif  // REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_FORMATTER_H_
