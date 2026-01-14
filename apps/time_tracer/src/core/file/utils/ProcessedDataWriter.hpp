// core/file/utils/ProcessedDataWriter.hpp
#ifndef PROCESSED_DATA_WRITER_HPP
#define PROCESSED_DATA_WRITER_HPP

#include <filesystem>
#include <map>
#include <vector>
#include <string>
#include "common/model/DailyLog.hpp"
#include "converter/config/ConverterConfig.hpp"

class ProcessedDataWriter {
public:
    /**
     * @brief 将处理后的数据写入磁盘
     * @return 成功写入的文件列表
     */
    static std::vector<std::filesystem::path> write(
        const std::map<std::string, std::vector<DailyLog>>& data,
        const std::filesystem::path& output_root,
        const ConverterConfig& config
    );
};

#endif