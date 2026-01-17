// common/types/DateCheckMode.hpp
#ifndef COMMON_TYPES_DATE_CHECK_MODE_HPP_
#define COMMON_TYPES_DATE_CHECK_MODE_HPP_

// --- [架构优化] 将枚举定义下沉至 Common 层 ---
// 这样 Converter(业务层) 和 AppConfig(配置层) 都可以引用它，
// 而不需要 Common 依赖 Converter。

enum class DateCheckMode {
    None,       // 不检查
    Continuity, // 只检查连续性 (1号到当前存在的最大日期)
    Full        // 检查完整性 (1号到月底)
};

#endif // COMMON_TYPES_DATE_CHECK_MODE_HPP_
