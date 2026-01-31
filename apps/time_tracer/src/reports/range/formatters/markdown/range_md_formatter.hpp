// reports/range/formatters/markdown/range_md_formatter.hpp
#ifndef REPORTS_RANGE_FORMATTERS_MARKDOWN_RANGE_MD_FORMATTER_H_
#define REPORTS_RANGE_FORMATTERS_MARKDOWN_RANGE_MD_FORMATTER_H_

#include <memory>
#include <string>

#include "reports/data/model/range_report_data.hpp"
#include "reports/range/formatters/markdown/range_md_config.hpp"
#include "reports/shared/formatters/templates/base_md_formatter.hpp"

class RangeMdFormatter
    : public BaseMdFormatter<RangeReportData, RangeMdConfig> {
 public:
  explicit RangeMdFormatter(std::shared_ptr<RangeMdConfig> config);

 private:
  std::string validate_data(const RangeReportData& data) const override;
  bool is_empty_data(const RangeReportData& data) const override;
  int get_avg_days(const RangeReportData& data) const override;
  std::string get_no_records_msg() const override;
  void format_header_content(std::stringstream& ss,
                             const RangeReportData& data) const override;
};

#endif  // REPORTS_RANGE_FORMATTERS_MARKDOWN_RANGE_MD_FORMATTER_H_
