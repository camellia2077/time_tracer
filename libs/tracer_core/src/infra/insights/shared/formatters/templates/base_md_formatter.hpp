// infra/insights/shared/formatters/templates/base_md_formatter.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_SHARED_FORMATTERS_TEMPLATES_BASE_MD_FORMATTER_H_
#define INFRASTRUCTURE_INSIGHTS_SHARED_FORMATTERS_TEMPLATES_BASE_MD_FORMATTER_H_

#include <memory>
#include <string>

#include "infra/insights/shared/formatters/markdown/markdown_formatter.hpp"
#include "infra/insights/shared/interfaces/i_insights_formatter.hpp"

/**
 * @class BaseMdFormatter
 * @brief Markdown 报告格式化器的通用模板基类。
 */
template <typename InsightsDataT, typename ConfigT>
class BaseMdFormatter : public IInsightsFormatter<InsightsDataT> {
 public:
  explicit BaseMdFormatter(std::shared_ptr<ConfigT> config)
      : config_(std::move(config)) {}

  [[nodiscard]] auto FormatInsights(const InsightsDataT& data) const
      -> std::string override {
    // 1. 数据有效性检查
    if (std::string err = ValidateData(data); !err.empty()) {
      return err + "\n";  // Markdown 通常多加个换行比较安全
    }

    std::string insights_stream;

    // 2. 头部 / 摘要
    FormatHeaderContent(insights_stream, data);

    // 3. 主体内容
    if (IsEmptyData(data)) {
      // [修改] 调用纯虚函数，由子类负责适配具体的 Config 接口
      insights_stream += GetNoRecordsMsg();
      insights_stream += "\n";
    } else {
      FormatProjectTreeSection(insights_stream, data);
      FormatExtraContent(insights_stream, data);
    }

    return insights_stream;
  }

 protected:
  std::shared_ptr<ConfigT> config_;

  // [修改] 注释掉未使用参数以消除警告
  [[nodiscard]] virtual auto ValidateData(const InsightsDataT& /*data*/) const
      -> std::string {
    return "";
  }

  [[nodiscard]] virtual auto IsEmptyData(const InsightsDataT& data) const
      -> bool = 0;
  [[nodiscard]] virtual auto GetAvgDays(const InsightsDataT& data) const
      -> int = 0;

  virtual void FormatHeaderContent(std::string& insights_stream,
                                   const InsightsDataT& data) const = 0;

  // [修改] 注释掉未使用参数
  virtual void FormatExtraContent(std::string& /*insights_stream*/,
                                  const InsightsDataT& /*data*/) const {}

  // [修改] 改为纯虚函数，移除导致编译错误的默认实现
  [[nodiscard]] virtual auto GetNoRecordsMsg() const -> std::string = 0;

  virtual void FormatProjectTreeSection(std::string& insights_stream,
                                        const InsightsDataT& data) const {
    insights_stream += "\n## ";
    insights_stream += config_->GetProjectBreakdownLabel();
    insights_stream += "\n";
    insights_stream += MarkdownFormatter::FormatProjectTree(
        data.project_tree, data.total_duration, GetAvgDays(data));
  }
};

#endif  // INFRASTRUCTURE_INSIGHTS_SHARED_FORMATTERS_TEMPLATES_BASE_MD_FORMATTER_H_
