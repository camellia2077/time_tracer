// infra/insights/shared/formatters/templates/base_tex_formatter.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_SHARED_FORMATTERS_TEMPLATES_BASE_TEX_FORMATTER_H_
#define INFRASTRUCTURE_INSIGHTS_SHARED_FORMATTERS_TEMPLATES_BASE_TEX_FORMATTER_H_

#include <map>
#include <memory>
#include <string>

#include "infra/insights/shared/formatters/latex/tex_utils.hpp"
#include "infra/insights/shared/interfaces/i_insights_formatter.hpp"

template <typename InsightsDataT, typename ConfigT>
class BaseTexFormatter : public IInsightsFormatter<InsightsDataT> {
 public:
  explicit BaseTexFormatter(std::shared_ptr<ConfigT> config)
      : config_(std::move(config)) {}

  [[nodiscard]] auto FormatInsights(const InsightsDataT& data) const
      -> std::string override {
    if (std::string error = ValidateData(data); !error.empty()) {
      return error;
    }

    std::string output;
    output += GeneratePreamble();

    FormatHeaderContent(output, data);

    if (IsEmptyData(data)) {
      output += GetNoRecordsMsg();
      output += "\n";
    } else {
      FormatExtraContent(output, data);
      FormatProjectTreeSection(output, data);
    }

    output += GeneratePostfix();
    return output;
  }

 protected:
  std::shared_ptr<ConfigT> config_;

  [[nodiscard]] virtual auto ValidateData(const InsightsDataT& /*data*/) const
      -> std::string {
    return "";
  }

  [[nodiscard]] virtual auto IsEmptyData(const InsightsDataT& data) const
      -> bool = 0;
  [[nodiscard]] virtual auto GetAvgDays(const InsightsDataT& data) const
      -> int = 0;
  [[nodiscard]] virtual auto GetNoRecordsMsg() const -> std::string = 0;

  virtual void FormatHeaderContent(std::string& output,
                                   const InsightsDataT& data) const = 0;
  virtual void FormatExtraContent(std::string& /*output*/,
                                  const InsightsDataT& /*data*/) const {}

  [[nodiscard]] virtual auto GetKeywordColors() const
      -> std::map<std::string, std::string> {
    return {};
  }

  [[nodiscard]] virtual auto GeneratePreamble() const -> std::string {
    return TexUtils::GetTexPreamble(
        config_->GetMainFont(), config_->GetCjkMainFont(),
        config_->GetBaseFontSize(), config_->GetMarginIn(), GetKeywordColors());
  }

  virtual void FormatProjectTreeSection(std::string& output,
                                        const InsightsDataT& data) const {
    constexpr int kLineHeightScale = 12;
    constexpr int kLineHeightDivisor = 10;
    const int kTitleSize = config_->GetCategoryTitleFontSize();
    const int kLineHeightTenths = kTitleSize * kLineHeightScale;

    output += "{";
    output += "\\fontsize{";
    output += std::to_string(kTitleSize);
    output += "}{";
    output += std::to_string(kLineHeightTenths / kLineHeightDivisor);
    output += ".";
    output += std::to_string(kLineHeightTenths % kLineHeightDivisor);
    output += "}\\selectfont";
    output += "\\section*{";
    output += TexUtils::EscapeLatex(config_->GetProjectBreakdownLabel());
    output += "}";
    output += "}\n\n";

    output += TexUtils::FormatProjectTree(
        data.project_tree, data.activity.total_duration_seconds,
        GetAvgDays(data), config_->GetCategoryTitleFontSize(),
        config_->GetListTopSepPt(), config_->GetListItemSepEx());
  }

  [[nodiscard]] virtual auto GeneratePostfix() const -> std::string {
    return TexUtils::GetTexPostfix();
  }
};

#endif  // INFRASTRUCTURE_INSIGHTS_SHARED_FORMATTERS_TEMPLATES_BASE_TEX_FORMATTER_H_
