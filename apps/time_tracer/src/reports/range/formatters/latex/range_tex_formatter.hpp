// reports/range/formatters/latex/range_tex_formatter.hpp
#ifndef REPORTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_FORMATTER_H_
#define REPORTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_FORMATTER_H_

#include <memory>

#include "reports/data/model/range_report_data.hpp"
#include "reports/range/formatters/latex/range_tex_config.hpp"
#include "reports/shared/formatters/templates/base_tex_formatter.hpp"

class RangeTexFormatter
    : public BaseTexFormatter<RangeReportData, RangeTexConfig> {
 public:
  explicit RangeTexFormatter(std::shared_ptr<RangeTexConfig> config);

 private:
  auto validate_data(const RangeReportData& data) const -> std::string override;
  auto is_empty_data(const RangeReportData& data) const -> bool override;
  auto get_avg_days(const RangeReportData& data) const -> int override;
  auto get_no_records_msg() const -> std::string override;
  void format_header_content(std::stringstream& report_stream,
                             const RangeReportData& data) const override;
};

#endif  // REPORTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_FORMATTER_H_
