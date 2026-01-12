// converter/convert/facade/ConverterService.hpp
#ifndef CONVERTER_SERVICE_HPP
#define CONVERTER_SERVICE_HPP

#include "converter/config/ConverterConfig.hpp"

#include "common/model/DailyLog.hpp"
#include <string>
#include <istream>
#include <functional> 

/**
 * @brief 核心转换调度器 (Facade)
 * * [架构设计说明 / Architectural Note]
 * 此类仅实现核心业务逻辑：将原始文本流转化为结构化的 DailyLog 对象。
 * 遵循 "关注点分离 (Separation of Concerns)" 原则：
 * 1. 纯粹的计算单元：不处理任何文件读写 (File I/O) 操作。
 * 2. 流式输出：通过 data_consumer 回调将数据移交所有权。
 * 3. I/O 托管：具体的磁盘写入、文件管理由 Core 模块 (如 WorkflowHandler) 负责。
 */
class ConverterService {
public:
    explicit ConverterService(const ConverterConfig& config);

    // 执行转换逻辑
    // combined_input_stream: 数据源（不关心是文件流还是字符串流）
    // data_consumer: 数据去向（由 Core 层传入的回调，用于处理生成的数据，如写入磁盘或数据库）
    void executeConversion(std::istream& combined_input_stream, std::function<void(DailyLog&&)> data_consumer);

private:
    const ConverterConfig& config_;
};

#endif // CONVERTER_SERVICE_HPP