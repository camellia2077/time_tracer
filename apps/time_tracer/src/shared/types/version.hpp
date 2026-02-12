// shared/types/version.hpp
#ifndef COMMON_VERSION_H_
#define COMMON_VERSION_H_

#include <string_view>

// 使用命名空间来组织应用配置信息，避免全局污染
namespace AppInfo {
// 使用 constexpr 和 string_view 可以在编译期确定字符串，效率更高
// 这也是现代C++的推荐做法
constexpr std::string_view kVersion = "0.5.8";
constexpr std::string_view kLastUpdated = "2026-02-12";

}  // namespace AppInfo

#endif  // COMMON_VERSION_H_