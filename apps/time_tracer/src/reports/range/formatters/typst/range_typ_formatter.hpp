// reports/range/formatters/typst/range_typ_formatter.hpp
#ifndef REPORTS_RANGE_FORMATTERS_TYPST_RANGE_TYP_FORMATTER_H_
#define REPORTS_RANGE_FORMATTERS_TYPST_RANGE_TYP_FORMATTER_H_

#include <memory>

#include "reports/data/model/range_report_data.hpp"
#include "reports/range/formatters/typst/range_typ_config.hpp"
#include "reports/shared/formatters/templates/base_typ_formatter.hpp"

class RangeTypFormatter
    : public BaseTypFormatter<RangeReportData, RangeTypConfig> {
 public:
  explicit RangeTypFormatter(std::shared_ptr<RangeTypConfig> config);

 private:
  std::string validate_data(const RangeReportData& data) const override;
  bool is_empty_data(const RangeReportData& data) const override;
  int get_avg_days(const RangeReportData& data) const override;
  std::string get_no_records_msg() const override;
  void format_page_setup(std::stringstream& ss) const override;
  void format_header_content(std::stringstream& ss,
                             const RangeReportData& data) const override;
};

#endif  // REPORTS_RANGE_FORMATTERS_TYPST_RANGE_TYP_FORMATTER_H_
