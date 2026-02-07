// infrastructure/reports/monthly/formatters/latex/month_tex_config.hpp
#ifndef REPORTS_MONTHLY_FORMATTERS_LATEX_MONTH_TEX_CONFIG_H_
#define REPORTS_MONTHLY_FORMATTERS_LATEX_MONTH_TEX_CONFIG_H_

#include <toml++/toml.h>

#include <string>

#include "infrastructure/reports/monthly/common/month_base_config.hpp"
#include "infrastructure/reports/shared/config/tex_style_config.hpp"

class MonthTexConfig : public MonthBaseConfig {
 public:
  explicit MonthTexConfig(const toml::table& config);
  [[nodiscard]] auto GetMainFont() const -> const std::string& {
    return style_.GetMainFont();
  }
  [[nodiscard]] auto GetCjkMainFont() const -> const std::string& {
    return style_.GetCjkMainFont();
  }
  [[nodiscard]] auto GetBaseFontSize() const -> int {
    return style_.GetBaseFontSize();
  }
  [[nodiscard]] auto GetReportTitleFontSize() const -> int {
    return style_.GetReportTitleFontSize();
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
};

#endif  // REPORTS_MONTHLY_FORMATTERS_LATEX_MONTH_TEX_CONFIG_H_