// application/pipeline/steps/logic_linker_step.hpp
#ifndef APPLICATION_PIPELINE_STEPS_LOGIC_LINKER_STEP_H_
#define APPLICATION_PIPELINE_STEPS_LOGIC_LINKER_STEP_H_

#include "application/pipeline/context/pipeline_context.hpp"

namespace core::pipeline {

/**
 * @class LogicLinkerStep
 * @brief 流水线步骤：调用 Converter 模块的 LogLinker 服务
 * * 职责：
 * 1. 在所有文件转换完成后执行。
 * 2. 修复因文件分割（如按月存储）导致的数据断层（如缺少跨月睡眠）。
 * 3. 此时数据尚在内存中，未生成 JSON 文件。
 */
class LogicLinkerStep {
 public:
  // 执行链接逻辑
  static auto Execute(PipelineContext& context) -> bool;
};

}  // namespace core::pipeline

#endif  // APPLICATION_PIPELINE_STEPS_LOGIC_LINKER_STEP_H_
