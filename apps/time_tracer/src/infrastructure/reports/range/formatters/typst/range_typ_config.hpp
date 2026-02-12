// infrastructure/reports/range/formatters/typst/range_typ_config.hpp
#ifndef REPORTS_RANGE_FORMATTERS_TYPST_RANGE_TYP_CONFIG_H_
#define REPORTS_RANGE_FORMATTERS_TYPST_RANGE_TYP_CONFIG_H_

#include <string>

#include "infrastructure/reports/range/common/range_base_config.hpp"
#include "infrastructure/reports/shared/config/typst_style_config.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"

class RangeTypConfig : public RangeBaseConfig {
 public:
  explicit RangeTypConfig(const TtRangeTypConfigV1& config);

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

#endif  // REPORTS_RANGE_FORMATTERS_TYPST_RANGE_TYP_CONFIG_H_
