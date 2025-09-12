#pragma once
#include <string>

namespace Utils {
    // [核心修改] 移除 ConsoleColors 结构体
    
    // 设置控制台以支持UTF-8和颜色代码
    void setup_console();

    // 判断是否为闰年
    bool is_leap(int year);

    // 获取指定月份的天数
    int get_days_in_month(int year, int month);
    
} // namespace Utils