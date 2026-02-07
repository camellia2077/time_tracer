// infrastructure/reports/shared/formatters/templates/base_tex_formatter.hpp
#ifndef REPORTS_SHARED_FORMATTERS_TEMPLATES_BASE_TEX_FORMATTER_H_
#define REPORTS_SHARED_FORMATTERS_TEMPLATES_BASE_TEX_FORMATTER_H_

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "infrastructure/reports/shared/formatters/latex/tex_utils.hpp"
#include "infrastructure/reports/shared/interfaces/i_report_formatter.hpp"

/**
 * @class BaseTexFormatter
 * @brief LaTeX 报告格式化器的通用模板基类。
 */
template <typename ReportDataT, typename ConfigT>
class BaseTexFormatter : public IReportFormatter<ReportDataT> {
 public:
  explicit BaseTexFormatter(std::shared_ptr<ConfigT> config)
      : config_(config) {}

  [[nodiscard]] auto FormatReport(const ReportDataT& data) const
      -> std::string override {
    // 1. 数据有效性检查
    if (std::string err = ValidateData(data); !err.empty()) {
      return err;
    }

    std::stringstream ss;

    // 2. Preamble
    ss << GeneratePreamble();

    // 3. 头部 / 摘要
    FormatHeaderContent(ss, data);

    // 4. 主体内容
    if (IsEmptyData(data)) {
      // [修改] 统一使用 GetNoRecordsMsg() 钩子
      ss << GetNoRecordsMsg() << "\n";
    } else {
      // 钩子：用于 Daily 报告插入统计信息和详细活动记录
      FormatExtraContent(ss, data);

      // 项目树部分
      FormatProjectTreeSection(ss, data);
    }

    // 5. Postfix
    ss << GeneratePostfix();
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

  // [新增] 纯虚函数，强制子类适配 Config 接口
  [[nodiscard]] virtual auto GetNoRecordsMsg() const -> std::string = 0;

  virtual void FormatHeaderContent(std::stringstream& ss,
                                   const ReportDataT& data) const = 0;

  // [修改] 注释掉未使用参数
  virtual void FormatExtraContent(std::stringstream& /*ss*/,
                                  const ReportDataT& /*data*/) const {}

  [[nodiscard]] virtual auto GetKeywordColors() const
      -> std::map<std::string, std::string> {
    return std::map<std::string, std::string>{};
  }

  [[nodiscard]] virtual auto GeneratePreamble() const -> std::string {
    return TexUtils::GetTexPreamble(
        config_->GetMainFont(), config_->GetCjkMainFont(),
        config_->GetBaseFontSize(), config_->GetMarginIn(), GetKeywordColors());
  }

  virtual void FormatProjectTreeSection(std::stringstream& ss,
                                        const ReportDataT& data) const {
    int title_size = config_->GetCategoryTitleFontSize();
    ss << "{";
    ss << "\\fontsize{" << title_size << "}{" << title_size * 1.2
       << "}\\selectfont";
    ss << "\\section*{"
       << TexUtils::EscapeLatex(config_->GetProjectBreakdownLabel()) << "}";
    ss << "}\n\n";

    ss << TexUtils::FormatProjectTree(
        data.project_tree, data.total_duration, GetAvgDays(data),
        config_->GetCategoryTitleFontSize(), config_->GetListTopSepPt(),
        config_->GetListItemSepEx());
  }

  [[nodiscard]] virtual auto GeneratePostfix() const -> std::string {
    return TexUtils::GetTexPostfix();
  }
};

#endif  // REPORTS_SHARED_FORMATTERS_TEMPLATES_BASE_TEX_FORMATTER_H_