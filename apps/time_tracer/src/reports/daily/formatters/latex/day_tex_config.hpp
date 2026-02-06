// reports/daily/formatters/latex/day_tex_config.hpp
#ifndef REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_CONFIG_H_
#define REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_CONFIG_H_

#include <toml++/toml.h>

#include <map>
#include <string>

#include "reports/daily/common/day_base_config.hpp"
#include "reports/shared/config/tex_style_config.hpp"

class DayTexConfig : public DayBaseConfig {
 public:
  explicit DayTexConfig(const toml::table& config);

  [[nodiscard]] auto GetReportTitle() const -> const std::string&;
  [[nodiscard]] auto GetKeywordColors() const
      -> const std::map<std::string, std::string>&;

  // 代理给 style_ 的方法
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

#endif  // REPORTS_DAILY_FORMATTERS_LATEX_DAY_TEX_CONFIG_H_
