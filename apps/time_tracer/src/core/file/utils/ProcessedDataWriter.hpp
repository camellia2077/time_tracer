// core/file/utils/ProcessedDataWriter.hpp
#ifndef CORE_FILE_UTILS_PROCESSED_DATA_WRITER_HPP_
#define CORE_FILE_UTILS_PROCESSED_DATA_WRITER_HPP_

#include <filesystem>
#include <map>
#include <vector>
#include <string>
#include "common/model/DailyLog.hpp"
// [移除] 序列化逻辑已解耦，不再需要传递 ConverterConfig
// #include "converter/config/ConverterConfig.hpp"

class ProcessedDataWriter {
public:
    /**
     * @brief 将处理后的数据序列化为 JSON 并写入磁盘
     * @param data 按月分组的日志数据
     * @param output_root 输出根目录
     * @return 成功写入的文件列表
     */
    static std::vector<std::filesystem::path> write(
        const std::map<std::string, std::vector<DailyLog>>& data,
        const std::filesystem::path& output_root
    );
};

#endif