// infrastructure/reports/range/formatters/typst/range_typ_formatter.hpp
#ifndef REPORTS_RANGE_FORMATTERS_TYPST_RANGE_TYP_FORMATTER_H_
#define REPORTS_RANGE_FORMATTERS_TYPST_RANGE_TYP_FORMATTER_H_

#include <memory>

#include "domain/reports/models/range_report_data.hpp"
#include "infrastructure/reports/range/formatters/typst/range_typ_config.hpp"
#include "infrastructure/reports/shared/formatters/templates/base_typ_formatter.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"

class RangeTypFormatter
    : public BaseTypFormatter<RangeReportData, RangeTypConfig> {
 public:
  explicit RangeTypFormatter(std::shared_ptr<RangeTypConfig> config);
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
  void FormatPageSetup(std::string& report_stream) const override;
  void FormatHeaderContent(std::string& report_stream,
                           const RangeReportData& data) const override;
};

#endif  // REPORTS_RANGE_FORMATTERS_TYPST_RANGE_TYP_FORMATTER_H_
