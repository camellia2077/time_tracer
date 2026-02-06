// pch.hpp
#pragma once
#ifndef PCH_H_
#define PCH_H_

// ===================================================================
//  1. C++ 标准库 (Standard Library)
//  最稳定、最庞大、使用最频繁的部分。
// ===================================================================
#include <algorithm>      // 使用次数: 18
#include <filesystem>     // 使用次数: 32
#include <format>         // 使用次数: 17 (C++23)
#include <functional>     // 使用次数: 6
#include <iomanip>        // 使用次数: 16
#include <iostream>       // 使用次数: 66
#include <map>            // 使用次数: 38
#include <memory>         // 使用次数: 42
#include <optional>       // 使用次数: 10
#include <set>            // 使用次数: 16
#include <sstream>        // 使用次数: 31
#include <stdexcept>      // 使用次数: 43
#include <string>         // 使用次数: 126
#include <string_view>    // 使用次数: 8
#include <unordered_map>  // 使用次数: 5
#include <utility>        // 使用次数: 15
#include <vector>         // 使用次数: 75

// ===================================================================
//  2. 平台与第三方库 (Platform & Third-Party)
//  改动频率低，是 PCH 的理想候选。
// ===================================================================
#include <sqlite3.h>      // 使用次数: 25
#include <toml++/toml.h>  // 使用次数: 36

// ===================================================================
//  3. 项目内部稳定且常用的核心头文件
//  建议仅包含极少修改的核心接口。
// ===================================================================
#include "application/pipeline/context/pipeline_context.hpp"  // 使用次数: 6
#include "cli/framework/core/command_parser.hpp"              // 使用次数: 10
#include "cli/framework/core/command_registry.hpp"            // 使用次数: 8
#include "cli/framework/core/command_validator.hpp"           // 使用次数: 7
#include "cli/framework/interfaces/i_command.hpp"             // 使用次数: 11
#include "cli/impl/app/app_context.hpp"                       // 使用次数: 8
#include "common/ansi_colors.hpp"                             // 使用次数: 25
#include "common/app_options.hpp"                             // 使用次数: 7
#include "common/config/app_config.hpp"                       // 使用次数: 21
#include "common/config/models/converter_config_models.hpp"   // 使用次数: 13
#include "common/utils/string_utils.hpp"                      // 使用次数: 7
#include "config/validator/reports/strategies/base_strategy.hpp"  // 使用次数: 6
#include "domain/model/daily_log.hpp"                     // 使用次数: 17
#include "importer/model/time_sheet_data.hpp"             // 使用次数: 5
#include "infrastructure/io/core/file_system_helper.hpp"  // 使用次数: 8
#include "nlohmann/json.hpp"                              // 使用次数: 12
#include "reports/data/cache/project_name_cache.hpp"      // 使用次数: 14
#include "reports/data/model/daily_report_data.hpp"       // 使用次数: 10
#include "reports/data/model/monthly_report_data.hpp"     // 使用次数: 8
#include "reports/data/model/project_tree.hpp"            // 使用次数: 7
#include "reports/data/model/query_data_structs.hpp"      // 使用次数: 7
#include "reports/data/model/range_report_data.hpp"       // 使用次数: 10
#include "reports/data/utils/project_tree_builder.hpp"    // 使用次数: 14
#include "reports/shared/api/shared_api.hpp"              // 使用次数: 18
#include "reports/shared/factories/generic_formatter_factory.hpp"  // 使用次数: 6
#include "reports/shared/types/report_format.hpp"               // 使用次数: 13
#include "reports/shared/utils/format/iso_week_utils.hpp"       // 使用次数: 6
#include "reports/shared/utils/format/report_string_utils.hpp"  // 使用次数: 6
#include "reports/shared/utils/format/time_format.hpp"          // 使用次数: 14
#include "reports/shared/utils/format/year_utils.hpp"           // 使用次数: 6
#include "validator/common/validator_utils.hpp"                 // 使用次数: 10

#endif  // PCH_H_