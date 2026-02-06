// infrastructure/io/processed_data_writer.hpp
#ifndef INFRASTRUCTURE_IO_PROCESSED_DATA_WRITER_H_
#define INFRASTRUCTURE_IO_PROCESSED_DATA_WRITER_H_

#include <filesystem>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "domain/model/daily_log.hpp"

namespace infrastructure::io {

class ProcessedDataWriter {
 public:
  /**
   * @brief 将处理后的数据序列化为 JSON 并写入磁盘
   * @param data 按月分组的日志数据
   * @param cached_json_outputs [新增] 缓存的JSON对象，如果存在则直接使用
   * @param output_root 输出根目录
   * @return 成功写入的文件列表
   */
  static auto Write(

      const std::map<std::string, std::vector<DailyLog>>& data,
      const std::map<std::string, nlohmann::json>& cached_json_outputs,
      const std::filesystem::path& output_root)
      -> std::vector<std::filesystem::path>;
};

}  // namespace infrastructure::io

#endif  // INFRASTRUCTURE_IO_PROCESSED_DATA_WRITER_H_
