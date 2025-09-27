// reports/daily/formatters/statistics/TypstStrategy.hpp
#ifndef TYPST_STRATEGY_HPP
#define TYPST_STRATEGY_HPP

#include "IStatStrategy.hpp"
#include "reports/daily/formatters/typ/DayTypConfig.hpp"
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
        header += std::format("#align(center)[#text(size: statistic_title_font_size)[={0}]]\n\n", title);
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

#endif // TYPST_STRATEGY_HPP