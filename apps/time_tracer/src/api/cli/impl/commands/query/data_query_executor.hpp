// api/cli/impl/commands/query/data_query_executor.hpp
#ifndef CLI_IMPL_COMMANDS_QUERY_DATA_QUERY_EXECUTOR_H_
#define CLI_IMPL_COMMANDS_QUERY_DATA_QUERY_EXECUTOR_H_

#include <filesystem>

#include "api/cli/framework/core/arg_definitions.hpp"

class DataQueryExecutor {
 public:
  explicit DataQueryExecutor(std::filesystem::path db_path);

  void Execute(const ParsedArgs& args);

 private:
  std::filesystem::path db_path_;
};

#endif  // CLI_IMPL_COMMANDS_QUERY_DATA_QUERY_EXECUTOR_H_
