#ifndef PCH_H
#define PCH_H

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
#include "common/common_utils.h"
#include "queries/shared/ReportFormat.h"
#include "queries/shared/query_data_structs.h"

// --- 核心数据结构 (Core Data Structures) ---
#include "queries/shared/DailyReportData.h"
#include "queries/shared/MonthlyReportData.h"
#include "queries/shared/PeriodReportData.h"

// --- 核心接口与工具 (Core Interfaces & Utilities) ---
#include "queries/shared/Interface/IReportFormatter.h" 
// #include "queries/shared/query_utils.h"
// #include "queries/shared/breakdown/ITreeFmt.h"
// #include "queries/shared/breakdown/TreeFmtFactory.h"
// #include "queries/shared/BaseTexFormatter.h"

#endif //PCH_H