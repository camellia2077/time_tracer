// db_inserter/service/ImportService.hpp
#ifndef IMPORT_SERVICE_HPP
#define IMPORT_SERVICE_HPP

#include <string>
#include <vector>
#include <map>
#include "db_inserter/model/ImportStats.hpp"
#include "reprocessing/converter/model/InputData.hpp" 

// 负责编排 "解析 -> 转换 -> 入库" 的具体业务逻辑
class ImportService {
public:
    explicit ImportService(const std::string& db_path);

    // 批量处理文件导入
    ImportStats import_from_files(const std::vector<std::string>& files);

    // 处理内存数据导入
    ImportStats import_from_memory(const std::map<std::string, std::vector<InputData>>& data_map);

private:
    std::string db_path_;
};

#endif // IMPORT_SERVICE_HPP