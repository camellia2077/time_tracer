// infrastructure/reports/shared/config/typst_style_config.hpp
#ifndef INFRASTRUCTURE_REPORTS_SHARED_CONFIG_TYPST_STYLE_CONFIG_H_
#define INFRASTRUCTURE_REPORTS_SHARED_CONFIG_TYPST_STYLE_CONFIG_H_

#include <string>

#include "infrastructure/config/models/report_config_models.hpp"
#include "infrastructure/reports/shared/api/shared_api.hpp"

DISABLE_C4251_WARNING

class REPORTS_SHARED_API TypstStyleConfig {
 public:
  TypstStyleConfig(const FontConfig& fonts, const LayoutConfig& layout);

  [[nodiscard]] auto GetBaseFont() const -> const std::string&;
  [[nodiscard]] auto GetTitleFont() const -> const std::string&;
  [[nodiscard]] auto GetCategoryTitleFont() const -> const std::string&;
  [[nodiscard]] auto GetBaseFontSize() const -> int;
  [[nodiscard]] auto GetReportTitleFontSize() const -> int;
  [[nodiscard]] auto GetCategoryTitleFontSize() const -> int;
  [[nodiscard]] auto GetLineSpacingEm() const -> double;

 private:
  std::string base_font_;
  std::string title_font_;
  std::string category_title_font_;
  int base_font_size_;
  int report_title_font_size_;
  int category_title_font_size_;
  double line_spacing_em_;
};

ENABLE_C4251_WARNING

#endif  // INFRASTRUCTURE_REPORTS_SHARED_CONFIG_TYPST_STYLE_CONFIG_H_
