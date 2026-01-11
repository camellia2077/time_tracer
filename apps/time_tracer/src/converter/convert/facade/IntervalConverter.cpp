// converter/convert/facade/IntervalConverter.cpp
#include "IntervalConverter.hpp"
#include "common/AnsiColors.hpp"
#include "converter/convert/pipelines/InputParser.hpp"
#include "converter/convert/pipelines/DayProcessor.hpp"

// [核心修改] 构造函数
IntervalConverter::IntervalConverter(const ConverterConfig& config) 
    : config_(config) {}

std::vector<InputData> IntervalConverter::executeConversion(std::istream& combined_input_stream) {
    InputParser parser(config_);
    std::vector<InputData> all_days;

    parser.parse(combined_input_stream, [&](InputData& day) {
        all_days.push_back(day);
    });

    if (all_days.empty()) {
        return {};
    }

    DayProcessor processor(config_);
    InputData empty_prev_day;

    for (size_t i = 0; i < all_days.size(); ++i) {
        InputData& previous_day = (i > 0) ? all_days[i - 1] : empty_prev_day;
        InputData& current_day = all_days[i];
        processor.process(previous_day, current_day);
    }
    
    // 移除跨年的重复月份逻辑 (保持原样)
    if (all_days.size() > 1) {
        const auto& first_day = all_days[0];
        const auto& second_day = all_days[1];

        if (first_day.date.length() == 10 && second_day.date.length() == 10) {
            std::string first_month = first_day.date.substr(5, 2);
            std::string second_month = second_day.date.substr(5, 2);

            if (first_month == "12" && second_month == "01") {
                int first_year = std::stoi(first_day.date.substr(0, 4));
                int second_year = std::stoi(second_day.date.substr(0, 4));

                if (second_year == first_year + 1) {
                    all_days.erase(all_days.begin());
                }
            }
        }
    }
    
    return all_days;
}