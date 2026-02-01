// reports/daily/formatters/typst/day_typ_config.hpp
#ifndef REPORTS_DAILY_FORMATTERS_TYPST_DAY_TYP_CONFIG_H_
#define REPORTS_DAILY_FORMATTERS_TYPST_DAY_TYP_CONFIG_H_

#include <toml++/toml.h>

#include <map>
#include <string>

#include "reports/daily/common/day_base_config.hpp"
#include "reports/shared/config/typst_style_config.hpp"

class DayTypConfig : public DayBaseConfig {
 public:
  static constexpr int kDefaultStatFontSize = 10;
  static constexpr int kDefaultStatTitleFontSize = 12;

  explicit DayTypConfig(const toml::table& config);

  [[nodiscard]] auto get_statistic_font_size() const -> int;
  [[nodiscard]] auto get_statistic_title_font_size() const -> int;
  [[nodiscard]] auto get_keyword_colors() const
      -> const std::map<std::string, std::string>&;

  // 代理
  [[nodiscard]] auto get_title_font() const -> const std::string& {
    return style_.get_title_font();
  }
  [[nodiscard]] auto get_base_font() const -> const std::string& {
    return style_.get_base_font();
  }
  [[nodiscard]] auto get_category_title_font() const -> const std::string& {
    return style_.get_category_title_font();
  }
  [[nodiscard]] auto get_base_font_size() const -> int {
    return style_.get_base_font_size();
  }
  [[nodiscard]] auto get_report_title_font_size() const -> int {
    return style_.get_report_title_font_size();
  }
  [[nodiscard]] auto get_category_title_font_size() const -> int {
    return style_.get_category_title_font_size();
  }
  [[nodiscard]] auto get_line_spacing_em() const -> double {
    return style_.get_line_spacing_em();
  }

 private:
  TypstStyleConfig style_;
  int statistic_font_size_;
  int statistic_title_font_size_;
  std::map<std::string, std::string> keyword_colors_;
};

#endif  // REPORTS_DAILY_FORMATTERS_TYPST_DAY_TYP_CONFIG_H_