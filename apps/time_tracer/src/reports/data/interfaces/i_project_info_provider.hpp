// reports/data/interfaces/i_project_info_provider.hpp
#ifndef REPORTS_DATA_INTERFACES_I_PROJECT_INFO_PROVIDER_H_
#define REPORTS_DATA_INTERFACES_I_PROJECT_INFO_PROVIDER_H_

#include <sqlite3.h>

#include <string>
#include <vector>

// [修改] 移除 REPORTS_DATA_API 宏，直接声明类
class IProjectInfoProvider {
 public:
  virtual ~IProjectInfoProvider() = default;

  // 确保缓存数据已加载
  virtual void EnsureLoaded(sqlite3* sqlite_db) = 0;

  // 获取项目路径部分
  [[nodiscard]] virtual auto GetPathParts(long long project_id) const
      -> std::vector<std::string> = 0;
};

#endif  // REPORTS_DATA_INTERFACES_I_PROJECT_INFO_PROVIDER_H_