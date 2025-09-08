#ifndef PCH_HPP
#define PCH_HPP

// =================================================================
// 1. C++ 标准库 (Frequently Used Standard Library Headers)
// =================================================================
#include <string>                             // 使用次数: 57
#include <vector>                             // 使用次数: 38
#include <iostream>                           // 使用次数: 26
#include <iomanip>                            // 使用次数: 20
#include <filesystem>                         // 使用次数: 17
#include <stdexcept>                          // 使用次数: 16
#include <algorithm>                          // 使用次数: 16
#include <sstream>                            // 使用次数: 16
#include <map>                                // 使用次数: 12
#include <fstream>                            // 使用次数: 11
#include <memory>                             // 使用次数: 8
#include <unordered_set>                      // 使用次数: 8
#include <optional>                           // 使用次数: 7
#include <set>                                // 使用次数: 6
#include <cctype>                             // 使用次数: 4
#include <chrono>                             // 使用次数: 4
#include <format>                             // 使用次数: 4 (C++23)
#include <print>                              // 使用次数: 3 (C++23)
#include <functional>                         // 使用次数: 3
#include <ctime>                              // 使用次数: 2
#include <utility>                            // 使用次数: 2


// =================================================================
// 2. 第三方库 (Third-Party Library Headers)
// =================================================================
#include <sqlite3.h>
#include "nlohmann/json.hpp"


// =================================================================
// 3. 项目内部稳定且常用的核心头文件 (Stable & Common Project Headers)
// =================================================================
#include "common/common_utils.hpp"            // 使用次数: 18
#include "queries/shared/ReportFormat.hpp"    // 使用次数: 16
#include "common/AnsiColors.hpp"              // 使用次数: 15

#endif //PCH_HPP