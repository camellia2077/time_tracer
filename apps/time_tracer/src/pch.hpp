// pch.hpp
#pragma once
#ifndef PCH_H
#define PCH_H

// ===================================================================
//  1. C++ 标准库 (Standard Library)
//  最稳定、最庞大、使用最频繁的部分。
// ===================================================================
#include <algorithm>      // 使用次数: 20
#include <array>          // 使用次数: 9
#include <cctype>         // 使用次数: 9
#include <chrono>         // 使用次数: 7
#include <cstdint>        // 使用次数: 8
#include <exception>      // 使用次数: 8
#include <filesystem>     // 使用次数: 32
#include <format>         // 使用次数: 12 (C++23)
#include <iomanip>        // 使用次数: 7
#include <iostream>       // 使用次数: 13
#include <map>            // 使用次数: 45
#include <memory>         // 使用次数: 60
#include <optional>       // 使用次数: 34
#include <set>            // 使用次数: 11
#include <sstream>        // 使用次数: 13
#include <stdexcept>      // 使用次数: 48
#include <string>         // 使用次数: 183
#include <string_view>    // 使用次数: 42
#include <unordered_map>  // 使用次数: 8
#include <utility>        // 使用次数: 38
#include <vector>         // 使用次数: 80

// ===================================================================
//  2. 平台与第三方库 (Platform & Third-Party)
//  改动频率低，是 PCH 的理想候选。
// ===================================================================
#include <sqlite3.h>      // 使用次数: 26
#include <toml++/toml.h>  // 使用次数: 7

// ===================================================================
//  3. 项目内部稳定且常用的核心头文件
//  建议仅包含极少修改的核心接口。
// ===================================================================
#include "application/ports/logger.hpp"                       // 使用次数: 9
#include "cstddef"                                            // 使用次数: 8
#include "domain/logic/validator/common/validator_utils.hpp"  // 使用次数: 8
#include "domain/model/daily_log.hpp"                         // 使用次数: 15
#include "domain/ports/diagnostics.hpp"                       // 使用次数: 18
#include "domain/reports/models/daily_report_data.hpp"        // 使用次数: 18
#include "domain/reports/models/period_report_models.hpp"     // 使用次数: 19
#include "domain/reports/models/project_tree.hpp"             // 使用次数: 9
#include "domain/reports/models/range_report_data.hpp"        // 使用次数: 10
#include "domain/reports/types/report_types.hpp"              // 使用次数: 17
#include "domain/types/converter_config.hpp"                  // 使用次数: 9
#include "domain/types/date_check_mode.hpp"                   // 使用次数: 7
#include "infrastructure/config/models/report_catalog.hpp"    // 使用次数: 12
#include "infrastructure/reports/data/cache/project_name_cache.hpp"  // 使用次数: 7
#include "infrastructure/reports/shared/api/shared_api.hpp"  // 使用次数: 17
#include "infrastructure/reports/shared/factories/generic_formatter_factory.hpp"  // 使用次数: 9
#include "infrastructure/reports/shared/formatters/latex/tex_utils.hpp"  // 使用次数: 9
#include "infrastructure/reports/shared/interfaces/formatter_abi_wrapper_template.hpp"  // 使用次数: 9
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"  // 使用次数: 38
#include "infrastructure/reports/shared/interfaces/formatter_c_string_view_utils.hpp"  // 使用次数: 9
#include "infrastructure/reports/shared/interfaces/i_report_formatter.hpp"  // 使用次数: 8
#include "infrastructure/reports/shared/utils/format/report_string_utils.hpp"  // 使用次 数: 10
#include "infrastructure/reports/shared/utils/format/time_format.hpp"  // 使用次数: 16
#include "infrastructure/schema/day_schema.hpp"     // 使用次数: 13
#include "infrastructure/schema/sqlite_schema.hpp"  // 使用次数: 13
#include "shared/types/ansi_colors.hpp"             // 使用次数: 9
#include "shared/utils/string_utils.hpp"            // 使用次数: 7

#endif  // PCH_H