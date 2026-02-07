// infrastructure/reports/shared/formatters/templates/base_md_formatter.hpp
#ifndef REPORTS_SHARED_FORMATTERS_TEMPLATES_BASE_MD_FORMATTER_H_
#define REPORTS_SHARED_FORMATTERS_TEMPLATES_BASE_MD_FORMATTER_H_

#include <format>
#include <memory>
#include <sstream>
#include <string>

#include "infrastructure/reports/shared/formatters/markdown/markdown_formatter.hpp"
#include "infrastructure/reports/shared/interfaces/i_report_formatter.hpp"

/**
 * @class BaseMdFormatter
 * @brief Markdown 报告格式化器的通用模板基类。
 */
template <typename ReportDataT, typename ConfigT>
class BaseMdFormatter : public IReportFormatter<ReportDataT> {
 public:
  explicit BaseMdFormatter(std::shared_ptr<ConfigT> config)
      : config_(std::move(config)) {}

  [[nodiscard]] auto FormatReport(const ReportDataT& data) const
      -> std::string override {
    // 1. 数据有效性检查
    if (std::string err = ValidateData(data); !err.empty()) {
      return err + "\n";  // Markdown 通常多加个换行比较安全
    }

    std::stringstream report_stream;

    // 2. 头部 / 摘要
    FormatHeaderContent(report_stream, data);

    // 3. 主体内容
    if (IsEmptyData(data)) {
      // [修改] 调用纯虚函数，由子类负责适配具体的 Config 接口
      report_stream << GetNoRecordsMsg() << "\n";
    } else {
      FormatExtraContent(report_stream, data);
      FormatProjectTreeSection(report_stream, data);
    }

    return report_stream.str();
  }

 protected:
  std::shared_ptr<ConfigT> config_;

  // [修改] 注释掉未使用参数以消除警告
  [[nodiscard]] virtual auto ValidateData(const ReportDataT& /*data*/) const
      -> std::string {
    return "";
  }

  [[nodiscard]] virtual auto IsEmptyData(const ReportDataT& data) const
      -> bool = 0;
  [[nodiscard]] virtual auto GetAvgDays(const ReportDataT& data) const
      -> int = 0;

  virtual void FormatHeaderContent(std::stringstream& report_stream,
                                   const ReportDataT& data) const = 0;

  // [修改] 注释掉未使用参数
  virtual void FormatExtraContent(std::stringstream& /*report_stream*/,
                                  const ReportDataT& /*data*/) const {}

  // [修改] 改为纯虚函数，移除导致编译错误的默认实现
  [[nodiscard]] virtual auto GetNoRecordsMsg() const -> std::string = 0;

  virtual void FormatProjectTreeSection(std::stringstream& report_stream,
                                        const ReportDataT& data) const {
    report_stream << "\n## " << config_->GetProjectBreakdownLabel() << "\n";
    report_stream << MarkdownFormatter::FormatProjectTree(
        data.project_tree, data.total_duration, GetAvgDays(data));
  }
};

#endif  // REPORTS_SHARED_FORMATTERS_TEMPLATES_BASE_MD_FORMATTER_H_