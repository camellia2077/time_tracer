// infrastructure/reports/shared/config/typst_style_config.hpp
#ifndef INFRASTRUCTURE_REPORTS_SHARED_CONFIG_TYPST_STYLE_CONFIG_H_
#define INFRASTRUCTURE_REPORTS_SHARED_CONFIG_TYPST_STYLE_CONFIG_H_

#include <string>

#include "infrastructure/reports/shared/api/shared_api.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"

DISABLE_C4251_WARNING

class REPORTS_SHARED_API TypstStyleConfig {
 public:
  explicit TypstStyleConfig(const TtTypstStyleConfigV1& config);

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
