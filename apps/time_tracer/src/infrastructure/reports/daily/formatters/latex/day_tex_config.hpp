// infrastructure/reports/daily/formatters/latex/day_tex_config.hpp
#ifndef INFRASTRUCTURE_REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_CONFIG_H_
#define INFRASTRUCTURE_REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_CONFIG_H_

#include <map>
#include <string>

#include "infrastructure/reports/daily/common/day_base_config.hpp"
#include "infrastructure/reports/shared/config/tex_style_config.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"

class DayTexConfig : public DayBaseConfig {
 public:
  explicit DayTexConfig(const TtDayTexConfigV1& config);

  [[nodiscard]] auto GetReportTitle() const -> const std::string&;
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
  std::string report_title_;
  std::map<std::string, std::string> keyword_colors_;
};

#endif  // INFRASTRUCTURE_REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_CONFIG_H_
