// pch.hpp
#pragma once

// 标准库容器与算法
#include <algorithm>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

// 输入输出与文件系统
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>

// 时间与随机数
#include <chrono>
#include <random>

// 第三方库 (toml++ 编译较慢，放入 PCH 效果显著)
#include <toml++/toml.hpp>