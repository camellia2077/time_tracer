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
  auto ValidateData(const RangeReportData& data) const -> std::string override;
  auto IsEmptyData(const RangeReportData& data) const -> bool override;
  auto GetAvgDays(const RangeReportData& data) const -> int override;
  auto GetNoRecordsMsg() const -> std::string override;
  void FormatPageSetup(std::stringstream& report_stream) const override;
  void FormatHeaderContent(std::stringstream& report_stream,
                           const RangeReportData& data) const override;
};

#endif  // REPORTS_RANGE_FORMATTERS_TYPST_RANGE_TYP_FORMATTER_H_
