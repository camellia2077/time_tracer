// api/cli/impl/io/console_input.hpp
#ifndef CLI_IMPL_IO_CONSOLE_INPUT_H_
#define CLI_IMPL_IO_CONSOLE_INPUT_H_

#include <string>
#include <vector>

/**
 * @class ConsoleInput
 * @brief 负责处理 CLI 层面的用户输入交互。
 */
class ConsoleInput {
 public:
  /**
   * @brief 提示用户输入文件路径（支持多个，空格分隔）。
   * @details 逻辑迁移自 db_inserter/DataImporter.cpp 中的 UserInteractor。
   * @param message 提示信息。
   * @return 用户输入的路径列表。
   */
  static auto PromptForPaths(

      const std::string& message =
          "Enter .json file(s) or directory path(s) to process: ")
      -> std::vector<std::string>;
};

#endif  // CLI_IMPL_IO_CONSOLE_INPUT_H_
