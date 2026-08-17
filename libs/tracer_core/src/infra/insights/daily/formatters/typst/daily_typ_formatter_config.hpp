// infra/insights/daily/formatters/typst/daily_typ_formatter_config.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_TYPST_DAILY_TYP_FORMATTER_CONFIG_H_
#define INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_TYPST_DAILY_TYP_FORMATTER_CONFIG_H_

#include <map>
#include <string>

#include "infra/config/models/insights_config_models.hpp"
#include "infra/insights/daily/common/daily_formatter_base_config.hpp"
#include "infra/insights/shared/config/typst_style_config.hpp"

class DailyTypFormatterConfig : public DailyFormatterBaseConfig {
 public:
  explicit DailyTypFormatterConfig(const DailyTypConfig& config);

  [[nodiscard]] auto GetStatisticFontSize() const -> int;
  [[nodiscard]] auto GetStatisticTitleFontSize() const -> int;
  [[nodiscard]] auto GetKeywordColors() const
      -> const std::map<std::string, std::string>&;

  [[nodiscard]] auto GetTitleFont() const -> const std::string& {
    return style_.GetTitleFont();
  }
  [[nodiscard]] auto GetBaseFont() const -> const std::string& {
    return style_.GetBaseFont();
  }
  [[nodiscard]] auto GetCategoryTitleFont() const -> const std::string& {
    return style_.GetCategoryTitleFont();
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
  [[nodiscard]] auto GetLineSpacingEm() const -> double {
    return style_.GetLineSpacingEm();
  }

 private:
  TypstStyleConfig style_;
  int statistic_font_size_;
  int statistic_title_font_size_;
  std::map<std::string, std::string> keyword_colors_;
};

#endif  // INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_TYPST_DAILY_TYP_FORMATTER_CONFIG_H_
