// converter/convert/io/JsonWriter.hpp
#ifndef JSON_WRITER_HPP
#define JSON_WRITER_HPP

#include <ostream>
#include <vector> 
#include <nlohmann/json.hpp>
#include "common/model/DailyLog.hpp"
#include "converter/config/ConverterConfig.hpp"


// 用于管理json的键的命名和输出
/**
 * @brief JSON 格式化生成器
 * * [职责说明]
 * 此类负责将 DailyLog 内存对象序列化为 JSON 格式的流。
 * 它不管理文件句柄或文件路径，只向传入的 std::ostream 写入内容。
 */
class JsonWriter {
public:
    // [Legacy] 保留旧接口以防兼容性问题，但建议废弃
    void write(std::ostream& outputStream, const std::vector<DailyLog>& days, const ConverterConfig& config);

    // --- [核心修改] 新增流式输出接口 ---
    
    // 写入 JSON 数组开始符号 "["
    void beginJsonWriter(std::ostream& outputStream);

    // 写入单天数据 (会自动处理逗号分隔)
    // isFirstDay: 如果是第一条数据，不写前面的逗号
    void writeDay(std::ostream& outputStream, const DailyLog& day, bool isFirstDay);

    // 写入 JSON 数组结束符号 "]"
    void endJsonWriter(std::ostream& outputStream);

private:
    // 内部辅助函数，将单天数据转换为 JSON 对象
    nlohmann::json convertDayToJson(const DailyLog& day);
};

#endif // JSON_WRITER_HPP