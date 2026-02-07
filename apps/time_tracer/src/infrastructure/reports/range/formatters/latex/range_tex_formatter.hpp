// infrastructure/reports/range/formatters/latex/range_tex_formatter.hpp
#ifndef REPORTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_FORMATTER_H_
#define REPORTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_FORMATTER_H_

#include <memory>

#include "domain/reports/models/range_report_data.hpp"
#include "infrastructure/reports/range/formatters/latex/range_tex_config.hpp"
#include "infrastructure/reports/shared/formatters/templates/base_tex_formatter.hpp"

class RangeTexFormatter
    : public BaseTexFormatter<RangeReportData, RangeTexConfig> {
 public:
  explicit RangeTexFormatter(std::shared_ptr<RangeTexConfig> config);

 private:
  [[nodiscard]] auto ValidateData(const RangeReportData& data) const
      -> std::string override;
  [[nodiscard]] auto IsEmptyData(const RangeReportData& data) const
      -> bool override;
  [[nodiscard]] auto GetAvgDays(const RangeReportData& data) const
      -> int override;
  [[nodiscard]] auto GetNoRecordsMsg() const -> std::string override;
  void FormatHeaderContent(std::stringstream& report_stream,
                           const RangeReportData& data) const override;
};

#endif  // REPORTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_FORMATTER_H_
