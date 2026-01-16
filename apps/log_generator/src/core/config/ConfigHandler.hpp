// core/config/ConfigHandler.hpp
#ifndef CONFIG_HANDLER_HPP
#define CONFIG_HANDLER_HPP

#include "common/AppContext.hpp"
#include <optional>

namespace Core {

    class ConfigHandler {
    public:
        /**
         * @brief 初始化并加载所有配置。
         * 处理命令行参数解析、TOML文件读取和数据合并。
         * @return 如果成功返回 AppContext，如果失败或只需打印帮助则返回 nullopt。
         */
        std::optional<AppContext> load(int argc, char* argv[]);
    };

}

#endif // CONFIG_HANDLER_HPP