// importer/ImportService.hpp
#ifndef IMPORTER_IMPORT_SERVICE_HPP_
#define IMPORTER_IMPORT_SERVICE_HPP_

#include <string>
#include <vector>
#include <map>
#include <utility> // for std::pair
#include "importer/model/ImportStats.hpp"
#include "common/model/DailyLog.hpp" 

// 负责编排 "解析 -> 转换 -> 入库" 的具体业务逻辑
class ImportService {
public:
    explicit ImportService(const std::string& db_path);

    /**
     * @brief 批量处理 JSON 字符串数据导入。
     * @param inputs 输入数据列表，每个元素为 pair<标识符(如文件名), JSON内容>。
     */
    ImportStats import_json_data(const std::vector<std::pair<std::string, std::string>>& inputs);

    // 处理内存数据导入 (保持不变)
    ImportStats import_from_memory(const std::map<std::string, std::vector<DailyLog>>& data_map);

private:
    std::string db_path_;
};

#endif // IMPORTER_IMPORT_SERVICE_HPP_