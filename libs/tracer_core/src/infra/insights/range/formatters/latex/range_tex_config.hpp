// infra/insights/range/formatters/latex/range_tex_config.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_CONFIG_H_
#define INFRASTRUCTURE_INSIGHTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_CONFIG_H_

#include <string>

#include "infra/config/models/insights_config_models.hpp"
#include "infra/insights/range/common/range_base_config.hpp"
#include "infra/insights/shared/config/tex_style_config.hpp"

class RangeTexConfig : public RangeBaseConfig {
 public:
  RangeTexConfig(const RangeInsightsLabels& labels, const FontConfig& fonts,
                 const LayoutConfig& layout);
  [[nodiscard]] auto GetMainFont() const -> const std::string& {
    return style_.GetMainFont();
  }
  [[nodiscard]] auto GetCjkMainFont() const -> const std::string& {
    return style_.GetCjkMainFont();
  }
  [[nodiscard]] auto GetBaseFontSize() const -> int {
    return style_.GetBaseFontSize();
  }
  [[nodiscard]] auto GetInsightsTitleFontSize() const -> int {
    return style_.GetInsightsTitleFontSize();
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

#endif  // INFRASTRUCTURE_INSIGHTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_CONFIG_H_
