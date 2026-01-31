// reports/daily/formatters/typst/day_typ_formatter.hpp
#ifndef REPORTS_DAILY_FORMATTERS_TYPST_DAY_TYP_FORMATTER_H_
#define REPORTS_DAILY_FORMATTERS_TYPST_DAY_TYP_FORMATTER_H_

#include "reports/daily/formatters/typst/day_typ_config.hpp"
#include "reports/data/model/daily_report_data.hpp"
#include "reports/shared/formatters/templates/base_typ_formatter.hpp"

class DayTypFormatter : public BaseTypFormatter<DailyReportData, DayTypConfig> {
 public:
  explicit DayTypFormatter(std::shared_ptr<DayTypConfig> config);

 protected:
  // --- 实现基类钩子 ---
  bool is_empty_data(const DailyReportData& data) const override;
  int get_avg_days(const DailyReportData& data) const override;

  void format_header_content(std::stringstream& ss,
                             const DailyReportData& data) const override;
  void format_extra_content(std::stringstream& ss,
                            const DailyReportData& data) const override;

  // 适配接口
  std::string get_no_records_msg() const override;
};

#endif  // REPORTS_DAILY_FORMATTERS_TYPST_DAY_TYP_FORMATTER_H_