
#include <iostream>
#include <string>
#include <vector>
#include <clocale>

#include "LogProcessor.h"
#include "SharedUtils.h"

#ifdef _WIN32
#include <windows.h>
#endif

void setup_console_for_utf8() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#else
    std::setlocale(LC_ALL, "");
#endif
}

void print_usage(const char* app_name) {
    std::cerr << GREEN_COLOR << "使用方法: " << app_name << " <路径> [标志...]" << RESET_COLOR << std::endl;
    std::cerr << "\n必须提供 <文件或文件夹路径>." << std::endl;
    std::cerr << "\n主要动作标志:" << std::endl;
    std::cerr << "  -a,  --all\t\t\t执行完整流程(检验源->转换->检验输出)，任何错误都会立即终止程序。" << std::endl;
    std::cerr << "  -c,  --convert\t\t仅转换文件格式。" << std::endl;
    std::cerr << "  -vs, --validate-source\t仅检验源文件的格式。" << std::endl;
    
    std::cerr << "\n可选的附加标志:" << std::endl;
    std::cerr << "  -vo, --validate-output\t转换后检验输出文件 (必须与 -c 或 -a 一起使用)。" << std::endl;
    std::cerr << "  -edc, --enable-day-check\t启用对月份天数完整性的检查。" << std::endl;
    
    std::cerr << "\n其他:" << std::endl;
    std::cerr << "  -h,  --help\t\t\t显示此帮助信息。" << std::endl;
    std::cerr << "  -v,  --version\t\t显示版本号和更新日期。" << std::endl;
}

int main(int argc, char* argv[]) {
    setup_console_for_utf8();
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    const std::string APP_VERSION = "0.5.0"; // Version updated for LogProcessor class
    const std::string LAST_UPDATE_DATE = "2025-07-01"; 

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    // 1. 解析命令行参数，填充 AppOptions
    AppOptions options;
    std::vector<std::string> args(argv + 1, argv + argc);
    for (const auto& arg : args) {
        if (arg == "-h" || arg == "--help") { print_usage(argv[0]); return 0; }
        if (arg == "-v" || arg == "--version") { std::cout << "版本号: " << APP_VERSION << "\n最后更新时间: " << LAST_UPDATE_DATE << std::endl; return 0; }
        if (arg == "-a" || arg == "--all") options.run_all = true;
        else if (arg == "-c" || arg == "--convert") options.convert = true;
        else if (arg == "-vs" || arg == "--validate-source") options.validate_source = true;
        else if (arg == "-vo" || arg == "--validate-output") options.validate_output = true;
        else if (arg == "-edc" || arg == "--enable-day-check") options.enable_day_count_check = true;
        else if (arg.rfind("-", 0) != 0) {
            if (!options.input_path.empty()) { std::cerr << RED_COLOR << "错误: 检测到多个输入路径。请只提供一个。" << RESET_COLOR << std::endl; return 1; }
            options.input_path = arg;
        }
    }

    if (options.run_all) {
        options.validate_source = true;
        options.convert = true;
        options.validate_output = true;
    }
    
    // 参数合法性检查
    if (options.input_path.empty()) { std::cerr << RED_COLOR << "错误: 未提供输入文件或文件夹路径。" << RESET_COLOR << std::endl; print_usage(argv[0]); return 1; }
    if (!options.convert && !options.validate_source && !options.run_all) { std::cerr << RED_COLOR << "错误: 必须指定至少一个主要动作 (-c, -vs, 或 -a)。" << RESET_COLOR << std::endl; print_usage(argv[0]); return 1; }
    if (options.validate_output && !options.convert) { std::cerr << RED_COLOR << "错误: -vo 必须与 -c 或 -a 一起使用。" << RESET_COLOR << std::endl; return 1; }
    
    // 2. 创建 AppConfig
    AppConfig config = {
        "interval_processor_config.json",
        "format_validator_config.json",
        "validation_errors.txt"
    };
    
    // 3. 创建 LogProcessor 实例并运行
    LogProcessor app(config);
    bool success = app.run(options);
    
    // 4. 根据结果返回退出码
    return success ? 0 : 1;
}