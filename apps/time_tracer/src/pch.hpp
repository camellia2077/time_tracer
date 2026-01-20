// pch.hpp
#ifndef PCH_HPP_
#define PCH_HPP_

// =================================================================
// 1. C++ 标准库 (Frequently Used Standard Library Headers)
// =================================================================
#include <string>                             // 使用次数: 115
#include <vector>                             // 使用次数: 68
#include <iostream>                           // 使用次数: 64
#include <stdexcept>                          // 使用次数: 41
#include <memory>                             // 使用次数: 40
#include <filesystem>                         // 使用次数: 30
#include <map>                                // 使用次数: 30
#include <sstream>                            // 使用次数: 27
#include <format>                             // 使用次数: 19 (C++23)
#include <iomanip>                            // 使用次数: 16
#include <algorithm>                          // 使用次数: 16
#include <set>                                // 使用次数: 15
#include <optional>                           // 使用次数: 9
#include <fstream>                            // 使用次数: 7
#include <functional>                         // 使用次数: 7
#include <unordered_set>                      // 使用次数: 6
#include <chrono>                             // 使用次数: 6
#include <utility>                            // 使用次数: 5
#include <unordered_map>                      // 使用次数: 5


// =================================================================
// 2. 第三方库 (Third-Party Library Headers)
// =================================================================
#include <sqlite3.h>                          // 使用次数: 21
#include "nlohmann/json.hpp"                  // 使用次数: 12
#include "toml++/toml.h"                      // 使用次数: 37

// ===================================================================
//  3. 项目内部稳定且常用的核心头文件 (Stable & Common Project Headers)
//  建议仅包含极少修改的核心接口，频繁变动的头文件不应放入 PCH。
// ===================================================================
#include "cli/framework/interfaces/i_command.hpp" // 使用次数: 12
#include "cli/framework/core/command_parser.hpp" // 使用次数: 11
#include "cli/framework/core/command_registry.hpp" // 使用次数: 9
#include "cli/impl/app/app_context.hpp"       // 使用次数: 9
#include "cli/framework/core/command_validator.hpp" // 使用次数: 8

#include "common/ansi_colors.hpp"             // 使用次数: 27
#include "common/config/app_config.hpp"       // 使用次数: 20
#include "common/model/daily_log.hpp"         // 使用次数: 18
#include "common/config/models/converter_config_models.hpp" // 使用次数: 14
#include "common/app_options.hpp"             // 使用次数: 8
#include "common/utils/string_utils.hpp"      // 使用次数: 8

#include "config/validator/reports/strategies/base_strategy.hpp" // 使用次数: 6

#include "core/pipeline/context/pipeline_context.hpp" // 使用次数: 7

#include "importer/model/time_sheet_data.hpp" // 使用次数: 6

#include "io/core/file_system_helper.hpp"     // 使用次数: 9

#include "reports/shared/api/shared_api.hpp"  // 使用次数: 17
#include "reports/shared/utils/format/time_format.hpp" // 使用次数: 15
#include "reports/shared/types/report_format.hpp" // 使用次数: 14
#include "reports/data/model/daily_report_data.hpp" // 使用次数: 11
#include "reports/data/cache/project_name_cache.hpp" // 使用次数: 9
#include "reports/data/model/monthly_report_data.hpp" // 使用次数: 9
#include "reports/data/model/period_report_data.hpp" // 使用次数: 9
#include "reports/data/model/project_tree.hpp" // 使用次数: 9
#include "reports/data/utils/project_tree_builder.hpp" // 使用次数: 9
#include "reports/data/model/query_data_structs.hpp" // 使用次数: 7
#include "reports/shared/factories/generic_formatter_factory.hpp" // 使用次数: 6
#include "reports/shared/formatters/latex/tex_utils.hpp" // 使用次数: 6
#include "reports/shared/interfaces/i_report_formatter.hpp" // 使用次数: 6

#include "validator/common/ValidatorUtils.hpp" // 使用次数: 11

#endif //PCH_HPP_