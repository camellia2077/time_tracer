// infrastructure/reports/range/formatters/latex/range_tex_config.hpp
#ifndef REPORTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_CONFIG_H_
#define REPORTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_CONFIG_H_

#include <string>

#include "infrastructure/reports/range/common/range_base_config.hpp"
#include "infrastructure/reports/shared/config/tex_style_config.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"

class RangeTexConfig : public RangeBaseConfig {
 public:
  explicit RangeTexConfig(const TtRangeTexConfigV1& config);
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
};

#endif  // REPORTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_CONFIG_H_
