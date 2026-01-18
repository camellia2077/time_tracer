// cli/framework/io/console_input.cpp
#include "console_input.hpp"
#include <iostream>
#include <sstream>
#include <limits> // for numeric_limits
#include "common/ansi_colors.hpp"

std::vector<std::string> ConsoleInput::prompt_for_paths(const std::string& message) const {
    std::cout << message;
    
    std::string line;
    // 使用 getline 读取整行，支持空格分隔的多个路径
    if (std::cin.peek() == '\n') std::cin.ignore(); // 防止之前的残留换行符影响
    std::getline(std::cin, line);
    
    std::stringstream ss(line);
    std::string token;
    std::vector<std::string> user_inputs;
    while (ss >> token) {
        user_inputs.push_back(token);
    }
    return user_inputs;
}

bool ConsoleInput::prompt_confirmation(const std::string& message) const {
    std::cout << message << " (y/n): ";
    char confirmation;
    std::cin >> confirmation;

    // 清除输入缓冲区的剩余字符（包括换行符），防止影响后续的 getline 调用
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (confirmation == 'y' || confirmation == 'Y') {
        return true;
    }
    return false;
}