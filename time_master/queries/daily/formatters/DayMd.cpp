#include "DayMd.h"
#include <iomanip>

#include "common/common_utils.h"                      // 为了 ProjectTree

// --- 核心改动：包含新的依赖 ---
#include "queries/shared/query_utils.h"   // 用于 build_project_tree_from_records 和 get_parent_map
#include "queries/shared/breakdown/TreeFmtFactory.h" // 新的工厂
#include "queries/shared/breakdown/ITreeFmt.h"     // 工厂返回的接口
#include "queries/shared/DailyReportData.h"


// format_report 方法保持不变，因为它将工作委托给了辅助函数
std::string DayMd::format_report(const DailyReportData& data, sqlite3* db) const {
    std::stringstream ss;
    _display_header(ss, data);

    if (data.total_duration == 0) {
        ss << "No time records for this day.\n";
        return ss.str();
    }
    
    // 调用更新后的 _display_project_breakdown
    _display_project_breakdown(ss, data, db);
    return ss.str();
}

// _display_header 方法保持不变
void DayMd::_display_header(std::stringstream& ss, const DailyReportData& data) const {
    ss << "## Daily Report for " << data.date << "\n\n"; 
    ss << "- **Date**: " << data.date << "\n";
    ss << "- **Total Time Recorded**: " << time_format_duration(data.total_duration) << "\n";
    ss << "- **Status**: " << data.metadata.status << "\n";
    ss << "- **Getup Time**: " << data.metadata.getup_time << "\n";
    ss << "- **Remark**:" << data.metadata.remark << "\n";
}

// --- 核心改动：_display_project_breakdown 的全新实现 ---
void DayMd::_display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const {
    // 1. 准备数据：构建项目树
    // 注意：这些函数现在是来自 query_utils.cpp 的独立工具函数
    std::map<std::string, std::string> parent_map = get_parent_map(db);
    ProjectTree project_tree;
    build_project_tree_from_records(project_tree, data.records, parent_map);

    // 2. 使用工厂创建格式化器
    // 由于这是 Markdown 格式化器，我们硬编码请求 Markdown 格式
    auto formatter = TreeFmtFactory::createFormatter(ReportFormat::Markdown);

    // 3. 调用格式化器并获取结果
    if (formatter) {
        // 调用新接口的 format 方法
        std::string breakdown_output = formatter->format(project_tree, data.total_duration, 1);
        ss << breakdown_output;
    }
}