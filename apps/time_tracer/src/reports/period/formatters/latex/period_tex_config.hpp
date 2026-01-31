// reports/period/formatters/latex/period_tex_config.hpp
#ifndef REPORTS_PERIOD_FORMATTERS_LATEX_PERIOD_TEX_CONFIG_H_
#define REPORTS_PERIOD_FORMATTERS_LATEX_PERIOD_TEX_CONFIG_H_

#include <toml++/toml.h>

#include <string>

#include "reports/period/common/period_base_config.hpp"
#include "reports/shared/config/tex_style_config.hpp"

namespace reporting {

class PeriodTexConfig : public PeriodBaseConfig {
 public:
  explicit PeriodTexConfig(const toml::table& config);

  const std::string& get_main_font() const { return style_.get_main_font(); }
  const std::string& get_cjk_main_font() const {
    return style_.get_cjk_main_font();
  }
  int get_base_font_size() const { return style_.get_base_font_size(); }
  int get_report_title_font_size() const {
    return style_.get_report_title_font_size();
  }
  int get_category_title_font_size() const {
    return style_.get_category_title_font_size();
  }
  double get_margin_in() const { return style_.get_margin_in(); }
  double get_list_top_sep_pt() const { return style_.get_list_top_sep_pt(); }
  double get_list_item_sep_ex() const { return style_.get_list_item_sep_ex(); }

 private:
  TexStyleConfig style_;
};

}  // namespace reporting

#endif  // REPORTS_PERIOD_FORMATTERS_LATEX_PERIOD_TEX_CONFIG_H_