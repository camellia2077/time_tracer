// domain/logic/validator/structure/rules/activity_rules.cpp
#include "domain/logic/validator/structure/rules/activity_rules.hpp"

namespace validator::structure {

void validateActivityCount(const DailyLog& day, std::set<Error>& errors) {
  // 检查 activities 数量
  // 规则：每天至少应该有 2 个活动（例如 Sleep 和其他活动），
  // 否则可能会导致某些统计计算异常。
  // 注意：这里使用的是 processedActivities，它是解析和合并后的活动列表
  if (day.processedActivities.size() < 2) {
    errors.insert({0,
                   "In file for date " + day.date +
                       ": The day has less than 2 activities. This may cause "
                       "issues with 'sleep' activity generation.",
                   ErrorType::kJsonTooFewActivities});
  }
}

}  // namespace validator::structure
