// common/ansi_colors.hpp
#ifndef COMMON_ANSI_COLORS_H_
#define COMMON_ANSI_COLORS_H_

// =================================================================
//                 ANSI 终端颜色宏定义
// =================================================================
// 通过宏定义提供简单的颜色访问方式，与任何具体模块解耦。
// 任何需要颜色输出的模块都可以直接包含此文件。
// =================================================================

#define RED_COLOR "\033[1;31m"
#define GREEN_COLOR "\033[1;32m"
#define RESET_COLOR "\033[0m"

#endif  // COMMON_ANSI_COLORS_H_