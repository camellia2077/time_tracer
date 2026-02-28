// infrastructure/reports/daily/formatters/typst/day_typ_config.hpp
#ifndef INFRASTRUCTURE_REPORTS_DAILY_FORMATTERS_TYPST_DAY_TYP_CONFIG_H_
#define INFRASTRUCTURE_REPORTS_DAILY_FORMATTERS_TYPST_DAY_TYP_CONFIG_H_

#include <map>
#include <string>

#include "infrastructure/reports/daily/common/day_base_config.hpp"
#include "infrastructure/reports/shared/config/typst_style_config.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"

class DayTypConfig : public DayBaseConfig {
 public:
  explicit DayTypConfig(const TtDayTypConfigV1& config);

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
  [[nodiscard]] auto GetReportTitleFontSize() const -> int {
    return style_.GetReportTitleFontSize();
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

#endif  // INFRASTRUCTURE_REPORTS_DAILY_FORMATTERS_TYPST_DAY_TYP_CONFIG_H_
