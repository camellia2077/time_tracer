// infrastructure/reports/shared/formatters/templates/base_typ_formatter.hpp
#ifndef INFRASTRUCTURE_REPORTS_SHARED_FORMATTERS_TEMPLATES_BASE_TYP_FORMATTER_H_
#define INFRASTRUCTURE_REPORTS_SHARED_FORMATTERS_TEMPLATES_BASE_TYP_FORMATTER_H_

#include <memory>
#include <string>

#include "infrastructure/reports/shared/formatters/typst/typ_utils.hpp"
#include "infrastructure/reports/shared/interfaces/i_report_formatter.hpp"

template <typename ReportDataT, typename ConfigT>
class BaseTypFormatter : public IReportFormatter<ReportDataT> {
 public:
  explicit BaseTypFormatter(std::shared_ptr<ConfigT> config)
      : config_(std::move(config)) {}

  [[nodiscard]] auto FormatReport(const ReportDataT& data) const
      -> std::string override {
    std::string stream;

    FormatPageSetup(stream);
    FormatTextSetup(stream);

    if (std::string error = ValidateData(data); !error.empty()) {
      stream += error;
      stream += "\n";
      return stream;
    }

    FormatHeaderContent(stream, data);

    if (IsEmptyData(data)) {
      stream += GetNoRecordsMsg();
      stream += "\n";
    } else {
      FormatExtraContent(stream, data);
      FormatProjectTreeSection(stream, data);
    }

    return stream;
  }

 protected:
  std::shared_ptr<ConfigT> config_;

  [[nodiscard]] virtual auto ValidateData(const ReportDataT& /*data*/) const
      -> std::string {
    return "";
  }

  [[nodiscard]] virtual auto IsEmptyData(const ReportDataT& data) const
      -> bool = 0;
  [[nodiscard]] virtual auto GetAvgDays(const ReportDataT& data) const
      -> int = 0;
  [[nodiscard]] virtual auto GetNoRecordsMsg() const -> std::string = 0;

  virtual void FormatHeaderContent(std::string& stream,
                                   const ReportDataT& data) const = 0;

  virtual void FormatExtraContent(std::string& /*stream*/,
                                  const ReportDataT& /*data*/) const {}

  virtual void FormatPageSetup(std::string& /*stream*/) const {}

  virtual void FormatTextSetup(std::string& stream) const {
    stream += TypUtils::BuildTextSetup(config_->GetBaseFont(),
                                       config_->GetBaseFontSize(),
                                       config_->GetLineSpacingEm());
    stream += "\n\n";
  }

  virtual void FormatProjectTreeSection(std::string& stream,
                                        const ReportDataT& data) const {
    stream += TypUtils::BuildTitleText(config_->GetCategoryTitleFont(),
                                       config_->GetCategoryTitleFontSize(),
                                       config_->GetProjectBreakdownLabel());
    stream += "\n\n";

    stream += TypUtils::FormatProjectTree(
        data.project_tree, data.total_duration, GetAvgDays(data),
        config_->GetCategoryTitleFont(), config_->GetCategoryTitleFontSize());
  }
};

#endif  // INFRASTRUCTURE_REPORTS_SHARED_FORMATTERS_TEMPLATES_BASE_TYP_FORMATTER_H_
