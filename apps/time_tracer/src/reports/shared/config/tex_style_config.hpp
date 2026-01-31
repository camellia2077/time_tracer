// reports/shared/config/tex_style_config.hpp
#ifndef REPORTS_SHARED_CONFIG_TEX_STYLE_CONFIG_H_
#define REPORTS_SHARED_CONFIG_TEX_STYLE_CONFIG_H_

#include <toml++/toml.h>

#include <string>

#include "reports/shared/api/shared_api.hpp"

DISABLE_C4251_WARNING

class REPORTS_SHARED_API TexStyleConfig {
 public:
  explicit TexStyleConfig(const toml::table& config);

  const std::string& get_main_font() const;
  const std::string& get_cjk_main_font() const;
  int get_base_font_size() const;
  int get_report_title_font_size() const;
  int get_category_title_font_size() const;
  double get_margin_in() const;
  double get_list_top_sep_pt() const;
  double get_list_item_sep_ex() const;

 private:
  std::string main_font_;
  std::string cjk_main_font_;
  int base_font_size_;
  int report_title_font_size_;
  int category_title_font_size_;
  double margin_in_;
  double list_top_sep_pt_;
  double list_item_sep_ex_;
};

ENABLE_C4251_WARNING

#endif  // REPORTS_SHARED_CONFIG_TEX_STYLE_CONFIG_H_