// core/pipeline/utils/processed_data_writer.hpp
#ifndef CORE_PIPELINE_UTILS_PROCESSED_DATA_WRITER_HPP_
#define CORE_PIPELINE_UTILS_PROCESSED_DATA_WRITER_HPP_

#include <filesystem>
#include <map>
#include <vector>
#include <string>
#include "common/model/daily_log.hpp"
#include <nlohmann/json.hpp> 

namespace core::pipeline {

class ProcessedDataWriter {
public:
    /**
     * @brief 将处理后的数据序列化为 JSON 并写入磁盘
     * @param data 按月分组的日志数据
     * @param cached_json_outputs [新增] 缓存的JSON对象，如果存在则直接使用
     * @param output_root 输出根目录
     * @return 成功写入的文件列表
     */
    static std::vector<std::filesystem::path> write(
        const std::map<std::string, std::vector<DailyLog>>& data,
        const std::map<std::string, nlohmann::json>& cached_json_outputs,
        const std::filesystem::path& output_root
    );
};

} // namespace core::pipeline

#endif