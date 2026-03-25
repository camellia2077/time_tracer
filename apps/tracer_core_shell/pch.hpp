#pragma once
#ifndef PCH_H
#define PCH_H

// ===================================================================
//  1. C++ 标准库 (Standard Library)
//  最稳定、最庞大、使用最频繁的部分。
// ===================================================================
#include <algorithm>      // 使用次数: 34
#include <array>          // 使用次数: 12
#include <cctype>         // 使用次数: 15
#include <chrono>         // 使用次数: 15
#include <cstdint>        // 使用次数: 17
#include <exception>      // 使用次数: 20
#include <filesystem>     // 使用次数: 57
#include <format>         // 使用次数: 17 (C++23)
#include <fstream>        // 使用次数: 9
#include <iomanip>        // 使用次数: 9
#include <iostream>       // 使用次数: 29
#include <map>            // 使用次数: 48
#include <memory>         // 使用次数: 65
#include <mutex>          // 使用次数: 10
#include <optional>       // 使用次数: 79
#include <set>            // 使用次数: 14
#include <sstream>        // 使用次数: 18
#include <stdexcept>      // 使用次数: 67
#include <string>         // 使用次数: 243
#include <string_view>    // 使用次数: 88
#include <unordered_map>  // 使用次数: 9
#include <utility>        // 使用次数: 57
#include <vector>         // 使用次数: 115

// ===================================================================
//  2. 平台与第三方库 (Platform & Third-Party)
//  改动频率低，是 PCH 的理想候选。
// ===================================================================

// ===================================================================
//  3. 项目内部稳定且常用的核心头文件
//  建议仅包含极少修改的核心接口。
// ===================================================================
#include "cstddef"            // 使用次数: 10
#include "nlohmann/json.hpp"  // 使用次数: 14

#endif  // PCH_H
