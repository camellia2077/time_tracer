// domain/types/date_check_mode.hpp
#ifndef DOMAIN_TYPES_DATE_CHECK_MODE_H_
#define DOMAIN_TYPES_DATE_CHECK_MODE_H_

// --- [架构优化] 将枚举定义下沉至 Common 层 ---
// 这样 Converter(业务层) 和 AppConfig(配置层) 都可以引用它，
// 而不需要 Common 依赖 Converter。

enum class DateCheckMode {
  kNone,        // 不检查
  kContinuity,  // 只检查连续性 (1号到当前存在的最大日期)
  kFull         // 检查完整性 (1号到月底)
};

#endif  // DOMAIN_TYPES_DATE_CHECK_MODE_H_
