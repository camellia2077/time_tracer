// infra/insights/daily/formatters/latex/daily_tex_formatter_config.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_LATEX_DAILY_TEX_FORMATTER_CONFIG_H_
#define INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_LATEX_DAILY_TEX_FORMATTER_CONFIG_H_

#include <map>
#include <string>

#include "infra/config/models/insights_config_models.hpp"
#include "infra/insights/daily/common/daily_formatter_base_config.hpp"
#include "infra/insights/shared/config/tex_style_config.hpp"

class DailyTexFormatterConfig : public DailyFormatterBaseConfig {
 public:
  explicit DailyTexFormatterConfig(const DailyTexConfig& config);

  [[nodiscard]] auto GetKeywordColors() const
      -> const std::map<std::string, std::string>&;

  [[nodiscard]] auto GetMainFont() const -> const std::string& {
    return style_.GetMainFont();
  }
  [[nodiscard]] auto GetCjkMainFont() const -> const std::string& {
    return style_.GetCjkMainFont();
  }
  [[nodiscard]] auto GetBaseFontSize() const -> int {
    return style_.GetBaseFontSize();
  }
  [[nodiscard]] auto GetInsightsTitleFontSize() const -> int {
    return style_.GetInsightsTitleFontSize();
  }
  [[nodiscard]] auto GetCategoryTitleFontSize() const -> int {
    return style_.GetCategoryTitleFontSize();
  }
  [[nodiscard]] auto GetMarginIn() const -> double {
    return style_.GetMarginIn();
  }
  [[nodiscard]] auto GetListTopSepPt() const -> double {
    return style_.GetListTopSepPt();
  }
  [[nodiscard]] auto GetListItemSepEx() const -> double {
    return style_.GetListItemSepEx();
  }

 private:
  TexStyleConfig style_;
  std::map<std::string, std::string> keyword_colors_;
};

#endif  // INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_LATEX_DAILY_TEX_FORMATTER_CONFIG_H_
