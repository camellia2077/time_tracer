// infrastructure/reports/range/formatters/markdown/range_md_formatter.hpp
#ifndef INFRASTRUCTURE_REPORTS_RANGE_FORMATTERS_MARKDOWN_RANGE_MD_FORMATTER_H_
#define INFRASTRUCTURE_REPORTS_RANGE_FORMATTERS_MARKDOWN_RANGE_MD_FORMATTER_H_

#include <memory>
#include <string>

#include "domain/reports/models/range_report_data.hpp"
#include "infrastructure/reports/range/formatters/markdown/range_md_config.hpp"
#include "infrastructure/reports/shared/formatters/templates/base_md_formatter.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"

class RangeMdFormatter
    : public BaseMdFormatter<RangeReportData, RangeMdConfig> {
 public:
  explicit RangeMdFormatter(std::shared_ptr<RangeMdConfig> config);
  [[nodiscard]] auto FormatReportFromView(
      const TtRangeReportDataV1& data_view) const -> std::string;

 private:
  [[nodiscard]] auto ValidateData(const RangeReportData& data) const
      -> std::string override;
  [[nodiscard]] auto IsEmptyData(const RangeReportData& data) const
      -> bool override;
  [[nodiscard]] auto GetAvgDays(const RangeReportData& data) const
      -> int override;
  [[nodiscard]] auto GetNoRecordsMsg() const -> std::string override;
  void FormatHeaderContent(std::string& report_stream,
                           const RangeReportData& data) const override;
};

#endif  // INFRASTRUCTURE_REPORTS_RANGE_FORMATTERS_MARKDOWN_RANGE_MD_FORMATTER_H_
