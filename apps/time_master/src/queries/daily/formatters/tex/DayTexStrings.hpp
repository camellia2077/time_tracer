// queries/daily/formatters/tex/DayTexStrings.hpp
#ifndef DAY_TEX_STRINGS_HPP
#define DAY_TEX_STRINGS_HPP

#include <string>
#include <map>

namespace DayTexStrings {

    /**
     * @brief [新增] 用于存储颜色名称和其十六进制值的结构体。
     */
    struct ColorInfo {
        std::string name; // 例如 "studycolor"
        std::string hex;  // 例如 "2ECC40"
    };

    /**
     * @brief [修改] 将关键字映射到一个包含颜色名称和十六进制代码的结构体。
     * 现在，颜色的十六进制值在这里统一定义。
     */
    const std::map<std::string, ColorInfo> KeywordColors = {
        {"study",      {"studycolor",      "2ECC40"}},
        {"recreation", {"recreationcolor", "FF4136"}},
        {"meal",       {"mealcolor",       "FF851B"}},
        {"exercise",   {"exercisecolor",   "0074D9"}},
        {"routine",    {"routinecolor",    "AAAAAA"}},
        {"sleep",      {"sleepcolor",      "B10DC9"}},
        {"code",       {"codecolor",       "39CCCC"}}
    };
}

#endif // DAY_TEX_STRINGS_HPP