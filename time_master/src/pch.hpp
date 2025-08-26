#ifndef PCH_HPP
#define PCH_HPP

// =================================================================
// 1. C++ 标准库 (Frequently Used Standard Library Headers)
// =================================================================
#include <algorithm>
#include <cctype>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>


// =================================================================
// 2. 第三方库 (Third-Party Library Headers)
// =================================================================
#include <sqlite3.h>
#include "nlohmann/json.hpp"


// =================================================================
// 3. 项目内部稳定且常用的核心头文件 (Stable & Common Project Headers)
// =================================================================
#include "common/common_utils.hpp"
#include "queries/shared/ReportFormat.hpp"
#include "queries/shared/data/query_data_structs.hpp"

// --- 核心数据结构 (Core Data Structures) ---
#include "common/AppConfig.hpp"
#include "queries/shared/data/DailyReportData.hpp"
#include "queries/shared/data/MonthlyReportData.hpp"
#include "queries/shared/data/PeriodReportData.hpp"

// --- 核心接口与工具 (Core Interfaces & Utilities) ---
#include "queries/shared/Interface/IReportFormatter.hpp" 
#include "queries/shared/Interface/ITreeFmt.hpp"
#include "queries/shared/factories/TreeFmtFactory.hpp"

#include "queries/shared/utils/query_utils.hpp"


#endif //PCH_HPP