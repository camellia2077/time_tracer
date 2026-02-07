#pragma once
#ifndef PCH_H
#define PCH_H

// ===================================================================
//  1. C++ 标准库 (Standard Library)
//  最稳定、最庞大、使用最频繁的部分。
// ===================================================================
#include <algorithm>    // 使用次数: 21
#include <cctype>       // 使用次数: 6
#include <filesystem>   // 使用次数: 39
#include <format>       // 使用次数: 17 (C++23)
#include <functional>   // 使用次数: 6
#include <iomanip>      // 使用次数: 16
#include <iostream>     // 使用次数: 71
#include <map>          // 使用次数: 41
#include <memory>       // 使用次数: 46
#include <optional>     // 使用次数: 17
#include <set>          // 使用次数: 17
#include <sstream>      // 使用次数: 31
#include <stdexcept>    // 使用次数: 47
#include <string>       // 使用次数: 147
#include <string_view>  // 使用次数: 15
#include <utility>      // 使用次数: 19
#include <vector>       // 使用次数: 87

// ===================================================================
//  2. 平台与第三方库 (Platform & Third-Party)
//  改动频率低，是 PCH 的理想候选。
// ===================================================================
#include <sqlite3.h>      // 使用次数: 29
#include <toml++/toml.h>  // 使用次数: 36

// ===================================================================
//  3. 项目内部稳定且常用的核心头文件
//  建议仅包含极少修改的核心接口。
// ===================================================================
#include "api/cli/framework/core/arg_definitions.hpp"         // 使用次数: 7
#include "api/cli/framework/core/command_parser.hpp"          // 使用次数: 12
#include "api/cli/framework/core/command_registry.hpp"        // 使用次数: 10
#include "api/cli/framework/core/command_validator.hpp"       // 使用次数: 9
#include "api/cli/framework/interfaces/i_command.hpp"         // 使用次数: 12
#include "api/cli/impl/app/app_context.hpp"                   // 使用次数: 10
#include "domain/logic/validator/common/validator_utils.hpp"  // 使用次数: 11
#include "domain/model/daily_log.hpp"                         // 使用次数: 19
#include "domain/reports/models/daily_report_data.hpp"        // 使用次数: 11
#include "domain/reports/models/monthly_report_data.hpp"      // 使用次数: 9
#include "domain/reports/models/project_tree.hpp"             // 使用次数: 7
#include "domain/reports/models/query_data_structs.hpp"       // 使用次数: 7
#include "domain/reports/models/range_report_data.hpp"        // 使用次数: 10
#include "domain/reports/types/report_format.hpp"             // 使用次数: 14
#include "domain/types/app_options.hpp"                       // 使用次数: 7
#include "infrastructure/config/models/app_config.hpp"        // 使用次数: 20
#include "infrastructure/config/models/converter_config_models.hpp"  // 使用次数: 13
#include "infrastructure/config/validator/reports/strategies/base_strategy.hpp"  // 使用次数: 7
#include "infrastructure/io/core/file_system_helper.hpp"     // 使用次数: 9
#include "infrastructure/persistence/sqlite/db_manager.hpp"  // 使用次数: 6
#include "infrastructure/reports/data/cache/project_name_cache.hpp"  // 使用次数: 10
#include "infrastructure/reports/data/utils/project_tree_builder.hpp"  // 使用次数: 8
#include "infrastructure/reports/shared/api/shared_api.hpp"  // 使用次数: 18
#include "infrastructure/reports/shared/factories/generic_formatter_factory.hpp"  // 使用 次数: 7
#include "infrastructure/reports/shared/utils/format/iso_week_utils.hpp"  // 使用次数: 7
#include "infrastructure/reports/shared/utils/format/report_string_utils.hpp"  // 使用次数: 7
#include "infrastructure/reports/shared/utils/format/time_format.hpp"  // 使用次数: 15
#include "infrastructure/reports/shared/utils/format/year_utils.hpp"  // 使用次数: 7
#include "nlohmann/json.hpp"              // 使用次数: 14
#include "shared/types/ansi_colors.hpp"   // 使用次数: 26
#include "shared/utils/string_utils.hpp"  // 使用次数: 9

#endif  // PCH_H