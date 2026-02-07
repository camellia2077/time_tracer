// infrastructure/reports/shared/formatters/templates/base_typ_formatter.hpp
#ifndef REPORTS_SHARED_FORMATTERS_TEMPLATES_BASE_TYP_FORMATTER_H_
#define REPORTS_SHARED_FORMATTERS_TEMPLATES_BASE_TYP_FORMATTER_H_

#include <format>
#include <memory>
#include <sstream>
#include <string>

#include "infrastructure/reports/shared/formatters/typst/typ_utils.hpp"
#include "infrastructure/reports/shared/interfaces/i_report_formatter.hpp"

/**
 * @class BaseTypFormatter
 * @brief Typst 报告格式化器的通用模板基类。
 */
template <typename ReportDataT, typename ConfigT>
class BaseTypFormatter : public IReportFormatter<ReportDataT> {
 public:
  explicit BaseTypFormatter(std::shared_ptr<ConfigT> config)
      : config_(std::move(config)) {}

  auto FormatReport(const ReportDataT& data) const -> std::string override {
    std::stringstream ss;

    // 1. 页面和基础文本设置
    FormatPageSetup(ss);
    FormatTextSetup(ss);

    // 2. 数据有效性检查
    if (std::string err = ValidateData(data); !err.empty()) {
      ss << err << "\n";
      return ss.str();
    }

    // 3. 头部 / 摘要
    FormatHeaderContent(ss, data);

    // 4. 主体内容
    if (IsEmptyData(data)) {
      // [修改] 调用纯虚函数
      ss << GetNoRecordsMsg() << "\n";
    } else {
      FormatExtraContent(ss, data);
      FormatProjectTreeSection(ss, data);
    }

    return ss.str();
  }

 protected:
  std::shared_ptr<ConfigT> config_;

  // [修改] 注释掉未使用参数
  [[nodiscard]] virtual auto ValidateData(const ReportDataT& /*data*/) const
      -> std::string {
    return "";
  }

  [[nodiscard]] virtual auto IsEmptyData(const ReportDataT& data) const
      -> bool = 0;
  [[nodiscard]] virtual auto GetAvgDays(const ReportDataT& data) const
      -> int = 0;

  // [新增] 纯虚函数
  [[nodiscard]] virtual auto GetNoRecordsMsg() const -> std::string = 0;

  virtual void FormatHeaderContent(std::stringstream& ss,
                                   const ReportDataT& data) const = 0;

  // [修改] 注释掉未使用参数
  virtual void FormatExtraContent(std::stringstream& /*ss*/,
                                  const ReportDataT& /*data*/) const {}

  // [修改] 注释掉未使用参数
  virtual void FormatPageSetup(std::stringstream& /*ss*/) const {}

  virtual void FormatTextSetup(std::stringstream& ss) const {
    std::string spacing_str =
        std::to_string(config_->GetLineSpacingEm()) + "em";
    ss << std::format(R"(#set text(font: "{}", size: {}pt, spacing: {}))",
                      config_->GetBaseFont(), config_->GetBaseFontSize(),
                      spacing_str)
       << "\n\n";
  }

  virtual void FormatProjectTreeSection(std::stringstream& ss,
                                        const ReportDataT& data) const {
    // 统领性标题
    ss << std::format(R"(#text(font: "{}", size: {}pt)[= {}])",
                      config_->GetCategoryTitleFont(),
                      config_->GetCategoryTitleFontSize(),
                      config_->GetProjectBreakdownLabel())
       << "\n\n";

    ss << TypUtils::FormatProjectTree(
        data.project_tree, data.total_duration, GetAvgDays(data),
        config_->GetCategoryTitleFont(), config_->GetCategoryTitleFontSize());
  }
};

#endif  // REPORTS_SHARED_FORMATTERS_TEMPLATES_BASE_TYP_FORMATTER_H_