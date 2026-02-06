// reports/monthly/formatters/typst/month_typ_config.hpp
#ifndef REPORTS_MONTHLY_FORMATTERS_TYPST_MONTH_TYP_CONFIG_H_
#define REPORTS_MONTHLY_FORMATTERS_TYPST_MONTH_TYP_CONFIG_H_

#include <toml++/toml.h>

#include <string>

#include "reports/monthly/common/month_base_config.hpp"
#include "reports/shared/config/typst_style_config.hpp"

class MonthTypConfig : public MonthBaseConfig {
 public:
  explicit MonthTypConfig(const toml::table& config);

  [[nodiscard]] auto GetBaseFont() const -> const std::string& {
    return style_.GetBaseFont();
  }
  [[nodiscard]] auto GetTitleFont() const -> const std::string& {
    return style_.GetTitleFont();
  }
  [[nodiscard]] auto GetCategoryTitleFont() const -> const std::string& {
    return style_.GetCategoryTitleFont();
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
  [[nodiscard]] auto GetLineSpacingEm() const -> double {
    return style_.GetLineSpacingEm();
  }

  [[nodiscard]] auto GetMarginTopCm() const -> double;
  [[nodiscard]] auto GetMarginBottomCm() const -> double;
  [[nodiscard]] auto GetMarginLeftCm() const -> double;
  [[nodiscard]] auto GetMarginRightCm() const -> double;

 private:
  TypstStyleConfig style_;
  double margin_top_cm_;
  double margin_bottom_cm_;
  double margin_left_cm_;
  double margin_right_cm_;
};

#endif  // REPORTS_MONTHLY_FORMATTERS_TYPST_MONTH_TYP_CONFIG_H_