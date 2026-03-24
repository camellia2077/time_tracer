// infra/reporting/shared/config/tex_style_config.hpp
#ifndef INFRASTRUCTURE_REPORTS_SHARED_CONFIG_TEX_STYLE_CONFIG_H_
#define INFRASTRUCTURE_REPORTS_SHARED_CONFIG_TEX_STYLE_CONFIG_H_

#include <string>

#include "infra/config/models/report_config_models.hpp"
#include "infra/reporting/shared/api/shared_api.hpp"

DISABLE_C4251_WARNING

class REPORTS_SHARED_API TexStyleConfig {
 public:
  TexStyleConfig(const FontConfig& fonts, const LayoutConfig& layout);

  [[nodiscard]] auto GetMainFont() const -> const std::string&;
  [[nodiscard]] auto GetCjkMainFont() const -> const std::string&;
  [[nodiscard]] auto GetBaseFontSize() const -> int;
  [[nodiscard]] auto GetReportTitleFontSize() const -> int;
  [[nodiscard]] auto GetCategoryTitleFontSize() const -> int;
  [[nodiscard]] auto GetMarginIn() const -> double;
  [[nodiscard]] auto GetListTopSepPt() const -> double;
  [[nodiscard]] auto GetListItemSepEx() const -> double;

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

#endif  // INFRASTRUCTURE_REPORTS_SHARED_CONFIG_TEX_STYLE_CONFIG_H_
