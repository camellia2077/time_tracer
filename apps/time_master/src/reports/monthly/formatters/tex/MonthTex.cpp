// reports/monthly/formatters/tex/MonthTex.cpp
#include "MonthTex.hpp"
#include "MonthTexUtils.hpp"
#include "reports/shared/utils/format/TimeFormat.hpp"     
#include "reports/shared/formatters/latex/TexUtils.hpp"
#include "reports/shared/factories/GenericFormatterFactory.hpp"
#include "reports/monthly/formatters/tex/MonthTexConfig.hpp"
#include "reports/shared/data/MonthlyReportData.hpp"
#include "common/AppConfig.hpp" // [新增] 为导出函数引入 AppConfig
#include <sstream>
#include <memory>


MonthTex::MonthTex(std::shared_ptr<MonthTexConfig> config) : config_(config) {}

std::string MonthTex::format_report(const MonthlyReportData& data) const {
    if (data.year_month == "INVALID") {
        return config_->get_invalid_format_message() + "\n";
    }

    std::stringstream ss;
    ss << TexUtils::get_tex_preamble(
        config_->get_main_font(), 
        config_->get_cjk_main_font(),
        config_->get_base_font_size(),
        config_->get_margin_in()
    );

    MonthTexUtils::display_summary(ss, data, config_);

    if (data.actual_days == 0) {
        ss << config_->get_no_records_message() << "\n";
    } else {
        ss << TexUtils::format_project_tree(
            data.project_tree,
            data.total_duration,
            data.actual_days,
            config_->get_category_title_font_size(),
            config_->get_list_top_sep_pt(),
            config_->get_list_item_sep_ex()
        );
    }
    
    ss << TexUtils::get_tex_postfix();
    return ss.str();
}

// [新增] C-style functions to be exported from the DLL
extern "C" {
    __declspec(dllexport) FormatterHandle create_formatter(const AppConfig& cfg) {
        auto tex_config = std::make_shared<MonthTexConfig>(cfg.month_tex_config_path);
        auto formatter = new MonthTex(tex_config);
        return static_cast<FormatterHandle>(formatter);
    }

    __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
        if (handle) {
            delete static_cast<MonthTex*>(handle);
        }
    }

    static std::string report_buffer;

    __declspec(dllexport) const char* format_report(FormatterHandle handle, const MonthlyReportData& data) {
        if (handle) {
            auto* formatter = static_cast<MonthTex*>(handle);
            report_buffer = formatter->format_report(data);
            return report_buffer.c_str();
        }
        return "";
    }
}