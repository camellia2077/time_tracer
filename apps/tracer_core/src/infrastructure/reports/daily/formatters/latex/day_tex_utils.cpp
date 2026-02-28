// infrastructure/reports/daily/formatters/latex/day_tex_utils.cpp
#include "infrastructure/reports/daily/formatters/latex/day_tex_utils.hpp"

#include <vector>

#include "infrastructure/reports/shared/formatters/latex/tex_common_utils.hpp"
#include "infrastructure/reports/shared/formatters/latex/tex_utils.hpp"
#include "infrastructure/reports/shared/utils/format/report_string_utils.hpp"
#include "infrastructure/reports/shared/utils/format/time_format.hpp"

namespace DayTexUtils {

void DisplayHeader(std::string& report_stream, const DailyReportData& data,
                   const std::shared_ptr<DayTexConfig>& config) {
  // 1. 渲染标题
  std::string title_content =
      config->GetReportTitle() + " " + TexUtils::EscapeLatex(data.date);
  TexCommonUtils::RenderTitle(report_stream, title_content,
                              config->GetReportTitleFontSize());

  // 2. 准备列表数据
  // 处理多行备注：先转义，再添加 LaTeX 换行符 (\\)
  std::string safe_remark = TexUtils::EscapeLatex(data.metadata.remark);
  std::string formatted_remark = FormatMultilineForList(safe_remark, 0, "\\\\");

  std::vector<TexCommonUtils::SummaryItem> items = {
      {config->GetDateLabel(), TexUtils::EscapeLatex(data.date)},
      {config->GetTotalTimeLabel(),
       TexUtils::EscapeLatex(TimeFormatDuration(data.total_duration))},
      {config->GetStatusLabel(),
       TexUtils::EscapeLatex(BoolToString(data.metadata.status))},
      {config->GetSleepLabel(),
       TexUtils::EscapeLatex(BoolToString(data.metadata.sleep))},
      {config->GetExerciseLabel(),
       TexUtils::EscapeLatex(BoolToString(data.metadata.exercise))},
      {config->GetGetupTimeLabel(),
       TexUtils::EscapeLatex(data.metadata.getup_time)},
      {config->GetRemarkLabel(), formatted_remark}};

  // 3. 渲染列表
  TexCommonUtils::RenderSummaryList(report_stream, items,
                                    config->GetListTopSepPt(),
                                    config->GetListItemSepEx());
}

void DisplayDetailedActivities(std::string& report_stream,
                               const DailyReportData& data,
                               const std::shared_ptr<DayTexConfig>& config) {
  if (data.detailed_records.empty()) {
    return;
  }

  // [优化] 使用通用函数渲染子标题
  TexCommonUtils::RenderTitle(report_stream, config->GetAllActivitiesLabel(),
                              config->GetCategoryTitleFontSize(),
                              true  // is_subsection
  );

  std::string compact_list_options = TexCommonUtils::BuildCompactListOptions(
      config->GetListTopSepPt(), config->GetListItemSepEx());
  report_stream += "\\begin{itemize}";
  report_stream += compact_list_options;
  report_stream += "\n";

  for (const auto& record : data.detailed_records) {
    std::string project_path =
        ReplaceAll(record.project_path, "_", config->GetActivityConnector());
    std::string base_string =
        TexUtils::EscapeLatex(record.start_time) + " - " +
        TexUtils::EscapeLatex(record.end_time) + " (" +
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

    report_stream += "    \\item ";
    report_stream += colorized_string;
    report_stream += "\n";

    const auto& activity_remark_opt = record.activityRemark;
    if (activity_remark_opt.has_value()) {
      report_stream += "    \\begin{itemize}";
      report_stream += compact_list_options;
      report_stream += "\n";

      const std::string& activity_remark = activity_remark_opt.value();

      std::string safe_activity_remark = TexUtils::EscapeLatex(activity_remark);
      std::string formatted_activity_remark =
          FormatMultilineForList(safe_activity_remark, 0, "\\\\");

      report_stream += "        \\item \\textbf{";
      report_stream += config->GetActivityRemarkLabel();
      report_stream += "}: ";
      report_stream += formatted_activity_remark;
      report_stream += "\n";
      report_stream += "    \\end{itemize}\n";
    }
  }
  report_stream += "\\end{itemize}\n\n";
}

}  // namespace DayTexUtils
