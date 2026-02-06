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
  [[nodiscard]] auto ValidateData(const RangeReportData& data) const
      -> std::string override;
  auto IsEmptyData(const RangeReportData& data) const -> bool override;
  auto GetAvgDays(const RangeReportData& data) const -> int override;
  auto GetNoRecordsMsg() const -> std::string override;
  void FormatHeaderContent(std::stringstream& report_stream,
                           const RangeReportData& data) const override;
};

#endif  // REPORTS_RANGE_FORMATTERS_MARKDOWN_RANGE_MD_FORMATTER_H_
