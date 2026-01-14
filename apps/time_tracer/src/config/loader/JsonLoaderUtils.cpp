// config/loader/JsonLoaderUtils.cpp
#include "JsonLoaderUtils.hpp"
#include "io/core/FileReader.hpp"
#include <iostream>
#include <stdexcept>

namespace JsonLoaderUtils {

    json read_json(const fs::path& path) {
        try {
            std::string content = FileReader::read_content(path);
            return json::parse(content);
        } catch (const std::exception& e) {
            throw std::runtime_error("Config Load Error [" + path.string() + "]: " + e.what());
        }
    }

    void parse_statistics_items(const json& j_items, std::vector<ReportStatisticsItem>& out_items) {
        if (!j_items.is_array()) return;

        for (const auto& j_item : j_items) {
            ReportStatisticsItem item;
            item.label = get_required<std::string>(j_item, "label");
            item.show = get_optional<bool>(j_item, "show", true);
            item.db_column = get_optional<std::string>(j_item, "db_column", "");

            if (j_item.contains("sub_items")) {
                parse_statistics_items(j_item["sub_items"], item.sub_items);
            }
            out_items.push_back(item);
        }
    }

    void fill_tex_style(const json& j, FontConfig& fonts, LayoutConfig& layout) {
        fonts.main_font = get_required<std::string>(j, "main_font");
        fonts.cjk_main_font = get_optional<std::string>(j, "cjk_main_font", fonts.main_font);
        fonts.base_font_size = get_required<int>(j, "base_font_size");
        fonts.report_title_font_size = get_required<int>(j, "report_title_font_size");
        fonts.category_title_font_size = get_required<int>(j, "category_title_font_size");
        
        layout.margin_in = get_optional<double>(j, "margin_in", 1.0);
        layout.list_top_sep_pt = get_optional<double>(j, "list_top_sep_pt", 0.0);
        layout.list_item_sep_ex = get_optional<double>(j, "list_item_sep_ex", 0.0);
    }

    void fill_typ_style(const json& j, FontConfig& fonts, LayoutConfig& layout) {
        fonts.base_font = get_required<std::string>(j, "base_font");
        fonts.title_font = get_optional<std::string>(j, "title_font", fonts.base_font);
        fonts.category_title_font = get_optional<std::string>(j, "category_title_font", fonts.base_font);
        fonts.base_font_size = get_required<int>(j, "base_font_size");
        fonts.report_title_font_size = get_required<int>(j, "report_title_font_size");
        fonts.category_title_font_size = get_required<int>(j, "category_title_font_size");

        layout.line_spacing_em = get_optional<double>(j, "line_spacing_em", 0.65);
        layout.margin_top_cm = get_optional<double>(j, "margin_top_cm", 2.5);
        layout.margin_bottom_cm = get_optional<double>(j, "margin_bottom_cm", 2.5);
        layout.margin_left_cm = get_optional<double>(j, "margin_left_cm", 2.0);
        layout.margin_right_cm = get_optional<double>(j, "margin_right_cm", 2.0);
    }

    void fill_daily_labels(const json& j, DailyReportLabels& labels) {
        // 兼容不同配置文件的命名 (report_title vs title_prefix)
        if (j.contains("title_prefix")) {
            labels.report_title_prefix = j["title_prefix"].get<std::string>();
        } else {
            labels.report_title_prefix = get_optional<std::string>(j, "report_title", "Daily Report for");
        }

        labels.date_label = get_required<std::string>(j, "date_label");
        labels.total_time_label = get_required<std::string>(j, "total_time_label");
        labels.status_label = get_optional<std::string>(j, "status_label", "Status");
        labels.sleep_label = get_optional<std::string>(j, "sleep_label", "Sleep");
        labels.exercise_label = get_optional<std::string>(j, "exercise_label", "Exercise");
        labels.getup_time_label = get_optional<std::string>(j, "getup_time_label", "Getup Time");
        labels.remark_label = get_optional<std::string>(j, "remark_label", "Remark");
        labels.no_records_message = get_optional<std::string>(j, "no_records_message", "No records.");
        
        labels.statistics_label = get_optional<std::string>(j, "statistics_label", "Statistics");
        labels.all_activities_label = get_optional<std::string>(j, "all_activities_label", "All Activities");
        labels.activity_remark_label = get_optional<std::string>(j, "activity_remark_label", "Remark");
        labels.project_breakdown_label = get_optional<std::string>(j, "project_breakdown_label", "Project Breakdown");
        labels.activity_connector = get_optional<std::string>(j, "activity_connector", "->");
    }

    void fill_monthly_labels(const json& j, MonthlyReportLabels& labels) {
        labels.report_title = get_required<std::string>(j, "report_title");
        labels.total_time_label = get_required<std::string>(j, "total_time_label");
        labels.actual_days_label = get_required<std::string>(j, "actual_days_label");
        labels.no_records_message = get_required<std::string>(j, "no_records_message");
    }

    void fill_period_labels(const json& j, PeriodReportLabels& labels) {
        labels.report_title_prefix = get_required<std::string>(j, "report_title_prefix");
        labels.report_title_days = get_required<std::string>(j, "report_title_days");
        labels.report_title_date_separator = get_optional<std::string>(j, "report_title_date_separator", "to");
        labels.total_time_label = get_required<std::string>(j, "total_time_label");
        labels.actual_days_label = get_required<std::string>(j, "actual_days_label");
        labels.no_records_message = get_required<std::string>(j, "no_records_message");
        labels.invalid_days_message = get_optional<std::string>(j, "invalid_days_message", "Invalid days.");
    }

    void fill_keyword_colors(const json& j, std::map<std::string, std::string>& colors) {
        if (j.contains("keyword_colors")) {
            for (const auto& [key, val] : j["keyword_colors"].items()) {
                colors[key] = val.get<std::string>();
            }
        }
    }
}