#pragma once
#include <string>

namespace Utils {
    // 负责处理控制台颜色输出
    struct ConsoleColors {
        static const std::string red;
        static const std::string green;
        static const std::string reset;
    };

    // 设置控制台以支持UTF-8和颜色代码
    void setup_console();

    // 判断是否为闰年
    bool is_leap(int year);

    // 获取指定月份的天数
    int get_days_in_month(int year, int month);

    // 打印程序用法信息
    void print_usage(const char* prog_name);
    
    // 打印程序版本信息
    void print_version();

} // namespace Utils