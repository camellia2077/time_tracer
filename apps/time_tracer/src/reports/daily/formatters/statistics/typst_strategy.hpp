// reports/daily/formatters/statistics/typst_strategy.hpp
#include "i_stat_strategy.hpp"
#include "reports/daily/formatters/typst/day_typ_config.hpp"
#include <format>
#include <vector>

class TypstStrategy : public IStatStrategy {
public:
    explicit TypstStrategy(const std::shared_ptr<DayTypConfig>& config) : config_(config) {}

    std::string format_header(const std::string& title) const override {
        std::string header;
        header += std::format("#let statistic_font_size = {}pt\n", config_->get_statistic_font_size());
        header += std::format("#let statistic_title_font_size = {}pt\n", config_->get_statistic_title_font_size());
        header += "#set text(size: statistic_font_size)\n";
        // 注意必须是 = {0}而不是={0}，不然会没法正确渲染标题
        header += std::format("#text(size: statistic_title_font_size)[= {0}]\n\n", title);
        
        return header;
    }

    std::string format_main_item(const std::string& label, const std::string& value) const override {
        return std::format("- *{0}*: {1}", label, value);
    }

    std::string format_sub_item(const std::string& label, const std::string& value) const override {
        return std::format("  - *{0}*: {1}", label, value);
    }

    std::string build_output(const std::vector<std::string>& lines) const override {
        std::string result;
        for (const auto& line : lines) {
            result += line + "\n";
        }
        return result;
    }

private:
    std::shared_ptr<DayTypConfig> config_;
};