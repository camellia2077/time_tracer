// infrastructure/reports/daily/formatters/latex/day_tex_formatter.hpp
#ifndef REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_FORMATTER_H_
#define REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_FORMATTER_H_

#include "domain/reports/models/daily_report_data.hpp"
#include "infrastructure/reports/daily/formatters/latex/day_tex_config.hpp"
#include "infrastructure/reports/shared/formatters/templates/base_tex_formatter.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"

class DayTexFormatter : public BaseTexFormatter<DailyReportData, DayTexConfig> {
 public:
  explicit DayTexFormatter(std::shared_ptr<DayTexConfig> config);
  [[nodiscard]] auto FormatReportFromView(
      const TtDailyReportDataV1& data_view) const -> std::string;

 protected:
  // --- 实现基类钩子 ---
  [[nodiscard]] auto IsEmptyData(const DailyReportData& data) const
      -> bool override;
  [[nodiscard]] auto GetAvgDays(const DailyReportData& data) const
      -> int override;

  void FormatHeaderContent(std::string& report_stream,
                           const DailyReportData& data) const override;
  void FormatExtraContent(std::string& report_stream,
                          const DailyReportData& data) const override;

  [[nodiscard]] auto GetKeywordColors() const
      -> std::map<std::string, std::string> override;

  // [新增] 实现基类定义的纯虚函数
  [[nodiscard]] auto GetNoRecordsMsg() const -> std::string override;
};

#endif  // REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_FORMATTER_H_
