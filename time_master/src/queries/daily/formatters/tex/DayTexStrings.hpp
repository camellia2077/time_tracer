// queries/daily/formatters/tex/DayTexStrings.hpp
#ifndef DAY_TEX_STRINGS_HPP
#define DAY_TEX_STRINGS_HPP

#include <string>
#include <map>

namespace DayTexStrings {
    // Defines the mapping from keywords to the LaTeX color names
    // defined in BaseTexFormatter.hpp
    const std::map<std::string, std::string> KeywordColors = {
        {"study",      "studycolor"},
        {"recreation", "recreationcolor"},
        {"meal",       "mealcolor"},
        {"exercise",   "exercisecolor"},
        {"routine",    "routinecolor"},
        {"sleep",      "sleepcolor"},
        {"code",       "codecolor"}
    };
}

#endif // DAY_TEX_STRINGS_HPP