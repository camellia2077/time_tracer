# cmake/AddReportsDataLibrary.cmake
# 模块：reports_data 静态库定义
# 职责：负责数据层（Data Layer）的构建配置

# =========================================================
# [架构设计说明 / Architecture Note]
# 为什么 reports_data 值得单独做成静态库？
#
# 1. 依赖隔离 (Dependency Isolation):
#    reports_data 是系统中 **唯一** 直接依赖 SQLite 库的模块（数据层）。
#
# 2. 架构洁癖 (Architectural Hygiene):
#    把它封装成独立的库，通过 CMake 的链接属性控制，可以防止主程序里
#    其他模块（比如 CLI 界面层、Core 业务层）不小心直接调用 SQLite 的底层 API。
#    这强制上层逻辑只能通过 DayQuerier 等封装好的接口来访问数据。
#    这不是为了减小体积，而是为了保持代码边界清晰。
# =========================================================

# 1. 定义静态库
# 使用 STATIC 链接，意味着它最终会被合并进可执行文件，但编译时保持独立
add_library(reports_data STATIC ${REPORTS_DATA_SOURCES})

# 2. 应用项目通用配置
setup_project_target(reports_data)

# 3. 链接依赖
# reports_data 需要使用 reports_shared 中定义的数据结构 (如 DailyReportData)
# PUBLIC 属性确保链接 reports_data 的目标也能看到 reports_shared
target_link_libraries(reports_data PUBLIC reports_shared)

message(STATUS "Static Library target 'reports_data' (Data Layer) configured.")