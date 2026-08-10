// infra/insights/daily/formatters/latex/day_tex_utils.cpp
#include "infra/insights/daily/formatters/latex/day_tex_utils.hpp"

#include <vector>

#include "infra/insights/shared/formatters/latex/tex_common_utils.hpp"
#include "infra/insights/shared/formatters/latex/tex_utils.hpp"
#include "infra/insights/shared/utils/format/insights_string_utils.hpp"
#include "infra/insights/shared/utils/format/time_format.hpp"

namespace DayTexUtils {

void DisplayHeader(std::string& insights_stream, const DailyInsightsData& data,
                   const std::shared_ptr<DayTexConfig>& config) {
  // 1. 渲染标题
  std::string title_content =
      config->GetInsightsTitle() + " " + TexUtils::EscapeLatex(data.date);
  TexCommonUtils::RenderTitle(insights_stream, title_content,
                              config->GetInsightsTitleFontSize());

  // 2. 准备列表数据
  // 处理多行备注：先转义，再添加 LaTeX 换行符 (\\)
  std::string safe_remark = TexUtils::EscapeLatex(data.metadata.remark);
  std::string formatted_remark = FormatMultilineForList(safe_remark, 0, "\\\\");

  std::vector<TexCommonUtils::SummaryItem> items = {
      {config->GetDateLabel(), TexUtils::EscapeLatex(data.date)},
      {config->GetTotalTimeLabel(),
       TexUtils::EscapeLatex(TimeFormatDuration(data.total_duration))},
      {config->GetActivityCountLabel(), std::to_string(data.activity_count)}};

  for (const auto& status : data.metadata.statuses) {
    items.emplace_back(status.label, status.value ? "true" : "false");
  }

  items.emplace_back(config->GetGetupTimeLabel(),
                     TexUtils::EscapeLatex(data.metadata.getup_time));
  items.emplace_back(config->GetRemarkLabel(), formatted_remark);

  // 3. 渲染列表
  TexCommonUtils::RenderSummaryList(insights_stream, items,
                                    config->GetListTopSepPt(),
                                    config->GetListItemSepEx());
}

void DisplayDetailedActivities(std::string& insights_stream,
                               const DailyInsightsData& data,
                               const std::shared_ptr<DayTexConfig>& config) {
  if (data.detailed_records.empty()) {
    return;
  }

  // [优化] 使用通用函数渲染子标题
  TexCommonUtils::RenderTitle(insights_stream, config->GetAllActivitiesLabel(),
                              config->GetCategoryTitleFontSize(),
                              true  // is_subsection
  );

  std::string compact_list_options = TexCommonUtils::BuildCompactListOptions(
      config->GetListTopSepPt(), config->GetListItemSepEx());
  insights_stream += "\\begin{itemize}";
  insights_stream += compact_list_options;
  insights_stream += "\n";

  for (const auto& record : data.detailed_records) {
    std::string project_path =
        ReplaceAll(record.project_path, "_", config->GetActivityConnector());
    std::string base_string =
        TexUtils::EscapeLatex(FormatClockTime(record.start_time)) + " - " +
        TexUtils::EscapeLatex(FormatClockTime(record.end_time)) + " (" +
        TexUtils::EscapeLatex(TimeFormatDuration(record.duration_seconds)) +
        "): " + TexUtils::EscapeLatex(project_path);

    std::string colorized_string = base_string;

    for (const auto& pair : config->GetKeywordColors()) {
      if (record.project_path.find(pair.first) != std::string::npos) {
        colorized_string =
            "\\textcolor{" + pair.first + "color}{" + base_string + "}";
        break;
      }
    }

    insights_stream += "    \\item ";
    insights_stream += colorized_string;
    insights_stream += "\n";

    const auto& activity_remark_opt = record.activityRemark;
    if (activity_remark_opt.has_value()) {
      insights_stream += "    \\begin{itemize}";
      insights_stream += compact_list_options;
      insights_stream += "\n";

      const std::string& activity_remark = activity_remark_opt.value();

      std::string safe_activity_remark = TexUtils::EscapeLatex(activity_remark);
      std::string formatted_activity_remark =
          FormatMultilineForList(safe_activity_remark, 0, "\\\\");

      // Put the label and body on separate lines so a multiline remark remains
      // visually subordinate to the activity instead of widening the label.
      insights_stream += "        \\item \\textbf{";
      insights_stream += config->GetActivityRemarkLabel();
      insights_stream += "}:\\\\\n        ";
      insights_stream += formatted_activity_remark;
      insights_stream += "\n";
      insights_stream += "    \\end{itemize}\n";
    }
  }
  insights_stream += "\\end{itemize}\n\n";
}

}  // namespace DayTexUtils
