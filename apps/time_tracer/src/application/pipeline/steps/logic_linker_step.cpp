// application/pipeline/steps/logic_linker_step.cpp
#include "logic_linker_step.hpp"

#include <iostream>

#include "common/ansi_colors.hpp"
#include "converter/convert/core/log_linker.hpp"  // 引用 Converter 服务

namespace core::pipeline {

auto LogicLinkerStep::execute(PipelineContext& context) -> bool {
  // 如果没有转换数据，直接跳过
  if (context.result.processed_data.empty()) {
    return true;
  }

  std::cout << "Step: Linking cross-month data..." << std::endl;

  try {
    // 1. 初始化 Linker 服务 (注入配置)
    LogLinker linker(context.state.converter_config);

    // 2. 执行链接 (原地修改 processed_data)
    linker.link_logs(context.result.processed_data);

  } catch (const std::exception& e) {
    std::cerr << RED_COLOR << "[Pipeline] Logic Linker Error: " << e.what()
              << RESET_COLOR << std::endl;
    // 链接失败通常不致命（只是缺睡眠数据），可以选择返回 true 继续，或者 false
    // 终止 这里选择从宽处理，确保后续步骤能继续输出已有的数据
    return true;
  }

  return true;
}

}  // namespace core::pipeline
