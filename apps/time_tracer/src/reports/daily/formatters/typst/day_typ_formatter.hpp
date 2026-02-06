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
  // --- 实现基类钩子 ---
  [[nodiscard]] auto IsEmptyData(const DailyReportData& data) const
      -> bool override;
  [[nodiscard]] auto GetAvgDays(const DailyReportData& data) const
      -> int override;

  void FormatHeaderContent(std::stringstream& report_stream,
                           const DailyReportData& data) const override;
  void FormatExtraContent(std::stringstream& report_stream,
                          const DailyReportData& data) const override;

  // 适配接口
  [[nodiscard]] auto GetNoRecordsMsg() const -> std::string override;
};

#endif  // REPORTS_DAILY_FORMATTERS_TYPST_DAY_TYP_FORMATTER_H_