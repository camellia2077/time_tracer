// converter/convert/facade/IntervalConverter.cpp
#include "IntervalConverter.hpp"
#include "common/AnsiColors.hpp"
#include "converter/convert/pipelines/InputParser.hpp"
#include "converter/convert/pipelines/DayProcessor.hpp"

IntervalConverter::IntervalConverter(const ConverterConfig& config) 
    : config_(config) {}

void IntervalConverter::executeConversion(std::istream& combined_input_stream, std::function<void(InputData&&)> data_consumer) {
    InputParser parser(config_);
    DayProcessor processor(config_);
    
    // 滑动窗口：只在内存中保留前一天
    InputData previous_day;
    bool has_previous = false;

    parser.parse(combined_input_stream, [&](InputData& current_day) {
        // [流式流水线逻辑]
        
        // 1. 初始化 / 跨天逻辑处理
        if (!has_previous) {
             InputData empty_day; 
             processor.process(empty_day, current_day);
        } else {
             // 利用 current_day 完善 previous_day 的睡眠逻辑
             processor.process(previous_day, current_day);
             
             // 2. 去重逻辑 (例如跨年重复的 12-31 和 01-01)
             bool skip_previous = false;
             if (previous_day.date.length() == 10 && current_day.date.length() == 10) {
                 std::string prev_month = previous_day.date.substr(5, 2);
                 std::string curr_month = current_day.date.substr(5, 2);
                 if (prev_month == "12" && curr_month == "01") {
                      int prev_year = std::stoi(previous_day.date.substr(0, 4));
                      int curr_year = std::stoi(current_day.date.substr(0, 4));
                      if (curr_year == prev_year + 1) {
                          skip_previous = true; 
                      }
                 }
             }

             // 3. [关键点] 将 previous_day 移交给回调函数
             if (!skip_previous) {
                 if (data_consumer) {
                     data_consumer(std::move(previous_day));
                 }
             }
        }

        // 4. 滑动窗口：current_day 变为 previous_day
        // 使用 std::move 避免拷贝
        previous_day = std::move(current_day);
        has_previous = true;
    });

    // 5. 处理最后一天 (Flush)
    if (has_previous) {
        InputData empty_next_day;
        processor.process(previous_day, empty_next_day);
        if (data_consumer) {
            data_consumer(std::move(previous_day));
        }
    }
}