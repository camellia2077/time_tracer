// api/cli/impl/io/console_input.cpp
#include "api/cli/impl/io/console_input.hpp"

#include <iostream>
#include <sstream>

auto ConsoleInput::PromptForPaths(const std::string& message)

    -> std::vector<std::string> {
  std::cout << message;

  std::string line;
  // 使用 getline 读取整行，支持空格分隔的多个路径
  if (std::cin.peek() == '\n') {
    std::cin.ignore();  // 防止之前的残留换行符影响
  }
  std::getline(std::cin, line);

  std::stringstream token_stream(line);
  std::string token;
  std::vector<std::string> user_inputs;
  while (token_stream >> token) {
    user_inputs.push_back(token);
  }
  return user_inputs;
}
