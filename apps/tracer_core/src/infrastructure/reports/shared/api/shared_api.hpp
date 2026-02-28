// infrastructure/reports/shared/api/shared_api.hpp
#ifndef INFRASTRUCTURE_REPORTS_SHARED_API_SHARED_API_H_
#define INFRASTRUCTURE_REPORTS_SHARED_API_SHARED_API_H_

// ----------------------------------------------------------------------
// 定义导入/导出宏 (适用于 Windows/MSVC 和 MinGW)
// ----------------------------------------------------------------------
#ifdef _WIN32
#ifdef REPORTS_SHARED_EXPORTS
// 正在构建 reports_shared.dll 本身，我们需要导出 (export)
#define REPORTS_SHARED_API /* NOLINT(readability-identifier-naming) */ \
  __declspec(dllexport)
#else
// 正在构建 *依赖* reports_shared.dll 的代码 (例如插件DLL)，
// 我们需要导入 (import)
#define REPORTS_SHARED_API /* NOLINT(readability-identifier-naming) */ \
  __declspec(dllimport)
#endif
#else
// 对于非 Windows 平台 (Linux, macOS), 默认设为空
// (GCC/Clang 使用 -fvisibility=hidden 属性, 但这里简化处理)
#define REPORTS_SHARED_API  // NOLINT(readability-identifier-naming)
#endif

// ----------------------------------------------------------------------
// [可选] 抑制C4251警告：用于导出使用了STL模板的类
// ----------------------------------------------------------------------
// 当您导出的类中包含 std::string, std::vector, std::map, std::shared_ptr 等
// STL 模板作为成员变量时，MSVC 编译器会发出 C4251 警告。
// 这是一个复杂的问题，但对于您的项目，最简单的解决办法是
// 在导出类之前暂时禁用此警告。
// ----------------------------------------------------------------------
#ifdef _MSC_VER
#define DISABLE_C4251_WARNING       \
  __pragma(warning(push)) __pragma( \
      warning(disable : 4251))  // NOLINT(readability-identifier-naming)

#define ENABLE_C4251_WARNING \
  __pragma(warning(pop))  // NOLINT(readability-identifier-naming)
#else
#define DISABLE_C4251_WARNING  // NOLINT(readability-identifier-naming)
#define ENABLE_C4251_WARNING   // NOLINT(readability-identifier-naming)
#endif

#endif  // INFRASTRUCTURE_REPORTS_SHARED_API_SHARED_API_H_