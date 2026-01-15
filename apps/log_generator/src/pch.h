#pragma once

// 标准库容器与算法
#include <vector>
#include <string>
#include <map>
#include <optional>
#include <memory>
#include <algorithm>
#include <iterator>

// 输入输出与文件系统
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <format>

// 时间与随机数
#include <chrono>
#include <random>

// 第三方库 (toml++ 编译较慢，放入 PCH 效果显著)
#include <toml++/toml.hpp>